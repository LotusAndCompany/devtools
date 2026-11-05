#include "regex_operations.h"

#include "features/regex_tool/core/regex_tool.h"
#include "features/web_frontend/gui/bridge/bridge_limits.h"
#include "features/web_frontend/gui/bridge/operations/operation_support.h"

#include <QJsonDocument>
#include <QMetaType>
#include <QRegularExpression>
#include <QVariantList>

using FrontendOperationSupport::invalidInput;
using FrontendOperationSupport::readText;

namespace FrontendOperationHandlers {
namespace {
constexpr qsizetype MAX_REGEX_MATCHES = 10000;
constexpr qsizetype REGEX_RESULT_METADATA_BYTES = 128;

enum class RegexResultBuildStatus {
    Completed,
    Cancelled,
    SizeLimitExceeded,
};

FrontendOperationService::Result regexSizeLimitExceeded(const QString &message)
{
    return {{}, QStringLiteral("SIZE_LIMIT_EXCEEDED"), message, false};
}

class RegexResultBuilder
{
public:
    explicit RegexResultBuilder(const FrontendOperationService::CancellationToken &cancellation)
        : cancellation_(cancellation)
    {}

    [[nodiscard]] bool isCancelled() const
    {
        return cancellation_->load(std::memory_order_relaxed);
    }

    [[nodiscard]] qsizetype remainingBytes() const
    {
        return BridgeLimits::MAX_OUTPUT_BYTES - result_budget_bytes_;
    }

    RegexResultBuildStatus appendMatch(const QRegularExpressionMatch &match,
                                       const QStringList &captureNames, int matchIndex)
    {
        QVariantList groups;
        for (int groupIndex = 0; groupIndex <= match.lastCapturedIndex(); ++groupIndex) {
            if (isCancelled()) {
                return RegexResultBuildStatus::Cancelled;
            }
            const QString name = captureNames.value(groupIndex);
            const QString groupValue = match.captured(groupIndex);
            if (!addMetadata() || !addString(name) || !addString(groupValue)) {
                return RegexResultBuildStatus::SizeLimitExceeded;
            }
            groups.append(
                QVariantMap{{QStringLiteral("index"), groupIndex},
                            {QStringLiteral("name"), name},
                            {QStringLiteral("value"), groupValue},
                            {QStringLiteral("offset"), match.capturedStart(groupIndex)},
                            {QStringLiteral("length"), match.capturedLength(groupIndex)}});
        }

        const QString matchValue = match.captured(0);
        if (!addMetadata() || !addString(matchValue)) {
            return RegexResultBuildStatus::SizeLimitExceeded;
        }
        matches_.append(QVariantMap{{QStringLiteral("index"), matchIndex},
                                    {QStringLiteral("offset"), match.capturedStart()},
                                    {QStringLiteral("length"), match.capturedLength()},
                                    {QStringLiteral("value"), matchValue},
                                    {QStringLiteral("groups"), groups}});
        return RegexResultBuildStatus::Completed;
    }

    [[nodiscard]] bool setReplacementText(const QString &text)
    {
        if (!addString(text)) {
            return false;
        }
        replaced_text_ = text;
        return true;
    }

    [[nodiscard]] QVariantMap value() const
    {
        return {{QStringLiteral("matches"), matches_},
                {QStringLiteral("replacedText"), replaced_text_}};
    }

    [[nodiscard]] bool fitsSerializedLimit() const
    {
        return QJsonDocument::fromVariant(value()).toJson(QJsonDocument::Compact).size() <=
               BridgeLimits::MAX_OUTPUT_BYTES;
    }

private:
    FrontendOperationService::CancellationToken cancellation_;
    QVariantList matches_;
    QString replaced_text_;
    qsizetype result_budget_bytes_{REGEX_RESULT_METADATA_BYTES};

    bool addMetadata()
    {
        if (REGEX_RESULT_METADATA_BYTES > remainingBytes()) {
            return false;
        }
        result_budget_bytes_ += REGEX_RESULT_METADATA_BYTES;
        return true;
    }

    bool addString(const QString &value)
    {
        const qsizetype bytes = value.toUtf8().size();
        if (bytes > remainingBytes()) {
            return false;
        }
        result_budget_bytes_ += bytes;
        return true;
    }
};

FrontendOperationService::Result completedRegexResult(const RegexResultBuilder &builder)
{
    if (!builder.fitsSerializedLimit()) {
        return regexSizeLimitExceeded(
            QStringLiteral("Regex results exceed the 4 MiB output limit."));
    }
    return {builder.value(), {}, {}, false};
}

FrontendOperationService::Result regexResultBuildFailure(RegexResultBuildStatus status)
{
    if (status == RegexResultBuildStatus::Cancelled) {
        return {{}, {}, {}, true};
    }
    return regexSizeLimitExceeded(QStringLiteral("Regex results exceed the 4 MiB output limit."));
}
} // namespace

FrontendOperationService::Result evaluateRegex(
    const QVariantMap &payload, const FrontendOperationService::CancellationToken &cancellation,
    const FrontendOperationService::ProgressCallback &progress)
{
    QString pattern;
    QString input;
    QString replacement;
    if (!readText(payload, QStringLiteral("pattern"), &pattern) ||
        !readText(payload, QStringLiteral("text"), &input) ||
        !readText(payload, QStringLiteral("replacement"), &replacement)) {
        return invalidInput(QStringLiteral(
            "Pattern, text and replacement must be UTF-8 strings no larger than 1 MiB."));
    }

    RegexResultBuilder resultBuilder(cancellation);
    if (resultBuilder.isCancelled()) {
        return {{}, {}, {}, true};
    }
    if (pattern.isEmpty()) {
        if (!resultBuilder.setReplacementText(input)) {
            return regexSizeLimitExceeded(
                QStringLiteral("Regex results exceed the 4 MiB output limit."));
        }
        return completedRegexResult(resultBuilder);
    }

    const auto options = devtools::RegexTool::patternOptions(
        payload.value(QStringLiteral("caseInsensitive")).toBool(),
        payload.value(QStringLiteral("multiline")).toBool(),
        payload.value(QStringLiteral("dotMatchesEverything")).toBool(),
        payload.value(QStringLiteral("useUnicodeProperties")).toBool(),
        payload.value(QStringLiteral("extendedPatternSyntax")).toBool());
    const bool global = !payload.contains(QStringLiteral("global")) ||
                        payload.value(QStringLiteral("global")).toBool();
    const QString limitedPattern = QStringLiteral("(*LIMIT_MATCH=1000000)") + pattern;
    const QRegularExpression regex(limitedPattern, options);
    if (!regex.isValid()) {
        return invalidInput(regex.errorString());
    }
    if (resultBuilder.isCancelled()) {
        return {{}, {}, {}, true};
    }

    progress(15, QStringLiteral("Matching regular expression"));
    const QStringList captureNames = regex.namedCaptureGroups();
    auto iterator = global ? regex.globalMatch(input) : QRegularExpressionMatchIterator{};
    int matchIndex = 0;
    if (global) {
        while (iterator.hasNext()) {
            if (resultBuilder.isCancelled()) {
                return {{}, {}, {}, true};
            }
            if (matchIndex >= MAX_REGEX_MATCHES) {
                return regexSizeLimitExceeded(
                    QStringLiteral("Regex results are limited to 10,000 matches."));
            }
            const RegexResultBuildStatus status =
                resultBuilder.appendMatch(iterator.next(), captureNames, matchIndex++);
            if (status != RegexResultBuildStatus::Completed) {
                return regexResultBuildFailure(status);
            }
        }
        if (!iterator.isValid()) {
            return invalidInput(QStringLiteral("Regular expression match limit exceeded."));
        }
    } else {
        const QRegularExpressionMatch match = regex.match(input);
        if (match.hasMatch()) {
            const RegexResultBuildStatus status = resultBuilder.appendMatch(match, captureNames, 0);
            if (status != RegexResultBuildStatus::Completed) {
                return regexResultBuildFailure(status);
            }
        }
    }
    if (resultBuilder.isCancelled()) {
        return {{}, {}, {}, true};
    }

    QString replaced;
    const auto replaceResult = devtools::RegexTool::replaceBounded(
        limitedPattern, input, replacement, options, global, resultBuilder.remainingBytes(),
        MAX_REGEX_MATCHES,
        [cancellation]() { return cancellation->load(std::memory_order_relaxed); }, &replaced);
    if (replaceResult == devtools::RegexTool::BoundedReplaceResult::Cancelled) {
        return {{}, {}, {}, true};
    }
    if (replaceResult == devtools::RegexTool::BoundedReplaceResult::MatchLimitExceeded) {
        return invalidInput(QStringLiteral("Regular expression match limit exceeded."));
    }
    if (replaceResult == devtools::RegexTool::BoundedReplaceResult::SizeLimitExceeded ||
        !resultBuilder.setReplacementText(replaced)) {
        return regexSizeLimitExceeded(
            QStringLiteral("Regex results exceed the 4 MiB output limit."));
    }

    progress(90, QStringLiteral("Building results"));
    return completedRegexResult(resultBuilder);
}

} // namespace FrontendOperationHandlers

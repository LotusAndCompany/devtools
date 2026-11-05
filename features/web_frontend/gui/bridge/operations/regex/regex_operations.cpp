#include "regex_operations.h"

#include "features/regex_tool/core/regex_tool.h"
#include "features/web_frontend/gui/bridge/bridge_limits.h"
#include "features/web_frontend/gui/bridge/operations/operation_support.h"

#include <QJsonDocument>
#include <QMetaType>
#include <QRegularExpression>
#include <QVariantList>

#include <cstdint>
#include <utility>

using FrontendOperationSupport::invalidInput;
using FrontendOperationSupport::readText;

namespace FrontendOperationHandlers {
namespace {
constexpr qsizetype MAX_REGEX_MATCHES = 10000;
constexpr qsizetype REGEX_RESULT_METADATA_BYTES = 128;

enum class RegexResultBuildStatus : std::uint8_t {
    Completed,
    Cancelled,
    SizeLimitExceeded,
    MatchCountExceeded,
    MatchLimitExceeded,
};

FrontendOperationService::Result regexSizeLimitExceeded(const QString &message)
{
    return {{}, QStringLiteral("SIZE_LIMIT_EXCEEDED"), message, false};
}

class RegexResultBuilder
{
public:
    explicit RegexResultBuilder(FrontendOperationService::CancellationToken cancellation)
        : cancellation_(std::move(cancellation))
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
        const bool accepted = addString(text);
        if (accepted) {
            replaced_text_ = text;
        }
        return accepted;
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
        const bool withinLimit = REGEX_RESULT_METADATA_BYTES <= remainingBytes();
        if (withinLimit) {
            result_budget_bytes_ += REGEX_RESULT_METADATA_BYTES;
        }
        return withinLimit;
    }

    bool addString(const QString &value)
    {
        const qsizetype bytes = value.toUtf8().size();
        const bool withinLimit = bytes <= remainingBytes();
        if (withinLimit) {
            result_budget_bytes_ += bytes;
        }
        return withinLimit;
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
    if (status == RegexResultBuildStatus::MatchLimitExceeded) {
        return invalidInput(QStringLiteral("Regular expression match limit exceeded."));
    }
    if (status == RegexResultBuildStatus::MatchCountExceeded) {
        return regexSizeLimitExceeded(
            QStringLiteral("Regex results are limited to 10,000 matches."));
    }
    return regexSizeLimitExceeded(QStringLiteral("Regex results exceed the 4 MiB output limit."));
}

bool readRegexInput(const QVariantMap &payload, QString *pattern, QString *input,
                    QString *replacement)
{
    return readText(payload, QStringLiteral("pattern"), pattern) &&
           readText(payload, QStringLiteral("text"), input) &&
           readText(payload, QStringLiteral("replacement"), replacement);
}

RegexResultBuildStatus appendRegexMatches(
    const QRegularExpression &regex, const QString &input, bool global,
    RegexResultBuilder *resultBuilder, const FrontendOperationService::ProgressCallback &progress)
{
    progress(15, QStringLiteral("Matching regular expression"));
    const QStringList captureNames = regex.namedCaptureGroups();
    int matchIndex = 0;
    if (!global) {
        if (resultBuilder->isCancelled()) {
            return RegexResultBuildStatus::Cancelled;
        }
        const QRegularExpressionMatch match = regex.match(input);
        return match.hasMatch() ? resultBuilder->appendMatch(match, captureNames, matchIndex)
                                : RegexResultBuildStatus::Completed;
    }

    QRegularExpressionMatchIterator iterator = regex.globalMatch(input);
    while (iterator.hasNext()) {
        if (resultBuilder->isCancelled()) {
            return RegexResultBuildStatus::Cancelled;
        }
        if (matchIndex >= MAX_REGEX_MATCHES) {
            return RegexResultBuildStatus::MatchCountExceeded;
        }
        const RegexResultBuildStatus status =
            resultBuilder->appendMatch(iterator.next(), captureNames, matchIndex++);
        if (status != RegexResultBuildStatus::Completed) {
            return status;
        }
    }
    if (resultBuilder->isCancelled()) {
        return RegexResultBuildStatus::Cancelled;
    }
    return iterator.isValid() ? RegexResultBuildStatus::Completed
                              : RegexResultBuildStatus::MatchLimitExceeded;
}

RegexResultBuildStatus buildRegexReplacement(
    const QString &pattern, const QString &input, const QString &replacement,
    QRegularExpression::PatternOptions options, bool global, RegexResultBuilder *resultBuilder,
    const FrontendOperationService::CancellationToken &cancellation)
{
    QString replaced;
    const auto status = devtools::RegexTool::replaceBounded(
        pattern, input, replacement, options, global, resultBuilder->remainingBytes(),
        MAX_REGEX_MATCHES,
        [cancellation]() { return cancellation->load(std::memory_order_relaxed); }, &replaced);
    if (status == devtools::RegexTool::BoundedReplaceResult::Cancelled) {
        return RegexResultBuildStatus::Cancelled;
    }
    if (status == devtools::RegexTool::BoundedReplaceResult::MatchLimitExceeded) {
        return RegexResultBuildStatus::MatchLimitExceeded;
    }
    if (status == devtools::RegexTool::BoundedReplaceResult::SizeLimitExceeded ||
        !resultBuilder->setReplacementText(replaced)) {
        return RegexResultBuildStatus::SizeLimitExceeded;
    }
    return RegexResultBuildStatus::Completed;
}
} // namespace

FrontendOperationService::Result evaluateRegex(
    const QVariantMap &payload, const FrontendOperationService::CancellationToken &cancellation,
    const FrontendOperationService::ProgressCallback &progress)
{
    QString pattern;
    QString input;
    QString replacement;
    if (!readRegexInput(payload, &pattern, &input, &replacement)) {
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

    auto status = appendRegexMatches(regex, input, global, &resultBuilder, progress);
    if (status != RegexResultBuildStatus::Completed) {
        return regexResultBuildFailure(status);
    }
    status = buildRegexReplacement(limitedPattern, input, replacement, options, global,
                                   &resultBuilder, cancellation);
    if (status != RegexResultBuildStatus::Completed) {
        return regexResultBuildFailure(status);
    }

    progress(90, QStringLiteral("Building results"));
    return completedRegexResult(resultBuilder);
}

} // namespace FrontendOperationHandlers

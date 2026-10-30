#include "regex_operations.h"

#include "features/regex_tool/core/regex_tool.h"
#include "operation_support.h"

#include <QMetaType>
#include <QRegularExpression>
#include <QVariantList>

using FrontendOperationSupport::invalidInput;
using FrontendOperationSupport::readText;

namespace FrontendOperationHandlers {

// NOLINTNEXTLINE(readability-function-size) -- one result flow.
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
    if (pattern.isEmpty()) {
        return {
            {{QStringLiteral("matches"), QVariantList{}}, {QStringLiteral("replacedText"), input}},
            {},
            {},
            false};
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
    if (cancellation->load(std::memory_order_relaxed)) {
        return {{}, {}, {}, true};
    }

    progress(15, QStringLiteral("Matching regular expression"));
    QVariantList matches;
    const QStringList captureNames = regex.namedCaptureGroups();
    auto iterator = global ? regex.globalMatch(input) : QRegularExpressionMatchIterator{};
    int index = 0;
    if (global) {
        while (iterator.hasNext()) {
            if (cancellation->load(std::memory_order_relaxed)) {
                return {{}, {}, {}, true};
            }
            const QRegularExpressionMatch match = iterator.next();
            QVariantList groups;
            for (int groupIndex = 0; groupIndex <= match.lastCapturedIndex(); ++groupIndex) {
                groups.append(
                    QVariantMap{{QStringLiteral("index"), groupIndex},
                                {QStringLiteral("name"), captureNames.value(groupIndex)},
                                {QStringLiteral("value"), match.captured(groupIndex)},
                                {QStringLiteral("offset"), match.capturedStart(groupIndex)},
                                {QStringLiteral("length"), match.capturedLength(groupIndex)}});
            }
            matches.append(QVariantMap{{QStringLiteral("index"), index++},
                                       {QStringLiteral("offset"), match.capturedStart()},
                                       {QStringLiteral("length"), match.capturedLength()},
                                       {QStringLiteral("value"), match.captured(0)},
                                       {QStringLiteral("groups"), groups}});
            if (matches.size() >= 10000) {
                return {{},
                        QStringLiteral("SIZE_LIMIT_EXCEEDED"),
                        QStringLiteral("Regex results are limited to 10,000 matches."),
                        false};
            }
        }
        if (!iterator.isValid()) {
            return invalidInput(QStringLiteral("Regular expression match limit exceeded."));
        }
    } else {
        const QRegularExpressionMatch match = regex.match(input);
        if (match.hasMatch()) {
            QVariantList groups;
            for (int groupIndex = 0; groupIndex <= match.lastCapturedIndex(); ++groupIndex) {
                groups.append(
                    QVariantMap{{QStringLiteral("index"), groupIndex},
                                {QStringLiteral("name"), captureNames.value(groupIndex)},
                                {QStringLiteral("value"), match.captured(groupIndex)},
                                {QStringLiteral("offset"), match.capturedStart(groupIndex)},
                                {QStringLiteral("length"), match.capturedLength(groupIndex)}});
            }
            matches.append(QVariantMap{{QStringLiteral("index"), 0},
                                       {QStringLiteral("offset"), match.capturedStart()},
                                       {QStringLiteral("length"), match.capturedLength()},
                                       {QStringLiteral("value"), match.captured(0)},
                                       {QStringLiteral("groups"), groups}});
        }
    }
    if (cancellation->load(std::memory_order_relaxed)) {
        return {{}, {}, {}, true};
    }
    const QString replaced =
        devtools::RegexTool::replace(limitedPattern, input, replacement, options, global);
    progress(90, QStringLiteral("Building results"));
    return {{{QStringLiteral("matches"), matches}, {QStringLiteral("replacedText"), replaced}},
            {},
            {},
            false};
}

} // namespace FrontendOperationHandlers

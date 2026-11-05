#include "features/regex_tool/core/regex_tool.h"

#include <QByteArray>
#include <QThread>

#include <utility>

namespace {

/**
 * @brief 置換文字列を1回だけ走査して展開する
 *
 * ECMA風の置換トークン（$&, $<name>, $1..$9, $$）を解決する。キャプチャ値は
 * そのまま出力し、再走査は行わない。
 */
QString substituteReplacement(const QString &replacement, const QRegularExpressionMatch &match)
{
    QString result;
    const qsizetype length = replacement.size();
    for (qsizetype i = 0; i < length; ++i) {
        const QChar current = replacement.at(i);
        if (current != '$' || i + 1 >= length) {
            result.append(current);
            continue;
        }

        const QChar next = replacement.at(i + 1);
        if (next == '$') {
            result.append('$');
            ++i;
        } else if (next == '&') {
            result.append(match.captured(0));
            ++i;
        } else if (next == '<') {
            const qsizetype end = replacement.indexOf('>', i + 2);
            if (end > i + 2) {
                result.append(match.captured(replacement.mid(i + 2, end - i - 2)));
                i = end;
            } else {
                result.append("$<");
                ++i;
            }
        } else if (next.isDigit() && next.digitValue() >= 1 && next.digitValue() <= 9) {
            result.append(match.captured(next.digitValue()));
            ++i;
        } else {
            result.append('$');
        }
    }
    return result;
}

using BoundedReplaceResult = devtools::RegexTool::BoundedReplaceResult;

BoundedReplaceResult appendBounded(QString *result, const QString &fragment, qsizetype *outputBytes,
                                   qsizetype maxOutputBytes)
{
    const qsizetype fragmentBytes = fragment.toUtf8().size();
    if (fragmentBytes > maxOutputBytes - *outputBytes) {
        return BoundedReplaceResult::SizeLimitExceeded;
    }
    result->append(fragment);
    *outputBytes += fragmentBytes;
    return BoundedReplaceResult::Completed;
}

BoundedReplaceResult appendReplacementTextChunk(QString *result, const QString &replacement,
                                                qsizetype *position, qsizetype *outputBytes,
                                                qsizetype maxOutputBytes)
{
    const qsizetype start = *position;
    const qsizetype endLimit = qMin(replacement.size(), start + 4096);
    while (*position < endLimit && replacement.at(*position) != '$') {
        ++*position;
    }
    return appendBounded(result, replacement.mid(start, *position - start), outputBytes,
                         maxOutputBytes);
}

BoundedReplaceResult appendReplacementToken(QString *result, const QString &replacement,
                                            const QRegularExpressionMatch &match,
                                            qsizetype *position, qsizetype *outputBytes,
                                            qsizetype maxOutputBytes)
{
    if (*position + 1 >= replacement.size()) {
        const auto status = appendBounded(result, QStringLiteral("$"), outputBytes, maxOutputBytes);
        if (status == BoundedReplaceResult::Completed) {
            ++*position;
        }
        return status;
    }

    const qsizetype start = *position;
    const QChar next = replacement.at(start + 1);
    QString captured;
    qsizetype consumed = 1;
    if (next == '$') {
        captured = QStringLiteral("$");
        consumed = 2;
    } else if (next == '&') {
        captured = match.captured(0);
        consumed = 2;
    } else if (next == '<') {
        const qsizetype end = replacement.indexOf('>', start + 2);
        if (end > start + 2) {
            captured = match.captured(replacement.mid(start + 2, end - start - 2));
            consumed = end - start + 1;
        } else {
            captured = QStringLiteral("$<");
            consumed = 2;
        }
    } else if (next.isDigit() && next.digitValue() >= 1 && next.digitValue() <= 9) {
        captured = match.captured(next.digitValue());
        consumed = 2;
    } else {
        captured = QStringLiteral("$");
    }

    const auto status = appendBounded(result, captured, outputBytes, maxOutputBytes);
    if (status == BoundedReplaceResult::Completed) {
        *position += consumed;
    }
    return status;
}

BoundedReplaceResult appendReplacementBounded(QString *result, const QString &replacement,
                                              const QRegularExpressionMatch &match,
                                              qsizetype *outputBytes, qsizetype maxOutputBytes,
                                              const std::function<bool()> &isCancelled)
{
    qsizetype position = 0;
    while (position < replacement.size()) {
        if (isCancelled()) {
            return BoundedReplaceResult::Cancelled;
        }
        const auto status = replacement.at(position) == '$'
                                ? appendReplacementToken(result, replacement, match, &position,
                                                         outputBytes, maxOutputBytes)
                                : appendReplacementTextChunk(result, replacement, &position,
                                                             outputBytes, maxOutputBytes);
        if (status != BoundedReplaceResult::Completed) {
            return status;
        }
    }
    return BoundedReplaceResult::Completed;
}

struct BoundedReplaceState
{
    QString *result{};
    const QString *text{};
    const QString *replacement{};
    qsizetype maxOutputBytes{};
    qsizetype maxMatches{};
    std::function<bool()> isCancelled;
    qsizetype outputBytes{};
    qsizetype lastPos{};
    qsizetype matchCount{};
};

BoundedReplaceResult appendBoundedMatch(BoundedReplaceState *state,
                                        const QRegularExpressionMatch &match)
{
    if (state->isCancelled()) {
        return BoundedReplaceResult::Cancelled;
    }
    if (state->matchCount >= state->maxMatches) {
        return BoundedReplaceResult::MatchLimitExceeded;
    }

    auto status = appendBounded(
        state->result, state->text->mid(state->lastPos, match.capturedStart() - state->lastPos),
        &state->outputBytes, state->maxOutputBytes);
    if (status != BoundedReplaceResult::Completed) {
        return status;
    }
    status =
        appendReplacementBounded(state->result, *state->replacement, match, &state->outputBytes,
                                 state->maxOutputBytes, state->isCancelled);
    if (status != BoundedReplaceResult::Completed) {
        return status;
    }

    state->lastPos = match.capturedEnd();
    ++state->matchCount;
    return BoundedReplaceResult::Completed;
}

BoundedReplaceResult appendBoundedMatches(const QRegularExpression &regex,
                                          BoundedReplaceState *state, bool global)
{
    if (!global) {
        const QRegularExpressionMatch match = regex.match(*state->text);
        return match.hasMatch() ? appendBoundedMatch(state, match)
                                : BoundedReplaceResult::Completed;
    }

    QRegularExpressionMatchIterator iterator = regex.globalMatch(*state->text);
    while (iterator.hasNext()) {
        if (state->isCancelled()) {
            return BoundedReplaceResult::Cancelled;
        }
        const auto status = appendBoundedMatch(state, iterator.next());
        if (status != BoundedReplaceResult::Completed) {
            return status;
        }
    }
    return iterator.isValid() ? BoundedReplaceResult::Completed
                              : BoundedReplaceResult::MatchLimitExceeded;
}

} // namespace

namespace devtools {

QRegularExpression::PatternOptions RegexTool::patternOptions(bool caseInsensitive, bool multiline,
                                                             bool dotMatchesEverything,
                                                             bool useUnicodeProperties,
                                                             bool extendedPatternSyntax)
{
    QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
    if (caseInsensitive) {
        options |= QRegularExpression::CaseInsensitiveOption;
    }
    if (multiline) {
        options |= QRegularExpression::MultilineOption;
    }
    if (dotMatchesEverything) {
        options |= QRegularExpression::DotMatchesEverythingOption;
    }
    if (useUnicodeProperties) {
        options |= QRegularExpression::UseUnicodePropertiesOption;
    }
    if (extendedPatternSyntax) {
        options |= QRegularExpression::ExtendedPatternSyntaxOption;
    }
    return options;
}

QVector<MatchResult> RegexTool::match(const QString &pattern, const QString &text,
                                      QRegularExpression::PatternOptions options)
{
    QVector<MatchResult> results;
    if (pattern.isEmpty()) {
        return results;
    }

    QRegularExpression const re(pattern, options);
    if (!re.isValid()) {
        return results;
    }

    const QStringList capture_group_names = re.namedCaptureGroups();
    QRegularExpressionMatchIterator i = re.globalMatch(text);
    int match_index = 0;
    while (i.hasNext()) {
        QRegularExpressionMatch const match = i.next();
        MatchResult res;
        res.index = match_index++;
        res.offset = match.capturedStart();
        res.length = match.capturedLength();

        for (int j = 0; j <= match.lastCapturedIndex(); ++j) {
            CaptureGroup group;
            group.index = j;
            group.name = capture_group_names.value(j);
            group.value = match.captured(j);
            group.offset = match.capturedStart(j);
            group.length = match.capturedLength(j);
            res.groups.append(group);
        }
        results.append(res);
    }

    return results;
}

QString RegexTool::replace(const QString &pattern, const QString &text, const QString &replacement,
                           QRegularExpression::PatternOptions options, bool global)
{
    if (pattern.isEmpty()) {
        return text;
    }

    QRegularExpression const re(pattern, options);
    if (!re.isValid()) {
        return text;
    }

    QString result;
    qsizetype lastPos = 0;

    if (global) {
        QRegularExpressionMatchIterator i = re.globalMatch(text);
        while (!QThread::currentThread()->isInterruptionRequested() && i.hasNext()) {
            const QRegularExpressionMatch match = i.next();
            result.append(text.mid(lastPos, match.capturedStart() - lastPos));
            result.append(substituteReplacement(replacement, match));
            lastPos = match.capturedEnd();
        }
    } else {
        const QRegularExpressionMatch match = re.match(text);
        if (match.hasMatch()) {
            result.append(text.mid(lastPos, match.capturedStart() - lastPos));
            result.append(substituteReplacement(replacement, match));
            lastPos = match.capturedEnd();
        }
    }

    // Append the remaining text
    result.append(text.mid(lastPos));
    return result;
}

RegexTool::BoundedReplaceResult RegexTool::replaceBounded(
    const QString &pattern, const QString &text, const QString &replacement,
    QRegularExpression::PatternOptions options, bool global, qsizetype maxOutputBytes,
    qsizetype maxMatches, const std::function<bool()> &isCancelled, QString *result)
{
    result->clear();
    BoundedReplaceState state{result, &text, &replacement, maxOutputBytes, maxMatches, isCancelled};
    if (isCancelled()) {
        return BoundedReplaceResult::Cancelled;
    }
    if (pattern.isEmpty()) {
        return appendBounded(result, text, &state.outputBytes, maxOutputBytes);
    }

    QRegularExpression const re(pattern, options);
    if (!re.isValid()) {
        return appendBounded(result, text, &state.outputBytes, maxOutputBytes);
    }

    const BoundedReplaceResult status = appendBoundedMatches(re, &state, global);
    if (status != BoundedReplaceResult::Completed) {
        return status;
    }

    if (isCancelled()) {
        return BoundedReplaceResult::Cancelled;
    }
    return appendBounded(result, text.mid(state.lastPos), &state.outputBytes, maxOutputBytes);
}

QString RegexTool::formatMatchesForClipboard(const QVector<MatchResult> &matches,
                                             const QString &text, const QString &headerFormat,
                                             const QString &matchFormat, const QString &groupFormat)
{
    QString result = headerFormat.arg(matches.size()) + QStringLiteral("\n\n");
    for (const MatchResult &match : matches) {
        const QString matchText = text.mid(match.offset, match.length);
        result += matchFormat.arg(match.index + 1)
                      .arg(matchText)
                      .arg(match.offset)
                      .arg(match.offset + match.length);

        for (int i = 1; i < match.groups.size(); ++i) {
            const CaptureGroup &group = match.groups.at(i);
            const QString groupIdentifier =
                group.name.isEmpty() ? QString::number(group.index)
                                     : QStringLiteral("%1 (%2)").arg(group.index).arg(group.name);
            result += groupFormat.arg(groupIdentifier)
                          .arg(group.value)
                          .arg(group.offset)
                          .arg(group.offset + group.length);
        }
        result += QLatin1Char('\n');
    }
    return result;
}

} // namespace devtools

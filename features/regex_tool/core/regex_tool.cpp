#include "features/regex_tool/core/regex_tool.h"

#include <QByteArray>
#include <QThread>

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

BoundedReplaceResult appendReplacementBounded(QString *result, const QString &replacement,
                                              const QRegularExpressionMatch &match,
                                              qsizetype *outputBytes, qsizetype maxOutputBytes,
                                              const std::function<bool()> &isCancelled)
{
    const qsizetype length = replacement.size();
    for (qsizetype i = 0; i < length;) {
        if (isCancelled()) {
            return BoundedReplaceResult::Cancelled;
        }

        const QChar current = replacement.at(i);
        if (current != '$') {
            const qsizetype start = i;
            const qsizetype endLimit = qMin(length, start + 4096);
            while (i < endLimit && replacement.at(i) != '$') {
                ++i;
            }
            const auto appendResult = appendBounded(result, replacement.mid(start, i - start),
                                                    outputBytes, maxOutputBytes);
            if (appendResult != BoundedReplaceResult::Completed) {
                return appendResult;
            }
            continue;
        }

        if (i + 1 >= length) {
            const auto appendResult =
                appendBounded(result, QStringLiteral("$"), outputBytes, maxOutputBytes);
            if (appendResult != BoundedReplaceResult::Completed) {
                return appendResult;
            }
            ++i;
            continue;
        }

        const QChar next = replacement.at(i + 1);
        QString captured;
        qsizetype consumed = 1;
        if (next == '$') {
            captured = QStringLiteral("$");
            consumed = 2;
        } else if (next == '&') {
            captured = match.captured(0);
            consumed = 2;
        } else if (next == '<') {
            const qsizetype end = replacement.indexOf('>', i + 2);
            if (end > i + 2) {
                captured = match.captured(replacement.mid(i + 2, end - i - 2));
                consumed = end - i + 1;
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

        const auto appendResult = appendBounded(result, captured, outputBytes, maxOutputBytes);
        if (appendResult != BoundedReplaceResult::Completed) {
            return appendResult;
        }
        i += consumed;
    }
    return BoundedReplaceResult::Completed;
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
    qsizetype outputBytes = 0;
    if (isCancelled()) {
        return BoundedReplaceResult::Cancelled;
    }
    if (pattern.isEmpty()) {
        return appendBounded(result, text, &outputBytes, maxOutputBytes);
    }

    QRegularExpression const re(pattern, options);
    if (!re.isValid()) {
        return appendBounded(result, text, &outputBytes, maxOutputBytes);
    }

    qsizetype lastPos = 0;
    qsizetype matchCount = 0;
    const auto appendMatch = [&](const QRegularExpressionMatch &match) {
        if (isCancelled()) {
            return BoundedReplaceResult::Cancelled;
        }
        if (matchCount >= maxMatches) {
            return BoundedReplaceResult::SizeLimitExceeded;
        }
        auto appendResult =
            appendBounded(result, text.mid(lastPos, match.capturedStart() - lastPos), &outputBytes,
                          maxOutputBytes);
        if (appendResult != BoundedReplaceResult::Completed) {
            return appendResult;
        }
        appendResult = appendReplacementBounded(result, replacement, match, &outputBytes,
                                                maxOutputBytes, isCancelled);
        if (appendResult != BoundedReplaceResult::Completed) {
            return appendResult;
        }
        lastPos = match.capturedEnd();
        ++matchCount;
        return BoundedReplaceResult::Completed;
    };

    if (global) {
        QRegularExpressionMatchIterator iterator = re.globalMatch(text);
        while (iterator.hasNext()) {
            if (isCancelled()) {
                return BoundedReplaceResult::Cancelled;
            }
            const BoundedReplaceResult appendResult = appendMatch(iterator.next());
            if (appendResult != BoundedReplaceResult::Completed) {
                return appendResult;
            }
        }
        if (!iterator.isValid()) {
            return BoundedReplaceResult::MatchLimitExceeded;
        }
    } else {
        const QRegularExpressionMatch match = re.match(text);
        if (match.hasMatch()) {
            const BoundedReplaceResult appendResult = appendMatch(match);
            if (appendResult != BoundedReplaceResult::Completed) {
                return appendResult;
            }
        }
    }

    if (isCancelled()) {
        return BoundedReplaceResult::Cancelled;
    }
    return appendBounded(result, text.mid(lastPos), &outputBytes, maxOutputBytes);
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

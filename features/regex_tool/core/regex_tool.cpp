#include "features/regex_tool/core/regex_tool.h"

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

} // namespace

namespace devtools {

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
        while (i.hasNext()) {
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

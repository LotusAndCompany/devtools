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
                           QRegularExpression::PatternOptions options)
{
    if (pattern.isEmpty()) {
        return text;
    }

    QRegularExpression const re(pattern, options);
    if (!re.isValid()) {
        return text;
    }

    QString result;
    int lastPos = 0;

    QRegularExpressionMatchIterator i = re.globalMatch(text);
    while (i.hasNext()) {
        QRegularExpressionMatch const match = i.next();

        // Append the text before this match
        result.append(text.mid(lastPos, match.capturedStart() - lastPos));

        // Expand the replacement string exactly once for this match
        result.append(substituteReplacement(replacement, match));
        lastPos = match.capturedEnd();
    }

    // Append the remaining text
    result.append(text.mid(lastPos));
    return result;
}

} // namespace devtools

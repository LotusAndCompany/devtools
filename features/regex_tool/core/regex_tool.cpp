#include "features/regex_tool/core/regex_tool.h"

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

        // Process the replacement string for this match
        QString substituted = replacement;

        // 1. Replace $$ with a placeholder, then restore to literal $ later
        substituted.replace("$$", "\x01");

        // 2. Replace $& with the full match
        substituted.replace("$&", match.captured(0));

        // 3. Replace named capture groups: $<name>
        QRegularExpression const namedGroupRx(R"(\$<([a-zA-Z0-9_]+)>)");
        QRegularExpressionMatchIterator namedIt = namedGroupRx.globalMatch(substituted);
        QList<QRegularExpressionMatch> namedMatches;
        while (namedIt.hasNext()) {
            namedMatches.append(namedIt.next());
        }
        for (int j = namedMatches.size() - 1; j >= 0; --j) {
            const auto &nm = namedMatches[j];
            QString const name = nm.captured(1);
            QString const capturedValue = match.captured(name);
            substituted.replace(nm.capturedStart(), nm.capturedLength(), capturedValue);
        }

        // 4. Replace numbered capture groups: $1..$9
        QRegularExpression const numGroupRx(R"(\$([1-9]))");
        QRegularExpressionMatchIterator numIt = numGroupRx.globalMatch(substituted);
        QList<QRegularExpressionMatch> numMatches;
        while (numIt.hasNext()) {
            numMatches.append(numIt.next());
        }
        for (int j = numMatches.size() - 1; j >= 0; --j) {
            const auto &nm = numMatches[j];
            int const groupIdx = nm.captured(1).toInt();
            QString const capturedValue = match.captured(groupIdx);
            substituted.replace(nm.capturedStart(), nm.capturedLength(), capturedValue);
        }

        // 5. Restore literal $ from placeholder
        substituted.replace("\x01", "$");

        result.append(substituted);
        lastPos = match.capturedEnd();
    }

    // Append the remaining text
    result.append(text.mid(lastPos));
    return result;
}

} // namespace devtools

#include "core/regex_tool/regex_tool.h"

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

    QString result = text;
    return result.replace(re, replacement);
}

} // namespace devtools

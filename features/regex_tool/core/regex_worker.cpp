#include "regex_worker.h"

#include <QMutexLocker>

namespace devtools {

namespace {

constexpr int REGEX_MATCH_LIMIT = 1'000'000;

QString boundedPattern(const QString &pattern)
{
    return QStringLiteral("(*LIMIT_MATCH=%1)").arg(REGEX_MATCH_LIMIT) + pattern;
}

/**
 * @brief 正規表現のマッチを収集する（中断要求を随時確認する）
 */
QVector<MatchResult> collectMatches(const QRegularExpression &re, const QString &text, bool global,
                                    bool &is_valid)
{
    QVector<MatchResult> matches;
    const QStringList capture_group_names = re.namedCaptureGroups();
    is_valid = true;

    if (global) {
        QRegularExpressionMatchIterator i = re.globalMatch(text);
        int match_index = 0;
        while (!QThread::currentThread()->isInterruptionRequested() && i.hasNext()) {
            const QRegularExpressionMatch match = i.next();
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
            matches.append(res);
        }
        is_valid = i.isValid();
    } else {
        if (QThread::currentThread()->isInterruptionRequested()) {
            return matches;
        }
        const QRegularExpressionMatch match = re.match(text);
        is_valid = match.isValid();
        if (match.hasMatch()) {
            MatchResult res;
            res.index = 0;
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
            matches.append(res);
        }
    }

    return matches;
}

} // namespace

// ============================================================================
// RegexWorker Implementation
// ============================================================================

RegexWorker::RegexWorker(QObject *parent)
    : QThread(parent)
    , m_requestId(0)
    , m_options(QRegularExpression::NoPatternOption)
    , m_global(true)
{}

void RegexWorker::setParams(int requestId, const QString &pattern, const QString &text,
                            const QString &replacePattern,
                            QRegularExpression::PatternOptions options, bool global)
{
    const QMutexLocker locker(&m_mutex);
    m_requestId = requestId;
    m_pattern = pattern;
    m_text = text;
    m_replacePattern = replacePattern;
    m_options = options;
    m_global = global;
}

void RegexWorker::run()
{
    int requestId;
    QString pattern;
    QString text;
    QString replacePattern;
    QRegularExpression::PatternOptions options;
    bool global;
    {
        const QMutexLocker locker(&m_mutex);
        requestId = m_requestId;
        pattern = m_pattern;
        text = m_text;
        replacePattern = m_replacePattern;
        options = m_options;
        global = m_global;
    }

    if (pattern.isEmpty()) {
        emit finishedMatching(requestId, QVector<MatchResult>(), text, true, QString());
        return;
    }

    const QString limited_pattern = boundedPattern(pattern);
    const QRegularExpression re(limited_pattern, options);
    if (!re.isValid()) {
        emit finishedMatching(requestId, QVector<MatchResult>(), text, false, re.errorString());
        return;
    }

    bool is_match_valid = true;
    const QVector<MatchResult> matches = collectMatches(re, text, global, is_match_valid);

    if (isInterruptionRequested()) {
        return;
    }
    if (!is_match_valid) {
        emit finishedMatching(requestId, QVector<MatchResult>(), text, false,
                              QStringLiteral("Regular expression match limit exceeded."));
        return;
    }

    // Execute replace using custom ECMA-like substitution
    const QString replacedText =
        RegexTool::replace(limited_pattern, text, replacePattern, options, global);

    emit finishedMatching(requestId, matches, replacedText, true, QString());
}

} // namespace devtools

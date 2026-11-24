#include "regex_worker.h"

#include <QMutexLocker>

namespace devtools {

namespace {

constexpr int REGEX_MATCH_LIMIT = 1'000'000;

QString boundedPattern(const QString &pattern)
{
    return QStringLiteral("(*LIMIT_MATCH=%1)").arg(REGEX_MATCH_LIMIT) + pattern;
}

MatchResult makeMatchResult(const QRegularExpressionMatch &match, int index,
                            const QStringList &capture_group_names)
{
    MatchResult result;
    result.index = index;
    result.offset = match.capturedStart();
    result.length = match.capturedLength();

    for (int group_index = 0; group_index <= match.lastCapturedIndex(); ++group_index) {
        CaptureGroup group;
        group.index = group_index;
        group.name = capture_group_names.value(group_index);
        group.value = match.captured(group_index);
        group.offset = match.capturedStart(group_index);
        group.length = match.capturedLength(group_index);
        result.groups.append(group);
    }

    return result;
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
            matches.append(makeMatchResult(i.next(), match_index++, capture_group_names));
        }
        is_valid = i.isValid();
    } else {
        if (QThread::currentThread()->isInterruptionRequested()) {
            return matches;
        }
        const QRegularExpressionMatch match = re.match(text);
        is_valid = match.isValid();
        if (match.hasMatch()) {
            matches.append(makeMatchResult(match, 0, capture_group_names));
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

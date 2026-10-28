#ifndef REGEX_WORKER_H
#define REGEX_WORKER_H

#include "regex_tool.h"

#include <QMutex>
#include <QThread>

namespace devtools {

/**
 * @brief Background worker to evaluate regular expressions asynchronously
 */
class RegexWorker : public QThread
{
    Q_OBJECT
public:
    explicit RegexWorker(QObject *parent = nullptr);
    void setParams(int requestId, const QString &pattern, const QString &text,
                   const QString &replacePattern, QRegularExpression::PatternOptions options,
                   bool global);

signals:
    void finishedMatching(int requestId, const QVector<MatchResult> &matches,
                          const QString &replacedText, bool isValid, const QString &errorStr);

protected:
    void run() override;

private:
    QMutex m_mutex;
    int m_requestId;
    QString m_pattern;
    QString m_text;
    QString m_replacePattern;
    QRegularExpression::PatternOptions m_options;
    bool m_global;
};

} // namespace devtools

#endif // REGEX_WORKER_H

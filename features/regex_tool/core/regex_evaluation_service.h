#ifndef REGEX_EVALUATION_SERVICE_H
#define REGEX_EVALUATION_SERVICE_H

#include "regex_worker.h"

#include <QObject>
#include <QSet>
#include <QTimer>

namespace devtools {

class RegexEvaluationService : public QObject
{
    Q_OBJECT

public:
    explicit RegexEvaluationService(QObject *parent = nullptr, int watchdogTimeoutMs = 500);
    ~RegexEvaluationService() override;

    void evaluate(const QString &pattern, const QString &text, const QString &replacePattern,
                  QRegularExpression::PatternOptions options, bool global);
    void cancel();

signals:
    void matchingFinished(int requestId, const QVector<MatchResult> &matches,
                          const QString &replacedText, bool isValid, const QString &errorStr);
    void evaluationTimedOut(int requestId);

private slots:
    void handleMatchingFinished(int requestId, const QVector<MatchResult> &matches,
                                const QString &replacedText, bool isValid, const QString &errorStr);
    void handleWatchdogTimeout();

private:
    QTimer watchdogTimer;
    QSet<RegexWorker *> workers;
    RegexWorker *activeWorker{nullptr};
    int currentRequestId{0};
    int watchdogTimeoutMs{500};
};

} // namespace devtools

#endif // REGEX_EVALUATION_SERVICE_H

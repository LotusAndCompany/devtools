#include "regex_evaluation_service.h"

#include <QMetaType>

namespace devtools {

RegexEvaluationService::RegexEvaluationService(QObject *parent, int watchdogTimeoutMs)
    : QObject(parent), watchdogTimeoutMs(watchdogTimeoutMs)
{
    watchdogTimer.setSingleShot(true);
    connect(&watchdogTimer, &QTimer::timeout, this, &RegexEvaluationService::handleWatchdogTimeout);
    qRegisterMetaType<QVector<MatchResult>>();
}

RegexEvaluationService::~RegexEvaluationService()
{
    cancel();
    for (RegexWorker *worker : workers) {
        worker->disconnect(this);
        worker->requestInterruption();
    }
    workers.clear();
}

void RegexEvaluationService::evaluate(const QString &pattern, const QString &text,
                                      const QString &replacePattern,
                                      QRegularExpression::PatternOptions options, bool global)
{
    watchdogTimer.stop();
    if (activeWorker != nullptr && activeWorker->isRunning()) {
        activeWorker->requestInterruption();
    }

    const int requestId = ++currentRequestId;
    auto *const worker = createWorker();
    workers.insert(worker);
    activeWorker = worker;
    connect(worker, &RegexWorker::finishedMatching, this,
            &RegexEvaluationService::handleMatchingFinished);
    connect(worker, &QThread::finished, this, [this, worker] {
        workers.remove(worker);
        if (activeWorker == worker) {
            activeWorker = nullptr;
        }
    });
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);

    worker->setParams(requestId, pattern, text, replacePattern, options, global);
    worker->start();
    watchdogTimer.start(watchdogTimeoutMs);
}

RegexWorker *RegexEvaluationService::createWorker()
{
    return new RegexWorker();
}

void RegexEvaluationService::cancel()
{
    watchdogTimer.stop();
    ++currentRequestId;
    if (activeWorker != nullptr && activeWorker->isRunning()) {
        activeWorker->requestInterruption();
    }
    activeWorker = nullptr;
}

void RegexEvaluationService::handleMatchingFinished(int requestId,
                                                    const QVector<MatchResult> &matches,
                                                    const QString &replacedText, bool isValid,
                                                    const QString &errorStr)
{
    if (requestId != currentRequestId) {
        return;
    }

    watchdogTimer.stop();
    emit matchingFinished(requestId, matches, replacedText, isValid, errorStr);
}

void RegexEvaluationService::handleWatchdogTimeout()
{
    const int timedOutRequestId = currentRequestId;
    ++currentRequestId;
    if (activeWorker != nullptr && activeWorker->isRunning()) {
        activeWorker->requestInterruption();
    }
    emit evaluationTimedOut(timedOutRequestId);
}

} // namespace devtools

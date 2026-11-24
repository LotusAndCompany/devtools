#include "features/regex_tool/core/regex_evaluation_service.h"

#include <QElapsedTimer>
#include <QSemaphore>
#include <QSignalSpy>
#include <QtTest>

#include <chrono>
#include <thread>

namespace {

class BlockingRegexWorker final : public devtools::RegexWorker
{
public:
    BlockingRegexWorker(QSemaphore &started, QSemaphore &resume) : started(started), resume(resume)
    {}

protected:
    void run() override
    {
        started.release();
        resume.acquire();
    }

private:
    QSemaphore &started;
    QSemaphore &resume;
};

class BlockingRegexEvaluationService final : public devtools::RegexEvaluationService
{
public:
    BlockingRegexEvaluationService(QSemaphore &started, QSemaphore &resume)
        : RegexEvaluationService(nullptr, 20), started(started), resume(resume)
    {}

    devtools::RegexWorker *worker{nullptr};

protected:
    devtools::RegexWorker *createWorker() override
    {
        worker = new BlockingRegexWorker(started, resume);
        return worker;
    }

private:
    QSemaphore &started;
    QSemaphore &resume;
};

} // namespace

class TestRegexEvaluationService : public QObject
{
    Q_OBJECT

private slots:
    void latestRequestDiscardsStaleResults();
    void cancellationDiscardsQueuedResults();
    void watchdogReportsTimeout();
    void destroyingServiceDoesNotWaitForActiveWorker();
};

void TestRegexEvaluationService::latestRequestDiscardsStaleResults()
{
    devtools::RegexEvaluationService service;
    QSignalSpy results(&service, &devtools::RegexEvaluationService::matchingFinished);
    QVERIFY(results.isValid());

    service.evaluate(QStringLiteral("old"), QStringLiteral("old old"), QStringLiteral("x"),
                     QRegularExpression::NoPatternOption, true);
    service.evaluate(QStringLiteral("(?<word>new)"), QStringLiteral("old new"),
                     QStringLiteral("[$<word>]"), QRegularExpression::NoPatternOption, true);

    QVERIFY(results.wait(5000));
    QTest::qWait(50);
    QCOMPARE(results.size(), 1);
    const QList<QVariant> result = results.takeFirst();
    QCOMPARE(result.at(0).toInt(), 2);
    const auto matches = result.at(1).value<QVector<devtools::MatchResult>>();
    QCOMPARE(matches.size(), 1);
    QCOMPARE(matches.first().groups.at(1).name, QStringLiteral("word"));
    QCOMPARE(result.at(2).toString(), QStringLiteral("old [new]"));
    QVERIFY(result.at(3).toBool());
}

void TestRegexEvaluationService::cancellationDiscardsQueuedResults()
{
    devtools::RegexEvaluationService service;
    QSignalSpy results(&service, &devtools::RegexEvaluationService::matchingFinished);
    QVERIFY(results.isValid());
    service.evaluate(QStringLiteral("a"), QStringLiteral("a"), QStringLiteral("b"),
                     QRegularExpression::NoPatternOption, true);

    service.cancel();
    QTest::qWait(50);

    QCOMPARE(results.size(), 0);
}

void TestRegexEvaluationService::watchdogReportsTimeout()
{
    devtools::RegexEvaluationService service(nullptr, 0);
    QSignalSpy results(&service, &devtools::RegexEvaluationService::matchingFinished);
    QSignalSpy timeouts(&service, &devtools::RegexEvaluationService::evaluationTimedOut);
    QVERIFY(results.isValid());
    QVERIFY(timeouts.isValid());
    service.evaluate(QStringLiteral("a"), QStringLiteral("a"), QString(),
                     QRegularExpression::NoPatternOption, true);

    QVERIFY(timeouts.wait(5000));
    QCOMPARE(timeouts.size(), 1);
    QCOMPARE(timeouts.takeFirst().at(0).toInt(), 1);
    QTest::qWait(50);
    QCOMPARE(results.size(), 0);
}

void TestRegexEvaluationService::destroyingServiceDoesNotWaitForActiveWorker()
{
    QSemaphore workerStarted;
    QSemaphore resumeWorker;
    auto *const service = new BlockingRegexEvaluationService(workerStarted, resumeWorker);
    QSignalSpy timeouts(service, &devtools::RegexEvaluationService::evaluationTimedOut);
    QVERIFY(timeouts.isValid());

    service->evaluate(QStringLiteral("a"), QStringLiteral("a"), QString(),
                      QRegularExpression::NoPatternOption, true);
    auto *const worker = service->worker;
    const bool workerDidStart = workerStarted.tryAcquire(1, 5000);
    if (!workerDidStart) {
        resumeWorker.release();
        delete service;
        if (worker != nullptr) {
            worker->wait(5000);
            QTest::qWait(1);
        }
        QFAIL("The blocking worker did not start.");
    }

    std::thread resumeThread([&resumeWorker] {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        resumeWorker.release();
    });
    const bool timeoutObserved = timeouts.wait(5000);
    if (!timeoutObserved) {
        resumeThread.join();
        delete service;
        worker->wait(5000);
        QTest::qWait(1);
        QFAIL("The watchdog did not time out the blocked worker.");
    }

    QElapsedTimer elapsed;
    elapsed.start();
    delete service;
    const qint64 destructionTimeMs = elapsed.elapsed();
    resumeThread.join();
    QVERIFY(worker->wait(5000));
    QTest::qWait(1);
    QVERIFY2(destructionTimeMs < 50, "Service destruction waited for the regex worker.");
}

QTEST_GUILESS_MAIN(TestRegexEvaluationService)
#include "test_regex_evaluation_service.moc"

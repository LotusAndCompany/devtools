#include "features/regex_tool/core/regex_evaluation_service.h"

#include <QSignalSpy>
#include <QtTest>

class TestRegexEvaluationService : public QObject
{
    Q_OBJECT

private slots:
    void latestRequestDiscardsStaleResults();
    void cancellationDiscardsQueuedResults();
    void watchdogReportsTimeout();
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

QTEST_GUILESS_MAIN(TestRegexEvaluationService)
#include "test_regex_evaluation_service.moc"

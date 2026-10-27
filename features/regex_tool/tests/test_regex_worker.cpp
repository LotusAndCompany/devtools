#include "features/regex_tool/core/regex_worker.h"

#include <QSignalSpy>
#include <QtTest>

class TestRegexWorker : public QObject
{
    Q_OBJECT

private slots:
    void results_data();
    void results();
    void formatsMatchesForClipboard();
};

void TestRegexWorker::results_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("replacement");
    QTest::addColumn<bool>("global");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<int>("count");
    QTest::addColumn<QString>("replaced");

    QTest::newRow("global named capture")
        << QStringLiteral("(?<letter>あ)") << QStringLiteral("あ あ")
        << QStringLiteral("[$<letter>]") << true << true << 2 << QStringLiteral("[あ] [あ]");
    QTest::newRow("first only") << QStringLiteral("a") << QStringLiteral("a a")
                                << QStringLiteral("b") << false << true << 1
                                << QStringLiteral("b a");
    QTest::newRow("empty pattern") << QString() << QStringLiteral("abc") << QStringLiteral("x")
                                   << true << true << 0 << QStringLiteral("abc");
    QTest::newRow("invalid pattern")
        << QStringLiteral("(") << QStringLiteral("abc") << QStringLiteral("x") << true << false << 0
        << QStringLiteral("abc");
}

void TestRegexWorker::results()
{
    QFETCH(QString, pattern);
    QFETCH(QString, text);
    QFETCH(QString, replacement);
    QFETCH(bool, global);
    QFETCH(bool, valid);
    QFETCH(int, count);
    QFETCH(QString, replaced);

    qRegisterMetaType<QVector<devtools::MatchResult>>();
    devtools::RegexWorker worker;
    QSignalSpy spy(&worker, &devtools::RegexWorker::finishedMatching);
    worker.setParams(42, pattern, text, replacement, QRegularExpression::NoPatternOption, global);
    worker.start();
    QVERIFY(spy.wait(5000));
    QVERIFY(worker.wait(5000));
    QCOMPARE(spy.size(), 1);
    const auto result = spy.takeFirst();
    QCOMPARE(result[0].toInt(), 42);
    const auto matches = result[1].value<QVector<devtools::MatchResult>>();
    QCOMPARE(matches.size(), count);
    QCOMPARE(result[2].toString(), replaced);
    QCOMPARE(result[3].toBool(), valid);
    QCOMPARE(result[4].toString().isEmpty(), valid);
    if (pattern == QStringLiteral("(?<letter>あ)")) {
        QCOMPARE(matches[0].offset, 0);
        QCOMPARE(matches[1].offset, 2);
        QCOMPARE(matches[0].groups[1].name, QStringLiteral("letter"));
        QCOMPARE(matches[0].groups[1].value, QStringLiteral("あ"));
    }
}

void TestRegexWorker::formatsMatchesForClipboard()
{
    const QVector<devtools::MatchResult> matches = devtools::RegexTool::match(
        QStringLiteral("(a)(?<bee>b)"), QStringLiteral("ab"), QRegularExpression::NoPatternOption);
    const QString result = devtools::RegexTool::formatMatchesForClipboard(
        matches, QStringLiteral("ab"), QStringLiteral("Match Results (Count: %1)"),
        QStringLiteral("Match %1: \"%2\" [%3, %4]\n"),
        QStringLiteral("  Group %1: \"%2\" [%3, %4]\n"));

    QCOMPARE(result, QStringLiteral("Match Results (Count: 1)\n\n"
                                    "Match 1: \"ab\" [0, 2]\n"
                                    "  Group 1: \"a\" [0, 1]\n"
                                    "  Group 2 (bee): \"b\" [1, 2]\n\n"));
}

QTEST_GUILESS_MAIN(TestRegexWorker)
#include "test_regex_worker.moc"

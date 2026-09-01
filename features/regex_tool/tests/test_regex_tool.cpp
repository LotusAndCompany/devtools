#include "features/regex_tool/core/regex_tool.h"

#include <QtTest>

class TestRegexTool : public QObject
{
    Q_OBJECT

private slots:
    void testMatch_data();
    void testMatch();
    void testReplace_data();
    void testReplace();
};

void TestRegexTool::testMatch_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QString>("text");
    QTest::addColumn<int>("expectedMatches");

    QTest::newRow("simple match") << "abc" << "abc abc" << 2;
    QTest::newRow("no match") << "xyz" << "abc abc" << 0;
    QTest::newRow("empty pattern") << "" << "abc abc" << 0;
    QTest::newRow("capture groups") << "(a)(b)c" << "abc" << 1;
    QTest::newRow("named capture group") << R"((?<year>\d{4}))" << "2026" << 1;
}

void TestRegexTool::testMatch()
{
    QFETCH(QString, pattern);
    QFETCH(QString, text);
    QFETCH(int, expectedMatches);

    auto const matches =
        devtools::RegexTool::match(pattern, text, QRegularExpression::NoPatternOption);
    QCOMPARE(matches.size(), expectedMatches);

    if (pattern == "(a)(b)c" && expectedMatches == 1) {
        QCOMPARE(matches[0].groups.size(), 3); // Full match + 2 groups
        QCOMPARE(matches[0].groups[1].index, 1);
        QCOMPARE(matches[0].groups[1].value, QString("a"));
        QCOMPARE(matches[0].groups[2].index, 2);
        QCOMPARE(matches[0].groups[2].value, QString("b"));
    } else if (pattern == R"((?<year>\d{4}))" && expectedMatches == 1) {
        QCOMPARE(matches[0].groups.size(), 2); // Full match + named group
        QCOMPARE(matches[0].groups[1].index, 1);
        QCOMPARE(matches[0].groups[1].name, QString("year"));
        QCOMPARE(matches[0].groups[1].value, QString("2026"));
    }
}

void TestRegexTool::testReplace_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("replacement");
    QTest::addColumn<QString>("expectedResult");

    QTest::newRow("simple replace") << "abc" << "abc abc" << "xyz" << "xyz xyz";
    QTest::newRow("no match") << "123" << "abc abc" << "xyz" << "abc abc";
    QTest::newRow("empty pattern") << "" << "abc abc" << "xyz" << "abc abc";
    QTest::newRow("numbered capture group replacement")
        << R"((\w+)@(\w+))" << "alice@example" << "$2/$1" << "example/alice";
    QTest::newRow("named capture group replacement")
        << R"((?<user>\w+)@(?<domain>\w+))" << "alice@example" << "$<domain>/$<user>"
        << "example/alice";
    QTest::newRow("dollar sign escape") << "a" << "a" << "$$" << "$";
    QTest::newRow("full match token") << R"(\d+)" << "42" << "[$&]" << "[42]";
    QTest::newRow("capture value is not re-expanded") << R"((?<x>\$2))" << "$2" << "$<x>" << "$2";
    QTest::newRow("literal control char preserved")
        << "a" << "a" << QString(QChar(0x0001)) << QString(QChar(0x0001));
}

void TestRegexTool::testReplace()
{
    QFETCH(QString, pattern);
    QFETCH(QString, text);
    QFETCH(QString, replacement);
    QFETCH(QString, expectedResult);

    const QString result = devtools::RegexTool::replace(pattern, text, replacement,
                                                        QRegularExpression::NoPatternOption);
    QCOMPARE(result, expectedResult);
}

QTEST_MAIN(TestRegexTool)
#include "test_regex_tool.moc"

#include "core/regex_tool/regex_tool.h"

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
        QCOMPARE(matches[0].groups[1].value, QString("a"));
        QCOMPARE(matches[0].groups[2].value, QString("b"));
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

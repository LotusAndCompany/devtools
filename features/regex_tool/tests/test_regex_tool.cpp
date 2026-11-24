#include "features/regex_tool/core/regex_tool.h"
#include "features/regex_tool/gui/regex_tester_gui.h"

#include <QCoreApplication>
#include <QGroupBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSettings>
#include <QTemporaryDir>
#include <QToolButton>
#include <QtTest>

class TestRegexTool : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testMatch_data();
    void testMatch();
    void testReplace_data();
    void testReplace();
    void testReplaceSingle();
    void testGuiPaneTitles();
    void testGuiSettingsRestoredAndSaved();

private:
    QTemporaryDir settingsDirectory;
};

void TestRegexTool::initTestCase()
{
    QVERIFY(settingsDirectory.isValid());
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDirectory.path());
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, settingsDirectory.path());
    QCoreApplication::setOrganizationName(QStringLiteral("DevToolsRegexTest"));
    QCoreApplication::setApplicationName(QStringLiteral("test_regex_tool"));
}

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

void TestRegexTool::testReplaceSingle()
{
    const QString result = devtools::RegexTool::replace("abc", "abc abc", "xyz",
                                                        QRegularExpression::NoPatternOption, false);
    QCOMPARE(result, QString("xyz abc"));
}

void TestRegexTool::testGuiPaneTitles()
{
    devtools::RegexTesterGUI gui;
    const auto panes = gui.findChildren<QGroupBox *>();

    QStringList titles;
    for (const auto *const pane : panes) {
        titles.append(pane->title());
    }

    QVERIFY(titles.contains(QStringLiteral("Regex Tester")));
    QVERIFY(titles.contains(QStringLiteral("Matches")));
    QVERIFY(titles.contains(QStringLiteral("Quick Reference")));
}

void TestRegexTool::testGuiSettingsRestoredAndSaved()
{
    {
        QSettings settings;
        settings.clear();
        settings.setValue(QStringLiteral("RegexTester/pattern"), QStringLiteral("seed-pattern"));
        settings.setValue(QStringLiteral("RegexTester/testText"), QStringLiteral("seed-text"));
        settings.setValue(QStringLiteral("RegexTester/replacePattern"),
                          QStringLiteral("seed-replace"));
        settings.setValue(QStringLiteral("RegexTester/flagG"), true);
        settings.setValue(QStringLiteral("RegexTester/flagI"), false);
        settings.setValue(QStringLiteral("RegexTester/flagM"), false);
        settings.setValue(QStringLiteral("RegexTester/flagS"), false);
        settings.setValue(QStringLiteral("RegexTester/flagU"), false);
        settings.setValue(QStringLiteral("RegexTester/flagX"), false);
        settings.sync();
    }

    {
        devtools::RegexTesterGUI gui;
        auto *const pattern = gui.findChild<QLineEdit *>(QStringLiteral("regexPattern"));
        auto *const testText = gui.findChild<QPlainTextEdit *>(QStringLiteral("regexTestText"));
        auto *const replacement = gui.findChild<QLineEdit *>(QStringLiteral("regexReplacement"));
        auto *const global = gui.findChild<QToolButton *>(QStringLiteral("regexFlagGlobal"));
        auto *const insensitive =
            gui.findChild<QToolButton *>(QStringLiteral("regexFlagCaseInsensitive"));
        auto *const multiline = gui.findChild<QToolButton *>(QStringLiteral("regexFlagMultiline"));
        auto *const dotMatches =
            gui.findChild<QToolButton *>(QStringLiteral("regexFlagDotMatchesEverything"));
        auto *const unicode =
            gui.findChild<QToolButton *>(QStringLiteral("regexFlagUnicodeProperties"));
        auto *const extended =
            gui.findChild<QToolButton *>(QStringLiteral("regexFlagExtendedSyntax"));
        QVERIFY(pattern != nullptr);
        QVERIFY(testText != nullptr);
        QVERIFY(replacement != nullptr);
        QVERIFY(global != nullptr);
        QVERIFY(insensitive != nullptr);
        QVERIFY(multiline != nullptr);
        QVERIFY(dotMatches != nullptr);
        QVERIFY(unicode != nullptr);
        QVERIFY(extended != nullptr);
        QCOMPARE(pattern->text(), QStringLiteral("seed-pattern"));
        QCOMPARE(testText->toPlainText(), QStringLiteral("seed-text"));
        QCOMPARE(replacement->text(), QStringLiteral("seed-replace"));
        QVERIFY(global->isChecked());
        QVERIFY(!insensitive->isChecked());

        pattern->setText(QStringLiteral("(a)"));
        testText->setPlainText(QStringLiteral("a"));
        replacement->setText(QStringLiteral("$1!"));
        global->setChecked(false);
        insensitive->setChecked(true);
        multiline->setChecked(true);
        dotMatches->setChecked(true);
        unicode->setChecked(true);
        extended->setChecked(true);
    }

    QSettings settings;
    QCOMPARE(settings.value(QStringLiteral("RegexTester/pattern")).toString(),
             QStringLiteral("(a)"));
    QCOMPARE(settings.value(QStringLiteral("RegexTester/testText")).toString(),
             QStringLiteral("a"));
    QCOMPARE(settings.value(QStringLiteral("RegexTester/replacePattern")).toString(),
             QStringLiteral("$1!"));
    QVERIFY(!settings.value(QStringLiteral("RegexTester/flagG")).toBool());
    QVERIFY(settings.value(QStringLiteral("RegexTester/flagI")).toBool());
    QVERIFY(settings.value(QStringLiteral("RegexTester/flagM")).toBool());
    QVERIFY(settings.value(QStringLiteral("RegexTester/flagS")).toBool());
    QVERIFY(settings.value(QStringLiteral("RegexTester/flagU")).toBool());
    QVERIFY(settings.value(QStringLiteral("RegexTester/flagX")).toBool());
}

QTEST_MAIN(TestRegexTool)
#include "test_regex_tool.moc"

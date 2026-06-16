#include "tests/test_util.h"

#include <QtTest>

#define _TEST_StatusLabel
#include "gui/components/status_label.h"
#undef _TEST_StatusLabel

namespace Test {
class TestStatusLabel : public QObject
{
    Q_OBJECT

private slots:
    static void test_constructorDefaults();
    static void test_showMessageError();
    static void test_showMessageWarning();
    static void test_showMessageSuccess();
    static void test_clear();
    static void test_setLevel();
};

void TestStatusLabel::test_constructorDefaults()
{
    StatusLabel label;
    QCOMPARE(label.isVisible(), false);
    QCOMPARE(label.text(), QString());
}

void TestStatusLabel::test_showMessageError()
{
    StatusLabel label;
    label.showMessage(StatusLabel::Level::Error, QStringLiteral("Error occurred"));
    QCOMPARE(label.isVisible(), true);
    QVERIFY(!label.text().isEmpty());
}

void TestStatusLabel::test_showMessageWarning()
{
    StatusLabel label;
    label.showMessage(StatusLabel::Level::Warning, QStringLiteral("Warning"));
    QCOMPARE(label.isVisible(), true);
}

void TestStatusLabel::test_showMessageSuccess()
{
    StatusLabel label;
    label.showMessage(StatusLabel::Level::Success, QStringLiteral("Success"));
    QCOMPARE(label.isVisible(), true);
}

void TestStatusLabel::test_clear()
{
    StatusLabel label;
    label.showMessage(StatusLabel::Level::Error, QStringLiteral("Error"));
    label.clear();
    QCOMPARE(label.isVisible(), false);
    QCOMPARE(label.text(), QString());
}

void TestStatusLabel::test_setLevel()
{
    StatusLabel label;
    label.setLevel(StatusLabel::Level::Warning);
    // Level change without showMessage should not make visible
    QCOMPARE(label.isVisible(), false);
}
} // namespace Test

QTEST_MAIN(Test::TestStatusLabel)

#include "test_status_label.moc"

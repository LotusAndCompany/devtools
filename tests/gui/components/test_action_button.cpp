#include "tests/test_util.h"

#include <QtTest>

#define _TEST_ActionButton
#include "gui/components/action_button.h"
#undef _TEST_ActionButton

namespace Test {
class TestActionButton : public QObject
{
    Q_OBJECT

private slots:
    static void test_constructorDefaults();
    static void test_setIcon();
    static void test_setText();
    static void test_kPaddingStyle();
};

void TestActionButton::test_constructorDefaults()
{
    ActionButton button(QIcon(), QStringLiteral("Click Me"));
    QCOMPARE(button.text(), QStringLiteral("Click Me"));
    QVERIFY(button.isEnabled());
}

void TestActionButton::test_setIcon()
{
    ActionButton button(QIcon(), QStringLiteral("Click Me"));
    button.setIcon(QIcon());
    QVERIFY(true); // verify no crash
}

void TestActionButton::test_setText()
{
    ActionButton button(QIcon(), QStringLiteral("Click Me"));
    button.setText(QStringLiteral("New Text"));
    QCOMPARE(button.text(), QStringLiteral("New Text"));
}

void TestActionButton::test_kPaddingStyle()
{
    QVERIFY(qstrlen(ActionButton::kPaddingStyle) > 0);
}
} // namespace Test

QTEST_MAIN(Test::TestActionButton)

#include "test_action_button.moc"

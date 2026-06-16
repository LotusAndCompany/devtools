#include "tests/test_util.h"

#include <QtTest>

#define _TEST_LabeledSpinbox
#include "gui/components/labeled_spinbox.h"
#undef _TEST_LabeledSpinbox

namespace Test {
class TestLabeledSpinbox : public QObject
{
    Q_OBJECT

private slots:
    static void test_constructorDefaults();
    static void test_setValue();
    static void test_setRange();
    static void test_valueChangedSignal();
};

void TestLabeledSpinbox::test_constructorDefaults()
{
    LabeledSpinbox spinbox(QStringLiteral("Count"));
    QCOMPARE(spinbox.value(), 0);
    QVERIFY(spinbox.spinBox() != nullptr);
}

void TestLabeledSpinbox::test_setValue()
{
    LabeledSpinbox spinbox(QStringLiteral("Count"));
    spinbox.setValue(42);
    QCOMPARE(spinbox.value(), 42);
}

void TestLabeledSpinbox::test_setRange()
{
    LabeledSpinbox spinbox(QStringLiteral("Count"));
    spinbox.setRange(10, 100);
    spinbox.setValue(50);
    QCOMPARE(spinbox.value(), 50);
}

void TestLabeledSpinbox::test_valueChangedSignal()
{
    LabeledSpinbox spinbox(QStringLiteral("Count"));
    int received = -1;

    connect(&spinbox, &LabeledSpinbox::valueChanged, [&](int value) { received = value; });

    spinbox.setValue(7);
    QCOMPARE(received, 7);
}
} // namespace Test

QTEST_MAIN(Test::TestLabeledSpinbox)

#include "test_labeled_spinbox.moc"

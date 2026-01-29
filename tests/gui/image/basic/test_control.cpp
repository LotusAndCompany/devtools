#include <QtTest>

#define _TEST_BasicImageViewControl
#include "gui/image/basic/control.h"
#undef _TEST_BasicImageViewControl

#include <QSignalSpy>

namespace Test {
class TestBasicImageViewControl : public QObject
{
    Q_OBJECT

private slots:
    // Test cases:
    static void test_constructor();
    void test_reset();
    // NOTE: dialogが上手く閉じられない
    static void test_load();
    static void test_save();
};

void TestBasicImageViewControl::test_constructor()
{
    BasicImageViewControl const control;

    /*
    QCOMPARE_EQ(control.ui->loadButton->text(), tr("Load"));
    QCOMPARE_EQ(control.ui->resetButton->text(), tr("Reset"));
    QCOMPARE_EQ(control.ui->saveButton->text(), tr("Save"));
    */

    QCOMPARE_EQ(control.saveFileDialogType, BasicImageViewControl::DialogType::SELECT_FILE);
}

void TestBasicImageViewControl::test_reset()
{
    /*
    BasicImageViewControl control;

    QSignalSpy spy(&control, &BasicImageViewControl::resetButtonClicked);
    QCOMPARE_EQ(spy.count(), 0);
    QTest::mouseClick(control.ui->resetButton, Qt::MouseButton::LeftButton);
    QCOMPARE_EQ(spy.count(), 1);*/
}

void TestBasicImageViewControl::test_load()
{
    QSKIP("how can we close QFileDialog?");

    BasicImageViewControl const control;

    // QTest::mouseClick(control.ui->loadButton, Qt::MouseButton::LeftButton);
}

void TestBasicImageViewControl::test_save()
{
    QSKIP("how can we close QFileDialog?");

    BasicImageViewControl const control;

    // QTest::mouseClick(control.ui->saveButton, Qt::MouseButton::LeftButton);
}
} // namespace Test

QTEST_MAIN(Test::TestBasicImageViewControl)

#include "test_control.moc"

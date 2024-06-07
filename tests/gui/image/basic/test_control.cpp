#include <QTest>

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
    void test_constructor();
    void test_reset();
    // NOTE: dialogが上手く閉じられない
    void test_load();
    void test_save();
};

void TestBasicImageViewControl::test_constructor()
{
    BasicImageViewControl control;

    QCOMPARE_EQ(control.ui->loadButton->text(), tr("Load"));
    QCOMPARE_EQ(control.ui->resetButton->text(), tr("Reset"));
    QCOMPARE_EQ(control.ui->saveButton->text(), tr("Save"));

    QCOMPARE_EQ(control.saveFileDailogType, BasicImageViewControl::DialogType::SELECT_FILE);
}

void TestBasicImageViewControl::test_reset()
{
    BasicImageViewControl control;

    QSignalSpy spy(&control, &BasicImageViewControl::resetButtonClicked);
    QCOMPARE_EQ(spy.count(), 0);
    QTest::mouseClick(control.ui->resetButton, Qt::MouseButton::LeftButton);
    QCOMPARE_EQ(spy.count(), 1);
}

void TestBasicImageViewControl::test_load()
{
    QSKIP("how can we close QFileDialog?");

    BasicImageViewControl control;

    QTest::mouseClick(control.ui->loadButton, Qt::MouseButton::LeftButton);
}

void TestBasicImageViewControl::test_save()
{
    QSKIP("how can we close QFileDialog?");

    BasicImageViewControl control;

    QTest::mouseClick(control.ui->saveButton, Qt::MouseButton::LeftButton);
}
}

QTEST_MAIN(Test::TestBasicImageViewControl)

#include "test_control.moc"

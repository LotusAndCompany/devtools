#include "tests/test_util.h"

#include <QtTest>

#define _TEST_ColorSample
// necessary includes here
#include "gui/image/transparent/color_sample.h"
#undef _TEST_ColorSample

#include <QObject>

namespace Test {
class TestColorSample : public QObject
{
    Q_OBJECT

    RandomData rd;

private slots:
    // Test cases:
    void test_constructor();
    void test_setColor();
};

void TestColorSample::test_constructor()
{
    ColorSample gui;

    // 枠のスタイルがQFrame::Panel | QFrame::Raisedであること
    QCOMPARE_EQ(gui.frameStyle(), QFrame::Panel | QFrame::Raised);

    // 色の初期値が白であること
    QCOMPARE_EQ(gui.color(), QColorConstants::White);
}

void TestColorSample::test_setColor()
{
    ColorSample gui;

    const QColor color1(rd.nextInt(256), rd.nextInt(256), rd.nextInt(256));
    gui.setColor(color1);
    // 指定した色が設定されていること
    QCOMPARE_EQ(gui.color(), color1);

    const QColor color2(rd.nextInt(256), rd.nextInt(256), rd.nextInt(256), rd.nextInt(255));
    gui.setColor(color2);
    // 透明度を無視して指定した色が設定されていること
    QCOMPARE_EQ(gui.color(), QColor(color2.red(), color2.green(), color2.blue()));
}
} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
// QTEST_APPLESS_MAIN(Test::TestColorSample)

// QCoreApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
// QTEST_GUILESS_MAIN(Test::TestColorSample)

// QApplicationもしくはQGuiApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
QTEST_MAIN(Test::TestColorSample)

#include "test_color_sample.moc"

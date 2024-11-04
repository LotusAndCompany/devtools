#include <QtTest>
#include "tests/test_util.h"

#define _TEST_ImageViewForImageDivision
#include "gui/image/division/image_view_for_image_division.h"
#ifdef _TEST_ImageViewForImageDivision
namespace Test {
class TestImageViewForImageDivision;
}
#endif

#include <QObject>

namespace Test {
class TestImageViewForImageDivision : public QObject
{
    Q_OBJECT

    RandomData rd;

private slots:
    void initTestCase() {}      // will be called before the first test function is executed.
    void initTestCase_data() {} // will be called to create a global test data table.
    void cleanupTestCase() {}   // will be called after the last test function was executed.
    void init() {}              // will be called before each test function is executed.
    void cleanup() {}           // will be called after every test function.

    // Test cases:
    void test_constructor();
    void test_ui_image();
    void test_setGridSize();
    void test_updateScale();
};

void TestImageViewForImageDivision::test_constructor()
{
    /*
    ImageViewForImageDivision gui;

    // LabelWithGridに置き換わっていること
    QVERIFY(gui.ui->image->metaObject()->metaType().name()
            == QString("_ImageViewForImageDivisionInternal::LabelWithGrid"));

    // 親ウィジェットが設定されていること(直接の親ではなくなっている)
    QCOMPARE_EQ(gui.findChild<_ImageViewForImageDivisionInternal::LabelWithGrid *>(), gui.ui->image);

    // 中心に描画するように設定されていること
    QCOMPARE_EQ(gui.ui->image->alignment(), Qt::AlignVCenter | Qt::AlignHCenter);

    // スクロールするように設定されていること
    QCOMPARE_EQ(gui.ui->scrollArea->widget(), gui.ui->image);

    // 画像が設定されていないこと
    QVERIFY(gui.ui->image->pixmap().isNull());
    QCOMPARE_EQ(gui.ui->image->text(), tr("No Image"));

    // scaleが1であること
    QCOMPARE_EQ(gui.scale, 1);

    // gridSizeが空であること
    QCOMPARE_EQ(gui.gridSize, QSizeF());

    // gui.ui_image()->gridSizeが空であること
    QCOMPARE_EQ(gui.ui_image()->gridSize, QSizeF());*/
}

void TestImageViewForImageDivision::test_ui_image()
{ /*
    ImageViewForImageDivision gui;

    // ui_image()の帰り値とui.imageが同一であること
    QCOMPARE_EQ(gui.ui_image(), gui.ui->image);*/
}

void TestImageViewForImageDivision::test_setGridSize()
{
    ImageViewForImageDivision gui;

    const QSizeF sizef(rd.nextDouble(16, 512), rd.nextDouble(16, 512));

    gui.scale = rd.nextDouble(0.1, 10);
    gui.setGridSize(sizef);
    // gui.gridSizeにはそのまま、gui.ui_image()->gridSizeにはscaleが適用されたサイズが設定されていること
    QCOMPARE_EQ(gui.gridSize, sizef);
    QCOMPARE_EQ(gui.ui_image()->gridSize, sizef.scaled(gui.scale * sizef, Qt::KeepAspectRatio));
}

void TestImageViewForImageDivision::test_updateScale()
{
    ImageViewForImageDivision gui;

    const QSizeF sizef(rd.nextDouble(16, 512), rd.nextDouble(16, 512));

    gui.setGridSize(sizef);
    QCOMPARE_EQ(gui.gridSize, sizef);
    QCOMPARE_EQ(gui.ui_image()->gridSize, sizef);

    const float scale = rd.nextDouble(0.1, 10);
    gui.updateScale(scale);
    // gui.gridSizeはそのまま、gui.ui_image()->gridSizeにはscaleが適用されること
    QCOMPARE_EQ(gui.gridSize, sizef);
    QCOMPARE_EQ(gui.ui_image()->gridSize, sizef.scaled(gui.scale * sizef, Qt::KeepAspectRatio));
}
} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
//QTEST_APPLESS_MAIN(Test::TestImageViewForImageDivision)

// QCoreApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
//QTEST_GUILESS_MAIN(Test::TestImageViewForImageDivision)

// QApplicationもしくはQGuiApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
QTEST_MAIN(Test::TestImageViewForImageDivision)

#include "test_image_view_for_image_division.moc"

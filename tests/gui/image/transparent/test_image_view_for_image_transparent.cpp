#include <QtTest>
#include "tests/test_util.h"

#define _TEST_ImageViewForImageTransparent
#include "gui/image//transparent/image_view_for_image_transparent.h"
#undef _TEST_ImageViewForImageTransparent

#include <QObject>
#include <QSignalSpy>

namespace Test {
class TestImageViewForImageTransparent : public QObject
{
    Q_OBJECT

    RandomData rd;

private slots:
    // Test cases:
    void test_constructor();
    void test_ui_image();
    void test_onLabelClicked();
};

void TestImageViewForImageTransparent::test_constructor()
{
    /*
    ImageViewForImageTransparent gui;

    // ClickableLabelに置き換わっていること
    QVERIFY(gui.ui->image->metaObject()->metaType().name()
            == QString("_ImageViewForImageTransparentInternal::ClickableLabel"));

    // 親ウィジェットが設定されていること(直接の親ではなくなっている)
    QCOMPARE_EQ(gui.findChild<_ImageViewForImageTransparentInternal::ClickableLabel *>(),
                gui.ui->image);

    // 中心に描画するように設定されていること
    QCOMPARE_EQ(gui.ui->image->alignment(), Qt::AlignVCenter | Qt::AlignHCenter);

    // スクロールするように設定されていること
    QCOMPARE_EQ(gui.ui->scrollArea->widget(), gui.ui->image);

    // 画像が設定されていないこと
    QVERIFY(gui.ui->image->pixmap().isNull());
    QCOMPARE_EQ(gui.ui->image->text(), tr("No Image"));

    // scaleが1であること
    QCOMPARE_EQ(gui.scale, 1);*/
}

void TestImageViewForImageTransparent::test_ui_image()
{
    /*
    ImageViewForImageTransparent gui;

    // ui_image()の帰り値とui.imageが同一であること
    QCOMPARE_EQ(gui.ui_image(), gui.ui->image);*/
}

void TestImageViewForImageTransparent::test_onLabelClicked()
{
    ImageViewForImageTransparent gui;
    QSignalSpy spyOnPixelSelected(&gui, &ImageViewForImageTransparent::pixelSelected);

    QImage image(QSize(8, 8), QImage::Format_RGBA8888);
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
            image.setPixelColor(x, y, QColor(rd.nextInt(256), rd.nextInt(256), rd.nextInt(256)));

    const QSize labelSize = gui.ui_image()->size();
    gui.setPixmap(QPixmap::fromImage(image));

    QTest::mouseClick(gui.ui_image(),
                      Qt::LeftButton,
                      Qt::KeyboardModifiers(),
                      QPoint(labelSize.width() / 2 - 4, labelSize.height() / 2 - 4));

    // pixelSelectedが発せられていること
    QCOMPARE_EQ(spyOnPixelSelected.count(), 1);
    QList<QVariant> params = spyOnPixelSelected[0];
    // 画像の左上の座標と色とが渡されること
    QCOMPARE_EQ(params[0].value<QPoint>(), QPoint(0, 0));
    QCOMPARE_EQ(params[1].value<QColor>(), image.pixelColor(0, 0));

    gui.updateScale(2.0);
    QTest::mouseClick(gui.ui_image(),
                      Qt::LeftButton,
                      Qt::KeyboardModifiers(),
                      QPoint(labelSize.width() / 2 + 7, labelSize.height() / 2 + 7));
    QCOMPARE_EQ(spyOnPixelSelected.count(), 2);
    params = spyOnPixelSelected[1];
    // 画像の右下の座標と色とが渡されること
    QCOMPARE_EQ(params[0].value<QPoint>(), QPoint(7, 7));
    QCOMPARE_EQ(params[1].value<QColor>(), image.pixelColor(7, 7));
}
} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
//QTEST_APPLESS_MAIN(Test::TestImageViewForImageTransparent)

// QCoreApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
//QTEST_GUILESS_MAIN(Test::TestImageViewForImageTransparent)

// QApplicationもしくはQGuiApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
QTEST_MAIN(Test::TestImageViewForImageTransparent)

#include "test_image_view_for_image_transparent.moc"

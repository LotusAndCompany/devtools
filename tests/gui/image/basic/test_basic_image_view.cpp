#include <QtTest>
#include "tests/test_util.h"

#define _TEST_BasicImageView
#include "gui/image/basic/image_view.h"
#undef _TEST_BasicImageView

#include <QDir>
#include <QFile>
#include <QPixmap>

namespace Test {
class TestBasicImageView : public QObject
{
    Q_OBJECT

    const QString testDirName = "test_basic_image_view_resources";
    const QString testDirPath = TEST_BIN_DIR + "/" + testDirName + "/";
    const QStringList resourceNames = {
        "320px-Qt_logo_2016.png", // 0
        "578px-Qt_logo_2016.png", // 1
    };

    QPixmap pixmap320, pixmap578;

private slots:
    void initTestCase();    // will be called before the first test function is executed.
    void cleanupTestCase(); // will be called after the last test function was executed.

    // Test cases:
    void test_setPixmap();
    void test_resizeEvent();
    void test_zoomIn();
    void test_zoomOut();
    void test_updateScale();
};

void TestBasicImageView::initTestCase()
{
    QDir dir(TEST_BIN_DIR);
    dir.mkpath(testDirName);

    for (const QString &src : resourceNames)
        QFile::copy(TEST_SRC_DIR + "/core/image/" + src, testDirPath + src);

    pixmap320 = std::move(QPixmap(testDirPath + resourceNames[0]));
    pixmap578 = std::move(QPixmap(testDirPath + resourceNames[1]));
}

void TestBasicImageView::cleanupTestCase()
{
    QDir testDir(testDirPath);
    testDir.removeRecursively();
}

void TestBasicImageView::test_setPixmap()
{
    BasicImageView imageView;

    const QSize size320 = pixmap320.size();
    const QSize size578 = pixmap578.size();

    // 初期状態で画像が空であること
    QVERIFY(imageView.original.isNull());
    QVERIFY(imageView.ui->image->pixmap().isNull());
    QVERIFY(imageView.ui->image->text() == tr("No Image"));

    // 適当な拡大率を設定しておく
    imageView.scale = 2.0;

    // 画像を設定できること
    imageView.setPixmap(pixmap320, false);
    QVERIFY(imageView.original.size() == size320);
    QVERIFY(imageView.ui->image->text() == "");
    // 拡大率が反映されていること
    QVERIFY(imageView.scale == 2.0);
    QVERIFY(imageView.ui->image->pixmap().size() == 2.0 * size320);

    // 別の画像を設定できること
    imageView.setPixmap(pixmap578, false);
    QVERIFY(imageView.original.size() == size578);
    // 拡大率が反映されていること
    QVERIFY(imageView.scale == 2.0);
    QVERIFY(imageView.ui->image->pixmap().size() == 2.0 * size578);

    // 空の画像を設定できること
    imageView.setPixmap(QPixmap(), false);
    QVERIFY(imageView.original.isNull());
    // 拡大率が反映されていること
    QVERIFY(imageView.scale == 2.0);
    QVERIFY(imageView.ui->image->pixmap().isNull());
    QVERIFY(imageView.ui->image->text() == tr("No Image"));

    // 別の画像を設定できること
    imageView.setPixmap(pixmap320, true);
    QVERIFY(imageView.original.size() == size320);
    QVERIFY(imageView.ui->image->text() == "");
    // 拡大率が1.0に戻ること
    QVERIFY(imageView.scale == 1.0);
    QVERIFY(imageView.ui->image->pixmap().size() == size320);
}

void TestBasicImageView::test_resizeEvent() {}

void TestBasicImageView::test_zoomIn() {}

void TestBasicImageView::test_zoomOut() {}

void TestBasicImageView::test_updateScale() {}
} // namespace Test

QTEST_MAIN(Test::TestBasicImageView)

#include "test_basic_image_view.moc"

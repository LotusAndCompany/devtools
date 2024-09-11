#include <QtTest>
#include "tests/test_util.h"

#define _TEST_BasicImageView
#include "gui/image/basic/image_view.h"
#undef _TEST_BasicImageView

#include <QDir>
#include <QFile>
#include <QMap>
#include <QPixmap>
#include <QResizeEvent>

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

    static bool isLinear(const QMap<double, double> &pairs, double errorRatio = 1.0 / 256.0);

private slots:
    void initTestCase();    // will be called before the first test function is executed.
    void cleanupTestCase(); // will be called after the last test function was executed.

    // Test cases:
    void test_constructor();
    void test_setPixmap();
    void test_resizeEvent();
    void test_zoomIn();
    void test_zoomOut();
    void test_updateScale();
    void test_zoomInButton();
    void test_zoomOutButton();
};

void TestBasicImageView::initTestCase()
{
    QDir dir(TEST_BIN_DIR);
    dir.mkpath(testDirName);

    for (const QString &src : resourceNames)
        QFile::copy(TEST_SRC_DIR + "/core/image/" + src, testDirPath + src);

    pixmap320 = QPixmap(testDirPath + resourceNames[0]);
    pixmap578 = QPixmap(testDirPath + resourceNames[1]);
}

void TestBasicImageView::cleanupTestCase()
{
    QDir testDir(testDirPath);
    testDir.removeRecursively();
}

void TestBasicImageView::test_constructor()
{
    BasicImageView imageView;

    // scaleが1.0で初期化されていること
    QCOMPARE_EQ(imageView.scale, 1.0);

    // "No Image"が設定されていること
    QCOMPARE_EQ(imageView.ui->image->text(), tr("No Image"));

    // 拡大縮小ボタンが有効であること
    QVERIFY(imageView.ui->zoomInButton->isEnabled());
    QVERIFY(imageView.ui->zoomOutButton->isEnabled());
}

void TestBasicImageView::test_setPixmap()
{
    BasicImageView imageView;

    const QSize size320 = pixmap320.size();
    const QSize size578 = pixmap578.size();

    // 初期状態で画像が空であること
    QVERIFY(imageView.original.isNull());
    QVERIFY(imageView.ui->image->pixmap().isNull());
    QCOMPARE_EQ(imageView.ui->image->text(), tr("No Image"));

    // 適当な拡大率を設定しておく
    imageView.scale = 2.0;

    // 画像を設定できること
    imageView.setPixmap(pixmap320, false);
    QCOMPARE_EQ(imageView.original.size(), size320);
    QCOMPARE_EQ(imageView.ui->image->text(), "");
    // 拡大率が反映されていること
    QCOMPARE_EQ(imageView.scale, 2.0);
    QCOMPARE_EQ(imageView.ui->image->pixmap().size(), 2.0 * size320);

    // 別の画像を設定できること
    imageView.setPixmap(pixmap578, false);
    QCOMPARE_EQ(imageView.original.size(), size578);
    // 拡大率が反映されていること
    QCOMPARE_EQ(imageView.scale, 2.0);
    QCOMPARE_EQ(imageView.ui->image->pixmap().size(), 2.0 * size578);

    // 空の画像を設定できること
    imageView.setPixmap(QPixmap(), false);
    QVERIFY(imageView.original.isNull());
    // 拡大率が反映されていること
    QCOMPARE_EQ(imageView.scale, 2.0);
    QVERIFY(imageView.ui->image->pixmap().isNull());
    QCOMPARE_EQ(imageView.ui->image->text(), tr("No Image"));

    // 別の画像を設定できること
    imageView.setPixmap(pixmap320, true);
    QCOMPARE_EQ(imageView.original.size(), size320);
    QCOMPARE_EQ(imageView.ui->image->text(), "");
    // 拡大率が1.0に戻ること
    QCOMPARE_EQ(imageView.scale, 1.0);
    QCOMPARE_EQ(imageView.ui->image->pixmap().size(), size320);
}

void TestBasicImageView::test_resizeEvent()
{
    BasicImageView imageView;
    imageView.setPixmap(pixmap320);
    const QSize originalSize = imageView.size();
    const QSize scalingUISize = imageView.ui->scalingUI->size();

    QResizeEvent shrink(originalSize / 2, originalSize);
    imageView.resizeEvent(&shrink);

    // 縮小した時にUIが想定通り調整されていること
    QCOMPARE_EQ(imageView.ui->scrollArea->size(), shrink.size());
    QCOMPARE_EQ(imageView.ui->scalingUI->pos(),
                QPoint(shrink.size().width() - scalingUISize.width(),
                       shrink.size().height() - scalingUISize.height()));

    QResizeEvent expand(originalSize * 2, originalSize / 2);
    imageView.resizeEvent(&expand);
    // 拡大した時にUIが想定通り調整されていること
    QCOMPARE_EQ(imageView.ui->scrollArea->size(), expand.size());
    QCOMPARE_EQ(imageView.ui->scalingUI->pos(),
                QPoint(expand.size().width() - scalingUISize.width(),
                       expand.size().height() - scalingUISize.height()));
}

bool TestBasicImageView::isLinear(const QMap<double, double> &pairs, double errorRatio)
{
    if (errorRatio < 0) {
        qCritical() << "errorRatio must be grater or equal to 0";
        return false;
    }
    if (pairs.size() <= 2)
        return true;

    // 最小二乗法による直線近似
    const auto keys = pairs.keys();
    const auto values = pairs.values();
    const double ax = std::accumulate(keys.cbegin(), keys.cend(), 0.0) / keys.size();
    const double ay = std::accumulate(values.cbegin(), values.cend(), 0.0) / values.size();
    double sx2 = 0.0, sxy = 0.0;

    for (const auto [x, y] : pairs.asKeyValueRange()) {
        sx2 += (x - ax) * (x - ax);
        sxy += (x - ax) * (y - ay);
    }

    // y=ax+b
    const double a = sxy / sx2;
    const double b = ay - a * ax;

    const double max = 1.0 + errorRatio;
    const double min = 1.0 - errorRatio;

    qDebug() << "a=" << a << ", b=" << b;

    for (const auto [x, y] : pairs.asKeyValueRange()) {
        if (y == 0.0) {
            qWarning() << "(" << x << ',' << y << ") appeared in value pairs, skip";
            continue;
        }

        double v = a * x + b;
        double r = abs(v / y);
        if (r < min || max < r) {
            qWarning() << "expected: (" << x << ", " << v << "), actual: (" << x << ", " << y
                       << "), error ratio: " << r;
            return false;
        }
    }

    return true;
}

void TestBasicImageView::test_zoomIn()
{
    BasicImageView imageView;

    imageView.setPixmap(pixmap320);
    QList<double> scaleValues;
    while (imageView.scale < BasicImageView::maxScale) {
        imageView.zoomIn();
        scaleValues.append(imageView.scale);
    }
    scaleValues.removeLast();

    // 対数スケールで拡大されていること
    QMap<double, double> values;
    for (int i = 0; i < scaleValues.size(); i++) {
        values[(double) i] = log(scaleValues[i]);
    }
    QVERIFY(isLinear(values));

    // BasicImageView::maxScaleで終了していること
    QCOMPARE_EQ(imageView.scale, BasicImageView::maxScale);

    // BasicImageView::maxScaleを超えないこと
    imageView.zoomIn();
    QVERIFY(imageView.scale <= BasicImageView::maxScale);

    // BasicImageView::minScaleから拡大できること
    imageView.scale = BasicImageView::minScale;
    imageView.zoomIn();
    QVERIFY(BasicImageView::minScale < imageView.scale);
}

void TestBasicImageView::test_zoomOut()
{
    BasicImageView imageView;

    imageView.setPixmap(pixmap320);
    QList<double> scaleValues;
    while (BasicImageView::minScale < imageView.scale) {
        imageView.zoomOut();
        scaleValues.append(imageView.scale);
    }
    scaleValues.removeLast();

    // 対数スケールで縮小されていること
    QMap<double, double> values;
    for (int i = 0; i < scaleValues.size(); i++) {
        values[(double) i] = log(scaleValues[i]);
    }
    QVERIFY(isLinear(values));

    // BasicImageView::minScaleで終了していること
    QCOMPARE_EQ(imageView.scale, BasicImageView::minScale);

    // BasicImageView::minScaleを下回らないこと
    imageView.zoomOut();
    QVERIFY(BasicImageView::minScale <= imageView.scale);

    // BasicImageView::maxScaleから縮小できること
    imageView.scale = BasicImageView::maxScale;
    imageView.zoomOut();
    QVERIFY(imageView.scale < BasicImageView::maxScale);
}

void TestBasicImageView::test_updateScale()
{
    BasicImageView imageView;

    // 画像が設定されていない場合は"No Image"が設定されること
    imageView.updateScale(1.0);
    QCOMPARE_EQ(imageView.ui->image->text(), tr("No Image"));

    const QSize size320 = pixmap320.size();
    imageView.setPixmap(pixmap320);

    // 画像が拡大されること
    imageView.updateScale(1.5);
    QCOMPARE_EQ(imageView.scale, 1.5);
    QCOMPARE_EQ(imageView.ui->image->pixmap().size(), 1.5 * size320);
    QCOMPARE_EQ(imageView.ui->scaleLabel->text(), "x1.5");

    // 画像が縮小されること
    imageView.updateScale(0.5);
    QCOMPARE_EQ(imageView.scale, 0.5);
    QCOMPARE_EQ(imageView.ui->image->pixmap().size(), 0.5 * size320);
    QCOMPARE_EQ(imageView.ui->scaleLabel->text(), "x0.5");

    // 最大の拡大率の時に拡大ボタンが無効化されること
    imageView.updateScale(BasicImageView::maxScale);
    QCOMPARE_EQ(imageView.ui->zoomInButton->isEnabled(), false);
    QCOMPARE_EQ(imageView.ui->zoomOutButton->isEnabled(), true);

    // 最小の拡大率の時に縮小ボタンが無効化されること
    imageView.updateScale(BasicImageView::minScale);
    QCOMPARE_EQ(imageView.ui->zoomInButton->isEnabled(), true);
    QCOMPARE_EQ(imageView.ui->zoomOutButton->isEnabled(), false);

    // それ以外の拡大率の時に拡大ボタン・縮小ボタンが有効化されること
    imageView.updateScale(1.0);
    QCOMPARE_EQ(imageView.ui->zoomInButton->isEnabled(), true);
    QCOMPARE_EQ(imageView.ui->zoomOutButton->isEnabled(), true);
}

void TestBasicImageView::test_zoomInButton()
{
    BasicImageView imageView;

    // 画像が設定されていない時は拡大率が変わらないこと
    QTest::mouseClick(imageView.ui->zoomInButton, Qt::MouseButton::LeftButton);
    QCOMPARE_EQ(imageView.scale, 1.0);

    imageView.setPixmap(pixmap320);

    // 画像を拡大できること
    QTest::mouseClick(imageView.ui->zoomInButton, Qt::MouseButton::LeftButton);
    QVERIFY(1.0 < imageView.scale);
}

void TestBasicImageView::test_zoomOutButton()
{
    BasicImageView imageView;

    // 画像が設定されていない時は拡大率が変わらないこと
    QTest::mouseClick(imageView.ui->zoomOutButton, Qt::MouseButton::LeftButton);
    QCOMPARE_EQ(imageView.scale, 1.0);

    imageView.setPixmap(pixmap320);

    // 画像を縮小できること
    QTest::mouseClick(imageView.ui->zoomOutButton, Qt::MouseButton::LeftButton);
    QVERIFY(imageView.scale < 1.0);
}
} // namespace Test

QTEST_MAIN(Test::TestBasicImageView)

#include "test_basic_image_view.moc"

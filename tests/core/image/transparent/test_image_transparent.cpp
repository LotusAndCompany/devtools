#include "tests/test_util.h"

#include <QtTest>

#define _TEST_ImageTransparent
#include "core/image/transparent/image_transparent.h"
#undef _TEST_ImageTransparent

namespace Test {
class TestImageTransparent : public QObject
{
    Q_OBJECT

    const QString testDirName = "test_image_transparent_resources";
    const QString testDirPath = TEST_BIN_DIR + "/" + testDirName + "/";
    const QStringList resourceNames = {
        "320px-Qt_logo_2016.png", // 0
        "578px-Qt_logo_2016.png", // 1
    };
    QSize size320, size578;

    RandomData rd;

    static QImage sampleImage(const QColor & /*baseColor*/, const QColor & /*attentionColor*/);

private slots:
    void initTestCase(); // will be called before the first test function is executed.
    void init();         // will be called before each test function is executed.
    void cleanup();      // will be called after every test function.

    // Test cases:
    static void test_constructor();
    void test_load();
    void test_save();
    void test_overwriteSave();
    void test_reset();
    void test_update();
    static void test_validateImageFormat();
    static void test_addTransparentColor();
    static void test_addTransparentPixel();
    static void test_colorComparisonFunction();
    static void test_maxColorDiffSquared();
    void test_colorDiffSquaredRgb();
    void test_colorDiffSquaredHsl();
    void test_colorDiffSquaredHsv();
};

QImage TestImageTransparent::sampleImage(const QColor &baseColor, const QColor &attentionColor)
{
    const auto size = QSize(6, 6);

    QImage image(size, QImage::Format_RGBA8888);
    image.fill(baseColor);

    for (unsigned int y = 1; y <= 2; y++) {
        for (unsigned int x = 1; x <= 2; x++) {
            image.setPixelColor(x, y, attentionColor);
        }
    }
    for (unsigned int y = 3; y <= 5; y++) {
        for (unsigned int x = 3; x <= 5; x++) {
            image.setPixelColor(x, y, attentionColor);
        }
    }

    return image;
}

void TestImageTransparent::initTestCase()
{
    size320 = QImage(TEST_SRC_DIR + "/core/image/" + resourceNames[0]).size();
    size578 = QImage(TEST_SRC_DIR + "/core/image/" + resourceNames[1]).size();
}

void TestImageTransparent::init()
{
    QDir const dir(TEST_BIN_DIR);
    dir.mkpath(testDirName);

    for (const QString &src : resourceNames) {
        QFile::copy(TEST_SRC_DIR + "/core/image/" + src, testDirPath + src);
    }
}

void TestImageTransparent::cleanup()
{
    QDir testDir(testDirPath);
    testDir.removeRecursively();
}

void TestImageTransparent::test_constructor()
{
    ImageTransparent const imageTransparent;

    // stringIDが想定通り設定されていること
    QCOMPARE_EQ(imageTransparent.stringID, "image-transparent");

    // 色空間の初期値がQColor::Spec::Rgbであること
    QCOMPARE_EQ(imageTransparent.colorSpec, QColor::Spec::Rgb);

    // 許容誤差の初期値が0.1であること
    QCOMPARE_EQ(imageTransparent.tolerance, 0.1);

    // 不透明度の初期値が0であること
    QCOMPARE_EQ(imageTransparent.opacity, 0);

    // 画像が空であること
    QVERIFY(imageTransparent.current().isNull());
}

void TestImageTransparent::test_load()
{
    ImageTransparent imageTransparent;

    // 読み込みが成功すること
    QVERIFY(imageTransparent.ImageTransparentInterface::load(testDirPath + resourceNames[0]));
    QCOMPARE_EQ(imageTransparent.original().size(), size320);
    QCOMPARE_EQ(imageTransparent.current().size(), size320);

    qDebug() << "QImage::Format_RGBA8888:" << QImage::Format_RGBA8888
             << "original format:" << imageTransparent.original().format();
    // QImage::Format_RGBA8888の形式で読み込まれていること
    QCOMPARE_EQ(imageTransparent.current().format(), QImage::Format_RGBA8888);

    // 別の画像を読み込むとoriginalとcurrentとが上書きされること
    QVERIFY(imageTransparent.ImageTransparentInterface::load(testDirPath + resourceNames[1]));
    QCOMPARE_EQ(imageTransparent.original().size(), size578);
    QCOMPARE_EQ(imageTransparent.current().size(), size578);
}

void TestImageTransparent::test_save()
{
    ImageTransparent imageTransparent;
    imageTransparent.ImageTransparentInterface::load(testDirPath + resourceNames[0]);
    if (imageTransparent.current().isNull()) {
        QFAIL("image is empty");
    }

    // 保存できること
    QVERIFY(imageTransparent.save(testDirPath + "save.png"));

    // 同名ファイルで保存できないこと
    QVERIFY(!imageTransparent.save(testDirPath + "save.png"));
}

void TestImageTransparent::test_overwriteSave()
{
    ImageTransparent imageTransparent;
    imageTransparent.ImageTransparentInterface::load(testDirPath + resourceNames[0]);
    if (imageTransparent.current().isNull()) {
        QFAIL("image is empty");
    }

    // 保存できること
    QVERIFY(imageTransparent.overwriteSave(testDirPath + "overwriteSave.png"));

    // 同名ファイルで保存できること
    QVERIFY(imageTransparent.overwriteSave(testDirPath + "overwriteSave.png"));
}

void TestImageTransparent::test_reset()
{
    ImageTransparent imageTransparent;

    const int randomOpacity = rd.nextInt(256);
    imageTransparent.opacity = randomOpacity;
    imageTransparent.colorSpec = QColor::Spec::Cmyk;
    const double randomTolerance = rd.nextDouble();
    imageTransparent.tolerance = randomTolerance;

    imageTransparent.reset();

    // 各値が保持されること
    QCOMPARE_EQ(imageTransparent.opacity, randomOpacity);
    QCOMPARE_EQ(imageTransparent.colorSpec, QColor::Spec::Cmyk);
    QCOMPARE_EQ(imageTransparent.tolerance, randomTolerance);
}

void TestImageTransparent::test_update()
{
    ImageTransparent imageTransparent;

    // 画像が空の場合は失敗すること
    QVERIFY(!imageTransparent.update());

    imageTransparent.ImageTransparentInterface::load(testDirPath + resourceNames[0]);
    if (imageTransparent.current().isNull()) {
        QFAIL("image is empty");
    }

    // updated()が成功すること
    QVERIFY(imageTransparent.update());
}

void TestImageTransparent::test_validateImageFormat()
{
    const QImage::Format min = QImage::Format_Invalid;
    const QImage::Format max = QImage::Format_RGBA32FPx4_Premultiplied;

    for (int format = min; format <= max; format++) {
        if (format == QImage::Format_RGBA8888) {
            // 例外が発生しないこと
            QVERIFY_THROWS_NO_EXCEPTION(
                ImageTransparent::validateImageFormat(QImage::Format_RGBA8888));
        } else {
            // 例外が発生すること
            QVERIFY_THROWS_EXCEPTION(
                InvalidArgumentException<int>,
                ImageTransparent::validateImageFormat(static_cast<QImage::Format>(format)));
        }
    }
}

void TestImageTransparent::test_addTransparentColor()
{
    ImageTransparent imageTransparent;

    imageTransparent._current = sampleImage(QColorConstants::White, QColorConstants::Black);

    int countTransparentPixels = 0;
    for (int y = 0; y < imageTransparent.current().height(); y++) {
        for (int x = 0; x < imageTransparent.current().width(); x++) {
            if (imageTransparent.current().pixelColor(x, y).alpha() == 0) {
                countTransparentPixels++;
            }
        }
    }
    QCOMPARE_EQ(countTransparentPixels, 0);

    imageTransparent.addTransparentColor(QColorConstants::Black);

    for (int y = 0; y < imageTransparent.current().height(); y++) {
        for (int x = 0; x < imageTransparent.current().width(); x++) {
            if (imageTransparent.current().pixelColor(x, y).alpha() == 0) {
                if ((1 > x || x > 2 || 1 > y || y > 2) && (3 > x || x > 5 || 3 > y || y > 5)) {
                    qWarning() << "at:" << x << y;
                    QFAIL("unexpected transparent pixel");
                }
            }
        }
    }
}

void TestImageTransparent::test_addTransparentPixel()
{
    ImageTransparent imageTransparent;

    imageTransparent._current = sampleImage(QColorConstants::White, QColorConstants::Black);

    int countTransparentPixels = 0;
    for (int y = 0; y < imageTransparent.current().height(); y++) {
        for (int x = 0; x < imageTransparent.current().width(); x++) {
            if (imageTransparent.current().pixelColor(x, y).alpha() == 0) {
                countTransparentPixels++;
            }
        }
    }
    QCOMPARE_EQ(countTransparentPixels, 0);

    imageTransparent.addTransparentPixel(QPoint(1, 1));

    for (int y = 0; y < imageTransparent.current().height(); y++) {
        for (int x = 0; x < imageTransparent.current().width(); x++) {
            if (imageTransparent.current().pixelColor(x, y).alpha() == 0) {
                if (1 > x || x > 2 || 1 > y || y > 2) {
                    qWarning() << "at:" << x << y;
                    QFAIL("unexpected transparent pixel");
                }
            }
        }
    }
}

void TestImageTransparent::test_colorComparisonFunction()
{
    const int min = QColor::Spec::Invalid;
    const int max = QColor::Spec::ExtendedRgb;

    for (int spec = min; spec <= max; spec++) {
        void const *actual = nullptr;
        void const *expected = nullptr;
        switch (spec) {
        case QColor::Spec::Rgb:
            // ImageTransparent::colorDiffSquaredRgbを返すこと
            actual = reinterpret_cast<void *>(
                ImageTransparent::colorComparisonFunction(QColor::Spec::Rgb));
            expected = reinterpret_cast<void *>(ImageTransparent::colorDiffSquaredRgb);
            break;
        case QColor::Spec::Hsl:
            // ImageTransparent::colorDiffSquaredHslを返すこと
            actual = reinterpret_cast<void *>(
                ImageTransparent::colorComparisonFunction(QColor::Spec::Hsl));
            expected = reinterpret_cast<void *>(ImageTransparent::colorDiffSquaredHsl);
            break;
        case QColor::Spec::Hsv:
            // ImageTransparent::colorDiffSquaredHsvを返すこと
            actual = reinterpret_cast<void *>(
                ImageTransparent::colorComparisonFunction(QColor::Spec::Hsv));
            expected = reinterpret_cast<void *>(ImageTransparent::colorDiffSquaredHsv);
            break;
        default:
            // 例外が発生すること
            QVERIFY_THROWS_EXCEPTION(
                InvalidArgumentException<int>,
                ImageTransparent::colorComparisonFunction(static_cast<QColor::Spec>(spec)));
            continue;
        }

        QCOMPARE_EQ(actual, expected);
    }
}

void TestImageTransparent::test_maxColorDiffSquared()
{
    const int min = QColor::Spec::Invalid;
    const int max = QColor::Spec::ExtendedRgb;

    for (int spec = min; spec <= max; spec++) {
        switch (spec) {
        case QColor::Spec::Rgb:
            // ImageTransparent::colorDiffSquaredRgbを返すこと
            QCOMPARE_EQ(ImageTransparent::maxColorDiffSquared(QColor::Spec::Rgb), 3.0);
            break;
        case QColor::Spec::Hsl:
            // ImageTransparent::colorDiffSquaredHslを返すこと
            QCOMPARE_EQ(ImageTransparent::maxColorDiffSquared(QColor::Spec::Hsl), 4.0);
            break;
        case QColor::Spec::Hsv:
            // ImageTransparent::colorDiffSquaredHsvを返すこと
            QCOMPARE_EQ(ImageTransparent::maxColorDiffSquared(QColor::Spec::Hsv), 4.0);
            break;
        default:
            // 例外が発生すること
            QVERIFY_THROWS_EXCEPTION(
                InvalidArgumentException<int>,
                ImageTransparent::maxColorDiffSquared(static_cast<QColor::Spec>(spec)));
            break;
        }
    }
}

void TestImageTransparent::test_colorDiffSquaredRgb()
{
    const QColor randomColor = QColor(rd.nextInt(256), rd.nextInt(256), rd.nextInt(256));

    // 同じ色同士の差が0であること
    QCOMPARE_EQ(ImageTransparent::colorDiffSquaredRgb(randomColor, randomColor), 0.0);

    // 白と黒との差が3.0であること
    QCOMPARE_EQ(
        ImageTransparent::colorDiffSquaredRgb(QColorConstants::White, QColorConstants::Black), 3.0);

    // 赤と黒との差が1.0であること
    QCOMPARE_EQ(ImageTransparent::colorDiffSquaredRgb(QColorConstants::Black, QColorConstants::Red),
                1.0);
    // 赤と白との差が2.0であること
    QCOMPARE_EQ(ImageTransparent::colorDiffSquaredRgb(QColorConstants::White, QColorConstants::Red),
                2.0);

    // 緑と黒との差が1.0であること
    QCOMPARE_EQ(
        ImageTransparent::colorDiffSquaredRgb(QColorConstants::Black, QColorConstants::Green), 1.0);
    // 緑と白との差が2.0であること
    QCOMPARE_EQ(
        ImageTransparent::colorDiffSquaredRgb(QColorConstants::White, QColorConstants::Green), 2.0);

    // 青と黒との差が1.0であること
    QCOMPARE_EQ(
        ImageTransparent::colorDiffSquaredRgb(QColorConstants::Black, QColorConstants::Blue), 1.0);
    // 青と白との差が2.0であること
    QCOMPARE_EQ(
        ImageTransparent::colorDiffSquaredRgb(QColorConstants::White, QColorConstants::Blue), 2.0);

    // 赤と緑との差が2.0であること
    QCOMPARE_EQ(ImageTransparent::colorDiffSquaredRgb(QColorConstants::Red, QColorConstants::Green),
                2.0);

    // 緑と青との差が2.0であること
    QCOMPARE_EQ(
        ImageTransparent::colorDiffSquaredRgb(QColorConstants::Green, QColorConstants::Blue), 2.0);

    // 青と赤との差が2.0であること
    QCOMPARE_EQ(ImageTransparent::colorDiffSquaredRgb(QColorConstants::Blue, QColorConstants::Red),
                2.0);
}

void TestImageTransparent::test_colorDiffSquaredHsl()
{
    const QColor randomColor = QColor(rd.nextInt(256), rd.nextInt(256), rd.nextInt(256));

    // 同じ色同士の差が0であること
    QCOMPARE_EQ(ImageTransparent::colorDiffSquaredHsl(randomColor, randomColor), 0.0);

    // 白と黒との差が1.0であること
    QCOMPARE_EQ(
        ImageTransparent::colorDiffSquaredHsl(QColorConstants::Black, QColorConstants::White), 1.0);

    // 赤と黒との差が1.25であること
    QVERIFY(isEqaulApprox(
        ImageTransparent::colorDiffSquaredHsl(QColorConstants::Red, QColorConstants::Black), 1.25));
    // 赤と白との差が1.25であること
    QVERIFY(isEqaulApprox(
        ImageTransparent::colorDiffSquaredHsl(QColorConstants::Red, QColorConstants::White), 1.25));

    // 緑と黒との差が1.25であること
    QVERIFY(isEqaulApprox(
        ImageTransparent::colorDiffSquaredHsl(QColorConstants::Green, QColorConstants::Black),
        1.25));
    // 緑と白との差が1.25であること
    QVERIFY(isEqaulApprox(
        ImageTransparent::colorDiffSquaredHsl(QColorConstants::Green, QColorConstants::White),
        1.25));

    // 青と黒との差が1.25であること
    QVERIFY(isEqaulApprox(
        ImageTransparent::colorDiffSquaredHsl(QColorConstants::Blue, QColorConstants::Black),
        1.25));
    // 青と白との差が1.25であること
    QVERIFY(isEqaulApprox(
        ImageTransparent::colorDiffSquaredHsl(QColorConstants::Blue, QColorConstants::White),
        1.25));

    // 赤と緑との差が3.0であること
    qDebug() << "赤 緑";
    QVERIFY(isEqaulApprox(
        3.0, ImageTransparent::colorDiffSquaredHsl(QColorConstants::Red, QColorConstants::Green)));
    // 緑と青との差が3.0であること
    qDebug() << "緑 青";
    QVERIFY(isEqaulApprox(
        3.0, ImageTransparent::colorDiffSquaredHsl(QColorConstants::Green, QColorConstants::Blue)));
    // 青と赤との差が3.0であること
    qDebug() << "青 赤";
    QVERIFY(isEqaulApprox(
        3.0, ImageTransparent::colorDiffSquaredHsl(QColorConstants::Blue, QColorConstants::Red)));
}

void TestImageTransparent::test_colorDiffSquaredHsv()
{
    const QColor randomColor = QColor(rd.nextInt(256), rd.nextInt(256), rd.nextInt(256));

    // 同じ色同士の差が0であること
    QCOMPARE_EQ(ImageTransparent::colorDiffSquaredHsv(randomColor, randomColor), 0.0);

    // 白と黒との差が1.0であること
    QCOMPARE_EQ(
        ImageTransparent::colorDiffSquaredHsv(QColorConstants::Black, QColorConstants::White), 1.0);

    // 赤と黒との差が2.0であること
    QVERIFY(isEqaulApprox(
        ImageTransparent::colorDiffSquaredHsv(QColorConstants::Red, QColorConstants::Black), 2.0));
    // 赤と白との差が1.0であること
    QVERIFY(isEqaulApprox(
        ImageTransparent::colorDiffSquaredHsv(QColorConstants::Red, QColorConstants::White), 1.0));

    // 緑と黒との差が2.0であること
    QVERIFY(isEqaulApprox(
        ImageTransparent::colorDiffSquaredHsv(QColorConstants::Green, QColorConstants::Black),
        2.0));
    // 緑と白との差が1.0であること
    QVERIFY(isEqaulApprox(
        ImageTransparent::colorDiffSquaredHsv(QColorConstants::Green, QColorConstants::White),
        1.0));

    // 青と黒との差が2.0であること
    QVERIFY(isEqaulApprox(
        ImageTransparent::colorDiffSquaredHsv(QColorConstants::Blue, QColorConstants::Black), 2.0));
    // 青と白との差が1.0であること
    QVERIFY(isEqaulApprox(
        ImageTransparent::colorDiffSquaredHsv(QColorConstants::Blue, QColorConstants::White), 1.0));

    // 赤と緑との差が3.0であること
    qDebug() << "赤 緑";
    QVERIFY(isEqaulApprox(
        3.0, ImageTransparent::colorDiffSquaredHsv(QColorConstants::Red, QColorConstants::Green)));
    // 緑と青との差が3.0であること
    qDebug() << "緑 青";
    QVERIFY(isEqaulApprox(
        3.0, ImageTransparent::colorDiffSquaredHsv(QColorConstants::Green, QColorConstants::Blue)));
    // 青と赤との差が3.0であること
    qDebug() << "青 赤";
    QVERIFY(isEqaulApprox(
        3.0, ImageTransparent::colorDiffSquaredHsv(QColorConstants::Blue, QColorConstants::Red)));
}
} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
QTEST_APPLESS_MAIN(Test::TestImageTransparent)

#include "test_image_transparent.moc"

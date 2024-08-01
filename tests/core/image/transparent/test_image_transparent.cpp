#include <QtTest>
#include "tests/test_util.h"

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

    static QImage sampleImage(const QColor &, const QColor &);

private slots:
    void initTestCase(); // will be called before the first test function is executed.
    void init();         // will be called before each test function is executed.
    void cleanup();      // will be called after every test function.

    // Test cases:
    void test_constructor();
    void test_load();
    void test_save();
    void test_overwriteSave();
    void test_reset();
    void test_update();
    void test_validateImageFormat();
    void test_addTransparentColor();
    void test_addTransparentPixel();
    void test_colorComparisonFunction();
    void test_maxColorDiffSquared();
    void test_colorDiffSquaredRgb();
    void test_colorDiffSquaredHsl();
    void test_colorDiffSquaredHsv();
};

QImage TestImageTransparent::sampleImage(const QColor &baseColor, const QColor &attentionColor)
{
    const auto size = QSize(6, 6);

    QImage image(size, QImage::Format_RGBA8888);
    image.fill(baseColor);

    for (unsigned int y = 1; y <= 2; y++)
        for (unsigned int x = 1; x <= 2; x++)
            image.setPixelColor(x, y, attentionColor);
    for (unsigned int y = 3; y <= 5; y++)
        for (unsigned int x = 3; x <= 5; x++)
            image.setPixelColor(x, y, attentionColor);

    return image;
}

void TestImageTransparent::initTestCase()
{
    size320 = QImage(TEST_SRC_DIR + "/core/image/" + resourceNames[0]).size();
    size578 = QImage(TEST_SRC_DIR + "/core/image/" + resourceNames[1]).size();
}

void TestImageTransparent::init()
{
    QDir dir(TEST_BIN_DIR);
    dir.mkpath(testDirName);

    for (const QString &src : resourceNames)
        QFile::copy(TEST_SRC_DIR + "/core/image/" + src, testDirPath + src);
}

void TestImageTransparent::cleanup()
{
    QDir testDir(testDirPath);
    testDir.removeRecursively();
}

void TestImageTransparent::test_constructor()
{
    ImageTransparent imageTransparent;

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
    if (imageTransparent.current().isNull())
        QFAIL("image is empty");

    // 保存できること
    QVERIFY(imageTransparent.save(testDirPath + "save.png"));

    // 同名ファイルで保存できないこと
    QVERIFY(!imageTransparent.save(testDirPath + "save.png"));
}

void TestImageTransparent::test_overwriteSave()
{
    ImageTransparent imageTransparent;
    imageTransparent.ImageTransparentInterface::load(testDirPath + resourceNames[0]);
    if (imageTransparent.current().isNull())
        QFAIL("image is empty");

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
    if (imageTransparent.current().isNull())
        QFAIL("image is empty");

    // updated()が成功すること
    QVERIFY(imageTransparent.update());
}

void TestImageTransparent::test_validateImageFormat()
{
    const QImage::Format min = QImage::Format_Invalid,
                         max = QImage::Format_RGBA32FPx4_Premultiplied;

    for (int format = min; format <= max; format++) {
        if (format == QImage::Format_RGBA8888)
            // 例外が発生しないこと
            QVERIFY_THROWS_NO_EXCEPTION(
                ImageTransparent::validateImageFormat(QImage::Format_RGBA8888));
        else
            // 例外が発生すること
            QVERIFY_THROWS_EXCEPTION(InvalidArgumentException<int>,
                                     ImageTransparent::validateImageFormat(
                                         static_cast<QImage::Format>(format)));
    }
}

void TestImageTransparent::test_addTransparentColor()
{
    QSKIP("TODO");

    ImageTransparent imageTransparent;

    imageTransparent._current = std::move(
        sampleImage(QColorConstants::White, QColorConstants::Black));
}

void TestImageTransparent::test_addTransparentPixel()
{
    QSKIP("TODO");

    ImageTransparent imageTransparent;

    imageTransparent._current = std::move(
        sampleImage(QColorConstants::White, QColorConstants::Black));
}

void TestImageTransparent::test_colorComparisonFunction()
{
    const int min = QColor::Spec::Invalid, max = QColor::Spec::ExtendedRgb;

    for (int spec = min; spec <= max; spec++) {
        switch (spec) {
        case QColor::Spec::Rgb:
            // ImageTransparent::colorDiffSquaredRgbを返すこと
            QCOMPARE_EQ(ImageTransparent::colorComparisonFunction(QColor::Spec::Rgb),
                        ImageTransparent::colorDiffSquaredRgb);
            break;
        case QColor::Spec::Hsl:
            // ImageTransparent::colorDiffSquaredHslを返すこと
            QCOMPARE_EQ(ImageTransparent::colorComparisonFunction(QColor::Spec::Hsl),
                        ImageTransparent::colorDiffSquaredHsl);
            break;
        case QColor::Spec::Hsv:
            // ImageTransparent::colorDiffSquaredHsvを返すこと
            QCOMPARE_EQ(ImageTransparent::colorComparisonFunction(QColor::Spec::Hsv),
                        ImageTransparent::colorDiffSquaredHsv);
            break;
        default:
            // 例外が発生すること
            QVERIFY_THROWS_EXCEPTION(InvalidArgumentException<int>,
                                     ImageTransparent::colorComparisonFunction(
                                         static_cast<QColor::Spec>(spec)));
            break;
        }
    }
}

void TestImageTransparent::test_maxColorDiffSquared()
{
    const int min = QColor::Spec::Invalid, max = QColor::Spec::ExtendedRgb;

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
            QVERIFY_THROWS_EXCEPTION(InvalidArgumentException<int>,
                                     ImageTransparent::maxColorDiffSquared(
                                         static_cast<QColor::Spec>(spec)));
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
    QCOMPARE_EQ(ImageTransparent::colorDiffSquaredRgb(QColorConstants::White,
                                                      QColorConstants::Black),
                3.0);

    // 赤と黒との差が1.0であること
    QCOMPARE_EQ(ImageTransparent::colorDiffSquaredRgb(QColorConstants::Black, QColorConstants::Red),
                1.0);
    // 赤と白との差が2.0であること
    QCOMPARE_EQ(ImageTransparent::colorDiffSquaredRgb(QColorConstants::White, QColorConstants::Red),
                2.0);

    // 緑と黒との差が1.0であること
    QCOMPARE_EQ(ImageTransparent::colorDiffSquaredRgb(QColorConstants::Black,
                                                      QColorConstants::Green),
                1.0);
    // 緑と白との差が2.0であること
    QCOMPARE_EQ(ImageTransparent::colorDiffSquaredRgb(QColorConstants::White,
                                                      QColorConstants::Green),
                2.0);

    // 青と黒との差が1.0であること
    QCOMPARE_EQ(ImageTransparent::colorDiffSquaredRgb(QColorConstants::Black, QColorConstants::Blue),
                1.0);
    // 青と白との差が2.0であること
    QCOMPARE_EQ(ImageTransparent::colorDiffSquaredRgb(QColorConstants::White, QColorConstants::Blue),
                2.0);

    // 赤と緑との差が2.0であること
    QCOMPARE_EQ(ImageTransparent::colorDiffSquaredRgb(QColorConstants::Red, QColorConstants::Green),
                2.0);

    // 緑と青との差が2.0であること
    QCOMPARE_EQ(ImageTransparent::colorDiffSquaredRgb(QColorConstants::Green, QColorConstants::Blue),
                2.0);

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
    QCOMPARE_EQ(ImageTransparent::colorDiffSquaredHsl(QColorConstants::Black,
                                                      QColorConstants::White),
                1.0);

    // 赤と黒との差が1.25であること
    QVERIFY(isEqaulApprox(ImageTransparent::colorDiffSquaredHsl(QColorConstants::Red,
                                                                QColorConstants::Black),
                          1.25));
    // 赤と白との差が1.25であること
    QVERIFY(isEqaulApprox(ImageTransparent::colorDiffSquaredHsl(QColorConstants::Red,
                                                                QColorConstants::White),
                          1.25));

    // 緑と黒との差が1.25であること
    QVERIFY(isEqaulApprox(ImageTransparent::colorDiffSquaredHsl(QColorConstants::Green,
                                                                QColorConstants::Black),
                          1.25));
    // 緑と白との差が1.25であること
    QVERIFY(isEqaulApprox(ImageTransparent::colorDiffSquaredHsl(QColorConstants::Green,
                                                                QColorConstants::White),
                          1.25));

    // 青と黒との差が1.25であること
    QVERIFY(isEqaulApprox(ImageTransparent::colorDiffSquaredHsl(QColorConstants::Blue,
                                                                QColorConstants::Black),
                          1.25));
    // 青と白との差が1.25であること
    QVERIFY(isEqaulApprox(ImageTransparent::colorDiffSquaredHsl(QColorConstants::Blue,
                                                                QColorConstants::White),
                          1.25));

    const double diffExpected = sqrt(3.0) + 2.0;
    // 赤と緑との差が√3+2であること
    qDebug() << "赤 緑";
    QVERIFY(isEqaulApprox(diffExpected,
                          ImageTransparent::colorDiffSquaredHsl(QColorConstants::Red,
                                                                QColorConstants::Green)));
    // 緑と青との差が√3+2であること
    qDebug() << "緑 青";
    QVERIFY(isEqaulApprox(diffExpected,
                          ImageTransparent::colorDiffSquaredHsl(QColorConstants::Green,
                                                                QColorConstants::Blue)));
    // 青と赤との差が√3+2であること
    qDebug() << "青 赤";
    QVERIFY(isEqaulApprox(diffExpected,
                          ImageTransparent::colorDiffSquaredHsl(QColorConstants::Blue,
                                                                QColorConstants::Red)));
}

void TestImageTransparent::test_colorDiffSquaredHsv()
{
    const QColor randomColor = QColor(rd.nextInt(256), rd.nextInt(256), rd.nextInt(256));

    // 同じ色同士の差が0であること
    QCOMPARE_EQ(ImageTransparent::colorDiffSquaredHsv(randomColor, randomColor), 0.0);

    // 白と黒との差が1.0であること
    QCOMPARE_EQ(ImageTransparent::colorDiffSquaredHsv(QColorConstants::Black,
                                                      QColorConstants::White),
                1.0);

    // 赤と黒との差が2.0であること
    QVERIFY(isEqaulApprox(ImageTransparent::colorDiffSquaredHsv(QColorConstants::Red,
                                                                QColorConstants::Black),
                          2.0));
    // 赤と白との差が1.25であること
    QVERIFY(isEqaulApprox(ImageTransparent::colorDiffSquaredHsv(QColorConstants::Red,
                                                                QColorConstants::White),
                          1.0));

    // 緑と黒との差が2.0であること
    QVERIFY(isEqaulApprox(ImageTransparent::colorDiffSquaredHsv(QColorConstants::Green,
                                                                QColorConstants::Black),
                          2.0));
    // 緑と白との差が1.0であること
    QVERIFY(isEqaulApprox(ImageTransparent::colorDiffSquaredHsv(QColorConstants::Green,
                                                                QColorConstants::White),
                          1.0));

    // 青と黒との差が2.0であること
    QVERIFY(isEqaulApprox(ImageTransparent::colorDiffSquaredHsv(QColorConstants::Blue,
                                                                QColorConstants::Black),
                          2.0));
    // 青と白との差が1.0であること
    QVERIFY(isEqaulApprox(ImageTransparent::colorDiffSquaredHsv(QColorConstants::Blue,
                                                                QColorConstants::White),
                          1.0));

    const double diffExpected = sqrt(3.0) + 2.0;
    // 赤と緑との差が√3+2であること
    qDebug() << "赤 緑";
    QVERIFY(isEqaulApprox(diffExpected,
                          ImageTransparent::colorDiffSquaredHsv(QColorConstants::Red,
                                                                QColorConstants::Green)));
    // 緑と青との差が√3+2であること
    qDebug() << "緑 青";
    QVERIFY(isEqaulApprox(diffExpected,
                          ImageTransparent::colorDiffSquaredHsv(QColorConstants::Green,
                                                                QColorConstants::Blue)));
    // 青と赤との差が√3+2であること
    qDebug() << "青 赤";
    QVERIFY(isEqaulApprox(diffExpected,
                          ImageTransparent::colorDiffSquaredHsv(QColorConstants::Blue,
                                                                QColorConstants::Red)));
}
}

// QCoreApplicationもQApplicationも不要な場合
QTEST_APPLESS_MAIN(Test::TestImageTransparent)

#include "test_image_transparent.moc"

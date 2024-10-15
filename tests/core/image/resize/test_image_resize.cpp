#include <QtTest>
#include "tests/test_util.h"

#define _TEST_ImageResize
#include "core/image/resize/image_resize.h"
#undef _TEST_ImageResize

#include "core/exception/invalid_argument_exception.h"
#include "core/exception/invalid_state_exception.h"

namespace Test {
class TestImageResize : public QObject
{
    Q_OBJECT

    const QString testDirName = "test_image_resize_resources";
    const QString testDirPath = TEST_BIN_DIR + "/" + testDirName + "/";
    const QStringList resourceNames = {
        "320px-Qt_logo_2016.png", // 0
        "578px-Qt_logo_2016.png", // 1
    };
    QSize size320, size578;

    RandomData rd;

private slots:
    void initTestCase(); // will be called before the first test function is executed.
    void init();    // will be called before each test function is executed.
    void cleanup(); // will be called after every test function.

    // Test cases:
    void test_constructor();
    void test_load();
    void test_save();
    void test_overwriteSave();
    void test_reset();
    void test_update();
    void test_setScale();
    void test_setScaleX();
    void test_setScaleY();
    void test_setSize();
    void test_setWidth();
    void test_setHeight();
    void test_computedSize();
    void test_computedScale();
    void test_setSmoothTransformationEnabled();
    void test_aspectRatio();
};

void TestImageResize::initTestCase()
{
    size320 = QImage(TEST_SRC_DIR + "/core/image/" + resourceNames[0]).size();
    size578 = QImage(TEST_SRC_DIR + "/core/image/" + resourceNames[1]).size();
}

void TestImageResize::init()
{
    QDir dir(TEST_BIN_DIR);
    dir.mkpath(testDirName);

    for (const QString &src : resourceNames)
        QFile::copy(TEST_SRC_DIR + "/core/image/" + src, testDirPath + src);
}

void TestImageResize::cleanup()
{
    QDir testDir(testDirPath);
    testDir.removeRecursively();
}

void TestImageResize::test_constructor()
{
    ImageResize imageResize;

    // stringIDが想定通り設定されていること
    QCOMPARE_EQ(imageResize.stringID, "image-resize");

    // 初期値がfalseであること
    QCOMPARE_EQ(imageResize.isSmoothTransformationEnabled(), false);

    // 画像が空であること
    QVERIFY(imageResize.current().isNull());

    // 初期値がtype == DEFEAULTであること
    QCOMPARE_EQ(imageResize.width.type, ImageResize::ResizeHints::Type::DEFAULT);
    QCOMPARE_EQ(imageResize.height.type, ImageResize::ResizeHints::Type::DEFAULT);
}

void TestImageResize::test_load()
{
    ImageResize imageResize;

    // 読み込みが成功すること
    QVERIFY(imageResize.ImageResizeInterface::load(testDirPath + resourceNames[0]));
    QCOMPARE_EQ(imageResize.original().size(), size320);
    QCOMPARE_EQ(imageResize.current().size(), size320);

    // 別の画像を読み込むとoriginalとcurrentとが上書きされること
    QVERIFY(imageResize.ImageResizeInterface::load(testDirPath + resourceNames[1]));
    QCOMPARE_EQ(imageResize.original().size(), size578);
    QCOMPARE_EQ(imageResize.current().size(), size578);
}

void TestImageResize::test_save()
{
    ImageResize imageResize;

    imageResize.ImageResizeInterface::load(testDirPath + resourceNames[0]);

    // 保存できること
    QVERIFY(imageResize.save(testDirPath + "save.png"));

    // 同名ファイルで保存できないこと
    QVERIFY(!imageResize.save(testDirPath + "save.png"));

    // currentが保存されていること
    imageResize.setScale(2.0);
    imageResize.update();
    const QSize size640 = imageResize.current().size();
    if (size640.isEmpty())
        QFAIL("image is empty");
    imageResize.save(testDirPath + "save2.png");
    QCOMPARE_EQ(QImage(testDirPath + "save2.png").size(), size640);
}

void TestImageResize::test_overwriteSave()
{
    ImageResize imageResize;

    imageResize.ImageResizeInterface::load(testDirPath + resourceNames[0]);

    // 保存できること
    QVERIFY(imageResize.overwriteSave(testDirPath + "overwriteSave.png"));

    // 同名ファイルで保存できること
    QVERIFY(imageResize.overwriteSave(testDirPath + "overwriteSave.png"));

    // currentが保存されていること
    imageResize.setScale(2.0);
    imageResize.update();
    const QSize size640 = imageResize.current().size();
    if (size640.isEmpty())
        QFAIL("image is empty");
    imageResize.overwriteSave(testDirPath + "overwriteSave2.png");
    QCOMPARE_EQ(QImage(testDirPath + "overwriteSave2.png").size(), size640);
}

void TestImageResize::test_reset()
{
    ImageResize imageResize;

    // 画像を設定していなくても呼び出せること
    imageResize.reset();

    imageResize.ImageResizeInterface::load(testDirPath + resourceNames[0]);
    if (imageResize.current().isNull())
        QFAIL("image is empty");

    imageResize.setSize(QSize(32, 32));

    imageResize.reset();

    // typeがDEFAULTに戻ること
    QCOMPARE_EQ(imageResize.width.type, ImageResize::ResizeHints::Type::DEFAULT);
    QCOMPARE_EQ(imageResize.height.type, ImageResize::ResizeHints::Type::DEFAULT);
}

void TestImageResize::test_update()
{
    ImageResize imageResize;

    // 画像が空の場合は失敗すること
    QVERIFY(!imageResize.update());

    imageResize.ImageResizeInterface::load(testDirPath + resourceNames[0]);
    if (imageResize.current().isNull())
        QFAIL("image is empty");

    imageResize.setScale(2.0);

    // currentが更新されること
    QVERIFY(imageResize.update());
    QCOMPARE_EQ(imageResize.current().size(), size320 * 2);
}

void TestImageResize::test_setScale()
{
    {
        ImageResize imageResize;
        imageResize.ImageResizeInterface::load(testDirPath + resourceNames[0]);
        if (imageResize.current().isNull())
            QFAIL("image is empty");

        const double randomScaleX = rd.nextDouble(0.1, 10);
        const double randomScaleY = rd.nextDouble(0.1, 10);

        // 縦横に拡大率が設定されていること
        imageResize.setScale(randomScaleX, randomScaleY);
        QCOMPARE_EQ(imageResize.width.type, ImageResize::ResizeHints::Type::SCALE);
        QCOMPARE_EQ(imageResize.height.type, ImageResize::ResizeHints::Type::SCALE);
        QCOMPARE_EQ(imageResize.width.scale, randomScaleX);
        QCOMPARE_EQ(imageResize.height.scale, randomScaleY);

        // outdatedがtrueに設定されること
        QCOMPARE_EQ(imageResize.isOutdated(), true);

        // 画像のサイズが想定通りであること
        imageResize.update();
        QCOMPARE_EQ(imageResize.current().size(), imageResize.computedSize());
    }

    {
        ImageResize imageResize;
        imageResize.ImageResizeInterface::load(testDirPath + resourceNames[0]);
        if (imageResize.current().isNull())
            QFAIL("image is empty");

        const double randomScale = rd.nextDouble(0.1, 10);

        // 縦横に同じ拡大率が設定されていること
        imageResize.setScale(randomScale);
        QCOMPARE_EQ(imageResize.width.scale, randomScale);
        QCOMPARE_EQ(imageResize.height.scale, randomScale);
    }
}

void TestImageResize::test_setScaleX()
{
    {
        ImageResize imageResize;

        // 0未満の拡大率を渡すと例外が発生すること
        QVERIFY_THROWS_EXCEPTION(InvalidArgumentException<double>, imageResize.setScaleX(-1));
    }

    {
        ImageResize imageResize;
        imageResize.ImageResizeInterface::load(testDirPath + resourceNames[0]);
        if (imageResize.current().isNull())
            QFAIL("image is empty");

        const double randomScale = rd.nextDouble(0.1, 10);

        // 横にのみ拡大率が設定されていること
        imageResize.setScaleX(randomScale);
        QCOMPARE_EQ(imageResize.width.type, ImageResize::ResizeHints::Type::SCALE);
        QCOMPARE_EQ(imageResize.width.scale, randomScale);
        QCOMPARE_EQ(imageResize.height.type, ImageResize::ResizeHints::Type::DEFAULT);

        // outdatedがtrueに設定されること
        QCOMPARE_EQ(imageResize.isOutdated(), true);

        // 画像のサイズが想定通りであること
        imageResize.update();
        QCOMPARE_EQ(imageResize.current().size(), imageResize.computedSize());
    }
}

void TestImageResize::test_setScaleY()
{
    {
        ImageResize imageResize;

        // 0未満の拡大率を渡すと例外が発生すること
        QVERIFY_THROWS_EXCEPTION(InvalidArgumentException<double>, imageResize.setScaleY(-1));
    }

    {
        ImageResize imageResize;
        imageResize.ImageResizeInterface::load(testDirPath + resourceNames[0]);
        if (imageResize.current().isNull())
            QFAIL("image is empty");

        const double randomScale = rd.nextDouble(0.1, 10);

        // 縦にのみ拡大率が設定されていること
        imageResize.setScaleY(randomScale);
        QCOMPARE_EQ(imageResize.width.type, ImageResize::ResizeHints::Type::DEFAULT);
        QCOMPARE_EQ(imageResize.height.type, ImageResize::ResizeHints::Type::SCALE);
        QCOMPARE_EQ(imageResize.height.scale, randomScale);

        // outdatedがtrueに設定されること
        QCOMPARE_EQ(imageResize.isOutdated(), true);

        // 画像のサイズが想定通りであること
        imageResize.update();
        QCOMPARE_EQ(imageResize.current().size(), imageResize.computedSize());
    }
}

void TestImageResize::test_setSize()
{
    {
        ImageResize imageResize;

        // 無効な画像サイズを渡すと例外が発生すること
        QVERIFY_THROWS_EXCEPTION(InvalidArgumentException<QSize>,
                                 imageResize.setSize(QSize(-1, -1)));
    }

    {
        ImageResize imageResize;
        imageResize.ImageResizeInterface::load(testDirPath + resourceNames[0]);
        if (imageResize.current().isNull())
            QFAIL("image is empty");

        const int randomWidth = rd.nextInt(32, 3200);
        const int randomHeight = rd.nextInt(32, 3200);

        // 縦横にサイズが設定されること
        imageResize.setSize(QSize(randomWidth, randomHeight));
        QCOMPARE_EQ(imageResize.width.type, ImageResize::ResizeHints::Type::SIZE);
        QCOMPARE_EQ(imageResize.width.size, randomWidth);
        QCOMPARE_EQ(imageResize.height.type, ImageResize::ResizeHints::Type::SIZE);
        QCOMPARE_EQ(imageResize.height.size, randomHeight);

        // outdatedがtrueに設定されること
        QCOMPARE_EQ(imageResize.isOutdated(), true);

        // 画像のサイズが想定通りであること
        imageResize.update();
        QCOMPARE_EQ(imageResize.current().size(), imageResize.computedSize());
    }
}

void TestImageResize::test_setWidth()
{
    {
        ImageResize imageResize;
        imageResize.ImageResizeInterface::load(testDirPath + resourceNames[0]);
        if (imageResize.current().isNull())
            QFAIL("image is empty");

        const int randomWidth = rd.nextInt(32, 3200);

        // 横にのみサイズが設定されること
        imageResize.setWidth(randomWidth);
        QCOMPARE_EQ(imageResize.width.type, ImageResize::ResizeHints::Type::SIZE);
        QCOMPARE_EQ(imageResize.width.size, randomWidth);
        QCOMPARE_EQ(imageResize.height.type, ImageResize::ResizeHints::Type::DEFAULT);

        // outdatedがtrueに設定されること
        QCOMPARE_EQ(imageResize.isOutdated(), true);

        // 画像のサイズが想定通りであること
        imageResize.update();
        QCOMPARE_EQ(imageResize.current().size(), imageResize.computedSize());
    }

    {
        ImageResize imageResize;
        imageResize.ImageResizeInterface::load(testDirPath + resourceNames[0]);
        if (imageResize.current().isNull())
            QFAIL("image is empty");

        const int randomWidth = rd.nextInt(32, 3200);

        // 縦横比固定フラグがtrueの場合、縦横両方にサイズが設定されること
        imageResize.setWidth(randomWidth, true);
        QCOMPARE_EQ(imageResize.width.type, ImageResize::ResizeHints::Type::SIZE);
        QCOMPARE_EQ(imageResize.width.size, randomWidth);
        QCOMPARE_EQ(imageResize.height.type, ImageResize::ResizeHints::Type::SIZE);

        // 縦横比が凡そ保存されていること
        QVERIFY(isEqaulApprox((double) imageResize.width.size / (double) imageResize.height.size,
                              imageResize.aspectRatio(),
                              1.0 / 16.0));

        // 画像のサイズが想定通りであること
        imageResize.update();
        QCOMPARE_EQ(imageResize.current().size(), imageResize.computedSize());
    }
}

void TestImageResize::test_setHeight()
{
    {
        ImageResize imageResize;
        imageResize.ImageResizeInterface::load(testDirPath + resourceNames[0]);
        if (imageResize.current().isNull())
            QFAIL("image is empty");

        const int randomHeight = rd.nextInt(32, 3200);

        // 縦にのみサイズが設定されること
        imageResize.setHeight(randomHeight);
        QCOMPARE_EQ(imageResize.width.type, ImageResize::ResizeHints::Type::DEFAULT);
        QCOMPARE_EQ(imageResize.height.type, ImageResize::ResizeHints::Type::SIZE);
        QCOMPARE_EQ(imageResize.height.size, randomHeight);

        // outdatedがtrueに設定されること
        QCOMPARE_EQ(imageResize.isOutdated(), true);

        // 画像のサイズが想定通りであること
        imageResize.update();
        QCOMPARE_EQ(imageResize.current().size(), imageResize.computedSize());
    }

    {
        ImageResize imageResize;
        imageResize.ImageResizeInterface::load(testDirPath + resourceNames[0]);
        if (imageResize.current().isNull())
            QFAIL("image is empty");

        const int randomHeight = rd.nextInt(32, 3200);

        // 縦横比固定フラグがtrueの場合、縦横両方にサイズが設定されること
        imageResize.setHeight(randomHeight, true);
        QCOMPARE_EQ(imageResize.width.type, ImageResize::ResizeHints::Type::SIZE);
        QCOMPARE_EQ(imageResize.height.type, ImageResize::ResizeHints::Type::SIZE);
        QCOMPARE_EQ(imageResize.height.size, randomHeight);

        // 縦横比が凡そ保存されていること
        QVERIFY(isEqaulApprox((double) imageResize.width.size / (double) imageResize.height.size,
                              imageResize.aspectRatio(),
                              1.0 / 16.0));

        // 画像のサイズが想定通りであること
        imageResize.update();
        QCOMPARE_EQ(imageResize.current().size(), imageResize.computedSize());
    }
}

void TestImageResize::test_computedSize()
{
    ImageResize imageResize;

    // 画像が空の場合はQSize(0, 0)が返ってくること
    QVERIFY(imageResize.computedSize() == QSize(0, 0));

    imageResize.ImageResizeInterface::load(testDirPath + resourceNames[0]);
    if (imageResize.current().isNull())
        QFAIL("image is empty");

    // DEFAULTの場合はそのままの値を返すこと
    QCOMPARE_EQ(imageResize.computedSize(), size320);

    // 設定した幅が反映されること
    const int randomWidth = rd.nextInt(32, 3200);
    imageResize.setWidth(randomWidth);
    QCOMPARE_EQ(imageResize.computedSize(), QSize(randomWidth, size320.height()));

    // 設定した高さが反映されること
    const int randomHeight = rd.nextInt(32, 3200);
    imageResize.setHeight(randomHeight);
    QCOMPARE_EQ(imageResize.computedSize(), QSize(randomWidth, randomHeight));

    // 設定した横の拡大率が反映されること
    const double randomScaleX = rd.nextDouble(0.1, 10);
    imageResize.setScaleX(randomScaleX);
    QCOMPARE_EQ(imageResize.computedSize(), QSize(size320.width() * randomScaleX, randomHeight));

    // 設定した縦の拡大率が反映されること
    const double randomScaleY = rd.nextDouble(0.1, 10);
    imageResize.setScaleY(randomScaleY);
    QCOMPARE_EQ(imageResize.computedSize(),
                QSize(size320.width() * randomScaleX, size320.height() * randomScaleY));
}

void TestImageResize::test_computedScale()
{
    ImageResize imageResize;

    // 画像が空の場合はQSize(0, 0)が返ってくること
    QCOMPARE_EQ(imageResize.computedSize(), QSize(0, 0));

    imageResize.ImageResizeInterface::load(testDirPath + resourceNames[0]);
    if (imageResize.current().isNull())
        QFAIL("image is empty");

    // DEFAULTの場合は1.0を返すこと
    QCOMPARE_EQ(imageResize.computedScaleX(), 1.0);
    QCOMPARE_EQ(imageResize.computedScaleY(), 1.0);

    // 設定した幅が反映されること
    const int randomWidth = rd.nextInt(32, 3200);
    imageResize.setWidth(randomWidth);
    QVERIFY(isEqaulApprox(imageResize.computedScaleX(), (double) randomWidth / size320.width()));

    // 設定した高さが反映されること
    const int randomHeight = rd.nextInt(32, 3200);
    imageResize.setHeight(randomHeight);
    QVERIFY(isEqaulApprox(imageResize.computedScaleY(), (double) randomHeight / size320.height()));

    // 設定した横の拡大率が反映されること
    const double randomScaleX = rd.nextDouble(0.1, 10);
    imageResize.setScaleX(randomScaleX);
    QCOMPARE_EQ(imageResize.computedScaleX(), randomScaleX);

    // 設定した縦の拡大率が反映されること
    const double randomScaleY = rd.nextDouble(0.1, 10);
    imageResize.setScaleY(randomScaleY);
    QCOMPARE_EQ(imageResize.computedScaleY(), randomScaleY);
}

void TestImageResize::test_setSmoothTransformationEnabled()
{
    ImageResize imageResize;
    imageResize.ImageResizeInterface::load(testDirPath + resourceNames[0]);
    if (imageResize.current().isNull())
        QFAIL("image is empty");

    // trueを設定できること
    imageResize.setSmoothTransformationEnabled(true);
    QCOMPARE_EQ(imageResize.isSmoothTransformationEnabled(), true);
    // outdatedがtrueに設定されること
    QCOMPARE_EQ(imageResize.isOutdated(), true);

    imageResize.update(); // outdatedをfalseにする

    // falseを設定できること
    imageResize.setSmoothTransformationEnabled(false);
    QCOMPARE_EQ(imageResize.isSmoothTransformationEnabled(), false);
    // outdatedがtrueに設定されること
    QCOMPARE_EQ(imageResize.isOutdated(), true);
}

void TestImageResize::test_aspectRatio()
{
    ImageResize imageResize;

    // 画像が空の場合は例外が発生すること
    QVERIFY_THROWS_EXCEPTION(InvalidStateException, imageResize.aspectRatio());

    imageResize.ImageResizeInterface::load(testDirPath + resourceNames[0]);
    if (imageResize.current().isNull())
        QFAIL("image is empty");

    // 横/縦の値を返すこと
    QVERIFY(isEqaulApprox(imageResize.aspectRatio(), (double) size320.width() / size320.height()));
}
} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
QTEST_APPLESS_MAIN(Test::TestImageResize)

#include "test_image_resize.moc"

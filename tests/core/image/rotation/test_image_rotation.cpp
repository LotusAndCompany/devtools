#include "tests/test_util.h"

#include <QtTest>

#define _TEST_ImageRotation
#include "core/image/rotation/image_rotation.h"
#undef _TEST_ImageRotation

namespace Test {
class TestImageRotation : public QObject
{
    Q_OBJECT

    const QString testDirName = "test_image_rotation_resources";
    const QString testDirPath = TEST_BIN_DIR + "/" + testDirName + "/";
    const QStringList resourceNames = {
        "320px-Qt_logo_2016.png", // 0
        "578px-Qt_logo_2016.png", // 1
    };
    QSize size320, size578;

    RandomData rd;

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
    void test_rotateDegrees();
    void test_flipHorizontal();
    void test_flipVertical();
    void test_setSmoothTransformationEnabled();
};

void TestImageRotation::initTestCase()
{
    size320 = QImage(TEST_SRC_DIR + "/core/image/" + resourceNames[0]).size();
    size578 = QImage(TEST_SRC_DIR + "/core/image/" + resourceNames[1]).size();
}

void TestImageRotation::init()
{
    QDir dir(TEST_BIN_DIR);
    dir.mkpath(testDirName);

    for (const QString &src : resourceNames)
        QFile::copy(TEST_SRC_DIR + "/core/image/" + src, testDirPath + src);
}

void TestImageRotation::cleanup()
{
    QDir testDir(testDirPath);
    testDir.removeRecursively();
}

void TestImageRotation::test_constructor()
{
    ImageRotation imageRotation;

    // stringIDが想定通り設定されていること
    QCOMPARE_EQ(imageRotation.stringID, "image-rotation");

    // 初期値がfalseであること
    QCOMPARE_EQ(imageRotation.isSmoothTransformationEnabled(), false);

    // 画像が空であること
    QVERIFY(imageRotation.current().isNull());

    // 初期値が単位行列であること
    QVERIFY(imageRotation.transform.isIdentity());
}

void TestImageRotation::test_load()
{
    ImageRotation imageRotation;

    // 読み込みが成功すること
    QVERIFY(imageRotation.ImageRotationInterface::load(testDirPath + resourceNames[0]));
    QCOMPARE_EQ(imageRotation.original().size(), size320);
    QCOMPARE_EQ(imageRotation.current().size(), size320);

    // 別の画像を読み込むとoriginalとcurrentとが上書きされること
    QVERIFY(imageRotation.ImageRotationInterface::load(testDirPath + resourceNames[1]));
    QCOMPARE_EQ(imageRotation.original().size(), size578);
    QCOMPARE_EQ(imageRotation.current().size(), size578);
}

void TestImageRotation::test_save()
{
    ImageRotation imageRotation;
    imageRotation.ImageRotationInterface::load(testDirPath + resourceNames[0]);
    if (imageRotation.current().isNull())
        QFAIL("image is empty");

    // 保存できること
    QVERIFY(imageRotation.save(testDirPath + "save.png"));

    // 同名ファイルで保存できないこと
    QVERIFY(!imageRotation.save(testDirPath + "save.png"));
}

void TestImageRotation::test_overwriteSave()
{
    ImageRotation imageRotation;
    imageRotation.ImageRotationInterface::load(testDirPath + resourceNames[0]);
    if (imageRotation.current().isNull())
        QFAIL("image is empty");

    // 保存できること
    QVERIFY(imageRotation.overwriteSave(testDirPath + "overwriteSave.png"));

    // 同名ファイルで保存できること
    QVERIFY(imageRotation.overwriteSave(testDirPath + "overwriteSave.png"));
}

void TestImageRotation::test_reset()
{
    ImageRotation imageRotation;

    imageRotation.rotateDegrees(90);

    imageRotation.reset();

    // 単位行列に戻ること
    QVERIFY(imageRotation.transform.isIdentity());
}

void TestImageRotation::test_update()
{
    ImageRotation imageRotation;

    // 画像が空の場合は失敗すること
    QVERIFY(!imageRotation.update());

    imageRotation.ImageRotationInterface::load(testDirPath + resourceNames[0]);
    if (imageRotation.current().isNull())
        QFAIL("image is empty");

    imageRotation.rotateDegrees(90);

    // current()が成功すること
    QVERIFY(imageRotation.update());
    QCOMPARE_EQ(imageRotation.current().size(), QSize(size320.height(), size320.width()));
}

void TestImageRotation::test_setSmoothTransformationEnabled()
{
    ImageRotation imageRotation;
    imageRotation.ImageRotationInterface::load(testDirPath + resourceNames[0]);
    if (imageRotation.current().isNull())
        QFAIL("image is empty");

    // trueを設定できること
    imageRotation.setSmoothTransformationEnabled(true);
    QCOMPARE_EQ(imageRotation.isSmoothTransformationEnabled(), true);
    // outdatedがtrueに設定されること
    QCOMPARE_EQ(imageRotation.isOutdated(), true);

    imageRotation.update(); // outdatedをfalseにする

    // falseを設定できること
    imageRotation.setSmoothTransformationEnabled(false);
    QCOMPARE_EQ(imageRotation.isSmoothTransformationEnabled(), false);
    // outdatedがtrueに設定されること
    QCOMPARE_EQ(imageRotation.isOutdated(), true);
}

void TestImageRotation::test_rotateDegrees()
{
    ImageRotation imageRotation;
    imageRotation.ImageRotationInterface::load(testDirPath + resourceNames[0]);
    if (imageRotation.current().isNull())
        QFAIL("image is empty");

    // 反時計回りに90°回転できること
    imageRotation.rotateDegrees(90);
    QVERIFY(imageRotation.update());
    QCOMPARE_EQ(imageRotation.current().size(), QSize(size320.height(), size320.width()));

    imageRotation.reset();

    // 時計回りに90°回転できること
    imageRotation.rotateDegrees(-90);
    QVERIFY(imageRotation.update());
    QCOMPARE_EQ(imageRotation.current().size(), QSize(size320.height(), size320.width()));

    imageRotation.reset();

    // 180°回転できること
    imageRotation.rotateDegrees(180);
    QVERIFY(imageRotation.update());
    QCOMPARE_EQ(imageRotation.current().size(), size320);

    // もう一度180°回転すると元に戻ること
    imageRotation.rotateDegrees(180);
    QVERIFY(imageRotation.update());
    QVERIFY(imageRotation.transform.isIdentity());
}

void TestImageRotation::test_flipHorizontal()
{
    ImageRotation imageRotation;
    imageRotation.ImageRotationInterface::load(testDirPath + resourceNames[0]);
    if (imageRotation.current().isNull())
        QFAIL("image is empty");

    // 左右反転できること
    imageRotation.flipHorizontal();
    QVERIFY(imageRotation.update());
    QCOMPARE_EQ(imageRotation.current().size(), size320);

    // もう一度反転すると元に戻ること
    imageRotation.flipHorizontal();
    QVERIFY(imageRotation.update());
    QVERIFY(imageRotation.transform.isIdentity());
}

void TestImageRotation::test_flipVertical()
{
    ImageRotation imageRotation;
    imageRotation.ImageRotationInterface::load(testDirPath + resourceNames[0]);
    if (imageRotation.current().isNull())
        QFAIL("image is empty");

    // 上下反転できること
    imageRotation.flipVertical();
    QVERIFY(imageRotation.update());
    QCOMPARE_EQ(imageRotation.current().size(), size320);

    // もう一度反転すると元に戻ること
    imageRotation.flipVertical();
    QVERIFY(imageRotation.update());
    QVERIFY(imageRotation.transform.isIdentity());
}
} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
QTEST_APPLESS_MAIN(Test::TestImageRotation)

#include "test_image_rotation.moc"

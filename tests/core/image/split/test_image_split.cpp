#include <QtTest>
#include "tests/test_util.h"

#define _TEST_ImageSplit
// necessary includes here
#include "core/image/split/image_split.h"
#undef _TEST_ImageSplit

#include <QObject>

namespace Test {
class TestImageSplit : public QObject
{
    Q_OBJECT

    const QString testDirName = "test_image_split_resources";
    const QString testDirPath = TEST_BIN_DIR + "/" + testDirName + "/";
    const QStringList resourceNames = {
        "320px-Qt_logo_2016.png", // 0
        "578px-Qt_logo_2016.png", // 1
    };
    QSize size320, size578;

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
};

void TestImageSplit::initTestCase()
{
    size320 = QImage(TEST_SRC_DIR + "/core/image/" + resourceNames[0]).size();
    size578 = QImage(TEST_SRC_DIR + "/core/image/" + resourceNames[1]).size();
}

void TestImageSplit::init()
{
    QDir dir(TEST_BIN_DIR);
    dir.mkpath(testDirName);

    for (const QString &src : resourceNames)
        QFile::copy(TEST_SRC_DIR + "/core/image/" + src, testDirPath + src);
}

void TestImageSplit::cleanup()
{
    QDir testDir(testDirPath);
    testDir.removeRecursively();
}

void TestImageSplit::test_constructor()
{
    ImageSplit imageSplit;

    // stringIDが想定通り設定されていること
    QCOMPARE_EQ(imageSplit.stringID, "image-split");

    // 初期値がtrueであること
    QCOMPARE_EQ(imageSplit.discardRemainders, true);

    // 初期値がfalseであること
    QCOMPARE_EQ(imageSplit.horizontal.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageSplit.vertical.isSpecifiedWithSize, false);

    // 初期値が1であること
    QCOMPARE_EQ(imageSplit.horizontal.value, 1);
    QCOMPARE_EQ(imageSplit.vertical.value, 1);
}

void TestImageSplit::test_load()
{
    ImageSplit imageSplit;

    // 読み込みが成功すること
    QVERIFY(imageSplit.ImageSplitInterface::load(testDirPath + resourceNames[0]));
    QCOMPARE_EQ(imageSplit.original().size(), size320);
    QCOMPARE_EQ(imageSplit.current().size(), size320);

    // 別の画像を読み込むとoriginalとcurrentとが上書きされること
    QVERIFY(imageSplit.ImageSplitInterface::load(testDirPath + resourceNames[1]));
    QCOMPARE_EQ(imageSplit.original().size(), size578);
    QCOMPARE_EQ(imageSplit.current().size(), size578);
}

void TestImageSplit::test_save()
{
    ImageSplit imageSplit;
    imageSplit.ImageSplitInterface::load(testDirPath + resourceNames[0]);
    if (imageSplit.current().isNull())
        QFAIL("image is empty");

    QFileInfo info(testDirPath + "save");
    QVERIFY(!info.exists());

    // パスが存在しない場合、失敗すること
    QVERIFY(!imageSplit.save(info.filePath()));

    QDir dir(testDirPath);
    dir.mkpath("save");
    QVERIFY(imageSplit.save(info.filePath()));

    // 保存できること
    QVERIFY(imageSplit.save(info.filePath()));

    // 同名ファイルで保存できないこと
    QVERIFY(!imageSplit.save(info.filePath()));
}

void TestImageSplit::test_overwriteSave()
{
    ImageSplit imageSplit;
    imageSplit.ImageSplitInterface::load(testDirPath + resourceNames[0]);
    if (imageSplit.current().isNull())
        QFAIL("image is empty");

    QFileInfo info(testDirPath + "overwriteSave");
    QVERIFY(!info.exists());

    // パスが存在しない場合、失敗すること
    QVERIFY(!imageSplit.save(info.filePath()));

    QDir dir(testDirPath);
    dir.mkpath("overwriteSave");
    QVERIFY(imageSplit.save(info.filePath()));

    // 保存できること
    QVERIFY(imageSplit.overwriteSave(info.filePath()));

    // 同名ファイルで保存できること
    QVERIFY(imageSplit.overwriteSave(info.filePath()));
}

void TestImageSplit::test_reset()
{
    ImageSplit imageSplit;
    imageSplit.ImageSplitInterface::load(testDirPath + resourceNames[0]);
    if (imageSplit.current().isNull())
        QFAIL("image is empty");

    imageSplit.setHorizontalSplit(2);
    imageSplit.setVerticalSplit(2);

    imageSplit.reset();

    // 1x1に戻ること
    QCOMPARE_EQ(imageSplit.horizontal.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageSplit.horizontal.value, 1);
    QCOMPARE_EQ(imageSplit.vertical.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageSplit.vertical.value, 1);

    imageSplit.setCellWidth(100);
    imageSplit.setCellHeight(100);

    imageSplit.reset();

    // 1x1に戻ること
    QCOMPARE_EQ(imageSplit.horizontal.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageSplit.horizontal.value, 1);
    QCOMPARE_EQ(imageSplit.vertical.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageSplit.vertical.value, 1);
}

void TestImageSplit::test_update()
{
    ImageSplit imageSplit;

    // 画像が空の場合は失敗すること
    QVERIFY(!imageSplit.update());

    imageSplit.ImageSplitInterface::load(testDirPath + resourceNames[0]);
    if (imageSplit.current().isNull())
        QFAIL("image is empty");

    // update()が成功すること
    QVERIFY(imageSplit.update());
}

} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
QTEST_APPLESS_MAIN(Test::TestImageSplit)

#include "test_image_split.moc"

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
    void test_setHorizontalSplit();
    void test_setVerticalSplit();
    void test_setCellWidth();
    void test_setCellHeight();
    void test_computedCellSize();
    void test_numberOfHorizontalSplit();
    void test_numberOfVerticalSplit();
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
    QVERIFY(info.exists());

    // 保存できること
    QVERIFY(imageSplit.save(info.filePath()));
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_0_0.png"));

    // 同名ファイルで保存できないこと
    QVERIFY(!imageSplit.save(info.filePath()));

    // 分割して保存
    imageSplit.reset();
    QDir(info.filePath()).remove("320px-Qt_logo_2016_0_0.png");
    imageSplit.setCellWidth(200);
    imageSplit.setCellHeight(200);
    QVERIFY(imageSplit.save(info.filePath()));

    // 左上の領域だけ保存されていること
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_0_0.png"));
    QVERIFY(!QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_0_1.png"));
    QVERIFY(!QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_1_0.png"));
    QVERIFY(!QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_1_1.png"));

    QDir(info.filePath()).remove("320px-Qt_logo_2016_0_0.png");
    imageSplit.discardRemainders = false;
    QVERIFY(imageSplit.save(info.filePath()));

    // 4つの領域が保存されていること
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_0_0.png"));
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_0_1.png"));
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_1_0.png"));
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_1_1.png"));
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
    QVERIFY(info.exists());

    // 保存できること
    QVERIFY(imageSplit.overwriteSave(info.filePath()));
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_0_0.png"));

    // 同名ファイルで保存できること
    QVERIFY(imageSplit.overwriteSave(info.filePath()));

    // 分割して保存
    imageSplit.reset();
    QDir(info.filePath()).remove("320px-Qt_logo_2016_0_0.png");
    imageSplit.setCellWidth(200);
    imageSplit.setCellHeight(200);
    QVERIFY(imageSplit.overwriteSave(info.filePath()));

    // 左上の領域だけ保存されていること
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_0_0.png"));
    QVERIFY(!QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_0_1.png"));
    QVERIFY(!QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_1_0.png"));
    QVERIFY(!QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_1_1.png"));

    QDir(info.filePath()).remove("320px-Qt_logo_2016_0_0.png");
    imageSplit.discardRemainders = false;
    QVERIFY(imageSplit.overwriteSave(info.filePath()));

    // 4つの領域が保存されていること
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_0_0.png"));
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_0_1.png"));
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_1_0.png"));
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_1_1.png"));
}

void TestImageSplit::test_reset()
{
    ImageSplit imageSplit;

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

void TestImageSplit::test_setHorizontalSplit()
{
    ImageSplit imageSplit;

    imageSplit.setHorizontalSplit(2);
    // 想定通りの値が設定されていること
    QCOMPARE_EQ(imageSplit.horizontal.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageSplit.horizontal.value, 2);

    // verticalが変更されていないこと
    QCOMPARE_EQ(imageSplit.vertical.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageSplit.vertical.value, 1);
}

void TestImageSplit::test_setVerticalSplit()
{
    ImageSplit imageSplit;

    imageSplit.setVerticalSplit(2);
    // 想定通りの値が設定されていること
    QCOMPARE_EQ(imageSplit.vertical.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageSplit.vertical.value, 2);

    // horizontalが変更されていないこと
    QCOMPARE_EQ(imageSplit.horizontal.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageSplit.horizontal.value, 1);
}

void TestImageSplit::test_setCellWidth()
{
    ImageSplit imageSplit;

    imageSplit.setCellWidth(50);
    // 想定通りの値が設定されていること
    QCOMPARE_EQ(imageSplit.horizontal.isSpecifiedWithSize, true);
    QCOMPARE_EQ(imageSplit.horizontal.value, 50);

    // verticalが変更されていないこと
    QCOMPARE_EQ(imageSplit.vertical.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageSplit.vertical.value, 1);
}

void TestImageSplit::test_setCellHeight()
{
    ImageSplit imageSplit;

    imageSplit.setCellHeight(50);
    // 想定通りの値が設定されていること
    QCOMPARE_EQ(imageSplit.vertical.isSpecifiedWithSize, true);
    QCOMPARE_EQ(imageSplit.vertical.value, 50);

    // horizontalが変更されていないこと
    QCOMPARE_EQ(imageSplit.horizontal.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageSplit.horizontal.value, 1);
}

void TestImageSplit::test_computedCellSize()
{
    ImageSplit imageSplit;

    // 画像が空の場合は(0, 0)を返すこと
    QCOMPARE_EQ(imageSplit.computedCellSize(), QSizeF(0, 0));

    imageSplit.ImageSplitInterface::load(testDirPath + resourceNames[0]);
    if (imageSplit.current().isNull())
        QFAIL("image is empty");

    // 初期状態では元の画像サイズを返すこと
    QCOMPARE_EQ(imageSplit.computedCellSize(), size320);

    // 分割数を設定
    imageSplit.setHorizontalSplit(2);
    imageSplit.setVerticalSplit(5);
    QCOMPARE_EQ(imageSplit.computedCellSize(), QSize(size320.width() / 2, size320.height() / 5));

    // サイズ指定
    imageSplit.setCellWidth(25);
    imageSplit.setCellHeight(32);
    QCOMPARE_EQ(imageSplit.computedCellSize(), QSize(25, 32));
}

void TestImageSplit::test_numberOfHorizontalSplit()
{
    ImageSplit imageSplit;

    // 画像が空の場合は0を返すこと
    QCOMPARE_EQ(imageSplit.numberOfHorizontalSplit(), 0);

    imageSplit.ImageSplitInterface::load(testDirPath + resourceNames[0]);
    if (imageSplit.current().isNull())
        QFAIL("image is empty");

    // 初期状態では1を返すこと
    QCOMPARE_EQ(imageSplit.numberOfHorizontalSplit(), 1);

    // 割り切れるをサイズ指定
    imageSplit.setCellWidth(32);
    QCOMPARE_EQ(imageSplit.numberOfHorizontalSplit(), size320.width() / 32);

    // 割り切れる場合は分割数が変わらないこと
    imageSplit.discardRemainders = false;
    QCOMPARE_EQ(imageSplit.numberOfHorizontalSplit(), size320.width() / 32);

    // 割り切れないサイズを指定
    imageSplit.discardRemainders = true;
    imageSplit.setCellWidth(30);
    QCOMPARE_EQ(imageSplit.numberOfHorizontalSplit(), size320.width() / 30);

    // 割り切れない場合は分割数が+1されること
    imageSplit.discardRemainders = false;
    QCOMPARE_EQ(imageSplit.numberOfHorizontalSplit(), 1 + size320.width() / 30);
}

void TestImageSplit::test_numberOfVerticalSplit()
{
    ImageSplit imageSplit;

    // 画像が空の場合は0を返すこと
    QCOMPARE_EQ(imageSplit.numberOfVerticalSplit(), 0);

    imageSplit.ImageSplitInterface::load(testDirPath + resourceNames[0]);
    if (imageSplit.current().isNull())
        QFAIL("image is empty");

    // 初期状態では1を返すこと
    QCOMPARE_EQ(imageSplit.numberOfVerticalSplit(), 1);

    // 割り切れるをサイズ指定
    imageSplit.setCellHeight(47);
    QCOMPARE_EQ(imageSplit.numberOfVerticalSplit(), size320.height() / 47);

    // 割り切れる場合は分割数が変わらないこと
    imageSplit.discardRemainders = false;
    QCOMPARE_EQ(imageSplit.numberOfVerticalSplit(), size320.height() / 47);

    // 割り切れないサイズを指定
    imageSplit.discardRemainders = true;
    imageSplit.setCellHeight(30);
    QCOMPARE_EQ(imageSplit.numberOfVerticalSplit(), size320.height() / 30);

    // 割り切れない場合は分割数が+1されること
    imageSplit.discardRemainders = false;
    QCOMPARE_EQ(imageSplit.numberOfVerticalSplit(), 1 + size320.height() / 30);
}

} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
QTEST_APPLESS_MAIN(Test::TestImageSplit)

#include "test_image_split.moc"

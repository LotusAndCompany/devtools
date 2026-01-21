#include "tests/test_util.h"

#include <QtTest>

#define _TEST_ImageDivision
// necessary includes here
#include "core/image/division/image_division.h"
#undef _TEST_ImageDivision

#include <QObject>

namespace Test {
class TestImageDivision : public QObject
{
    Q_OBJECT

    const QString testDirName = "test_image_division_resources";
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
    void test_setHorizontalDivision();
    void test_setVerticalDivision();
    void test_setCellWidth();
    void test_setCellHeight();
    void test_computedCellSize();
    void test_numberOfHorizontalDivision();
    void test_numberOfVerticalDivision();
    void test_saveFilename();
};

void TestImageDivision::initTestCase()
{
    size320 = QImage(TEST_SRC_DIR + "/core/image/" + resourceNames[0]).size();
    size578 = QImage(TEST_SRC_DIR + "/core/image/" + resourceNames[1]).size();
}

void TestImageDivision::init()
{
    QDir dir(TEST_BIN_DIR);
    dir.mkpath(testDirName);

    for (const QString &src : resourceNames)
        QFile::copy(TEST_SRC_DIR + "/core/image/" + src, testDirPath + src);
}

void TestImageDivision::cleanup()
{
    QDir testDir(testDirPath);
    testDir.removeRecursively();
}

void TestImageDivision::test_constructor()
{
    ImageDivision imageDivision;

    // stringIDが想定通り設定されていること
    QCOMPARE_EQ(imageDivision.stringID, "image-division");

    // 初期値がtrueであること
    QCOMPARE_EQ(imageDivision.discardRemainders, true);

    // 初期値がfalseであること
    QCOMPARE_EQ(imageDivision.horizontal.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageDivision.vertical.isSpecifiedWithSize, false);

    // 初期値が1であること
    QCOMPARE_EQ(imageDivision.horizontal.value, 1);
    QCOMPARE_EQ(imageDivision.vertical.value, 1);
}

void TestImageDivision::test_load()
{
    ImageDivision imageDivision;

    // 読み込みが成功すること
    QVERIFY(imageDivision.ImageDivisionInterface::load(testDirPath + resourceNames[0]));
    QCOMPARE_EQ(imageDivision.original().size(), size320);
    QCOMPARE_EQ(imageDivision.current().size(), size320);

    // 別の画像を読み込むとoriginalとcurrentとが上書きされること
    QVERIFY(imageDivision.ImageDivisionInterface::load(testDirPath + resourceNames[1]));
    QCOMPARE_EQ(imageDivision.original().size(), size578);
    QCOMPARE_EQ(imageDivision.current().size(), size578);
}

void TestImageDivision::test_save()
{
    ImageDivision imageDivision;

    // ファイルが設定されていない場合、失敗すること
    QVERIFY(!imageDivision.save(""));

    imageDivision.ImageDivisionInterface::load(testDirPath + resourceNames[0]);
    if (imageDivision.current().isNull())
        QFAIL("image is empty");

    QFileInfo info(testDirPath + "save");
    QVERIFY(!info.exists());

    // パスが存在しない場合、失敗すること
    QVERIFY(!imageDivision.save(info.filePath()));

    QDir dir(testDirPath);
    dir.mkpath("save");
    QVERIFY(info.exists());

    // 保存できること
    QVERIFY(imageDivision.save(info.filePath()));
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_0_0.png"));

    // 同名ファイルで保存できないこと
    QVERIFY(!imageDivision.save(info.filePath()));

    // 分割して保存
    imageDivision.reset();
    QDir(info.filePath()).remove("320px-Qt_logo_2016_0_0.png");
    imageDivision.setCellWidth(200);
    imageDivision.setCellHeight(200);
    QVERIFY(imageDivision.save(info.filePath()));

    // 左上の領域だけ保存されていること
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_0_0.png"));
    QVERIFY(!QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_0_1.png"));
    QVERIFY(!QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_1_0.png"));
    QVERIFY(!QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_1_1.png"));

    QDir(info.filePath()).remove("320px-Qt_logo_2016_0_0.png");
    imageDivision.discardRemainders = false;
    QVERIFY(imageDivision.save(info.filePath()));

    // 4つの領域が保存されていること
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_0_0.png"));
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_0_1.png"));
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_1_0.png"));
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_1_1.png"));
}

void TestImageDivision::test_overwriteSave()
{
    ImageDivision imageDivision;

    // ファイルが設定されていない場合、失敗すること
    QVERIFY(!imageDivision.overwriteSave(""));

    imageDivision.ImageDivisionInterface::load(testDirPath + resourceNames[0]);
    if (imageDivision.current().isNull())
        QFAIL("image is empty");

    QFileInfo info(testDirPath + "overwriteSave");
    QVERIFY(!info.exists());

    // パスが存在しない場合、失敗すること
    QVERIFY(!imageDivision.overwriteSave(info.filePath()));

    QDir dir(testDirPath);
    dir.mkpath("overwriteSave");
    QVERIFY(info.exists());

    // 保存できること
    QVERIFY(imageDivision.overwriteSave(info.filePath()));
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_0_0.png"));

    // 同名ファイルで保存できること
    QVERIFY(imageDivision.overwriteSave(info.filePath()));

    // 分割して保存
    imageDivision.reset();
    QDir(info.filePath()).remove("320px-Qt_logo_2016_0_0.png");
    imageDivision.setCellWidth(200);
    imageDivision.setCellHeight(200);
    QVERIFY(imageDivision.overwriteSave(info.filePath()));

    // 左上の領域だけ保存されていること
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_0_0.png"));
    QVERIFY(!QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_0_1.png"));
    QVERIFY(!QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_1_0.png"));
    QVERIFY(!QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_1_1.png"));

    QDir(info.filePath()).remove("320px-Qt_logo_2016_0_0.png");
    imageDivision.discardRemainders = false;
    QVERIFY(imageDivision.overwriteSave(info.filePath()));

    // 4つの領域が保存されていること
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_0_0.png"));
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_0_1.png"));
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_1_0.png"));
    QVERIFY(QFileInfo::exists(info.filePath() + "/320px-Qt_logo_2016_1_1.png"));
}

void TestImageDivision::test_reset()
{
    ImageDivision imageDivision;

    imageDivision.setHorizontalDivision(2);
    imageDivision.setVerticalDivision(2);

    imageDivision.reset();

    // 1x1に戻ること
    QCOMPARE_EQ(imageDivision.horizontal.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageDivision.horizontal.value, 1);
    QCOMPARE_EQ(imageDivision.vertical.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageDivision.vertical.value, 1);

    imageDivision.setCellWidth(100);
    imageDivision.setCellHeight(100);

    imageDivision.reset();

    // 1x1に戻ること
    QCOMPARE_EQ(imageDivision.horizontal.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageDivision.horizontal.value, 1);
    QCOMPARE_EQ(imageDivision.vertical.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageDivision.vertical.value, 1);
}

void TestImageDivision::test_update()
{
    ImageDivision imageDivision;

    // 画像が空の場合は失敗すること
    QVERIFY(!imageDivision.update());

    imageDivision.ImageDivisionInterface::load(testDirPath + resourceNames[0]);
    if (imageDivision.current().isNull())
        QFAIL("image is empty");

    // update()が成功すること
    QVERIFY(imageDivision.update());
}

void TestImageDivision::test_setHorizontalDivision()
{
    ImageDivision imageDivision;

    imageDivision.setHorizontalDivision(2);
    // 想定通りの値が設定されていること
    QCOMPARE_EQ(imageDivision.horizontal.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageDivision.horizontal.value, 2);

    // verticalが変更されていないこと
    QCOMPARE_EQ(imageDivision.vertical.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageDivision.vertical.value, 1);
}

void TestImageDivision::test_setVerticalDivision()
{
    ImageDivision imageDivision;

    imageDivision.setVerticalDivision(2);
    // 想定通りの値が設定されていること
    QCOMPARE_EQ(imageDivision.vertical.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageDivision.vertical.value, 2);

    // horizontalが変更されていないこと
    QCOMPARE_EQ(imageDivision.horizontal.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageDivision.horizontal.value, 1);
}

void TestImageDivision::test_setCellWidth()
{
    ImageDivision imageDivision;

    imageDivision.setCellWidth(50);
    // 想定通りの値が設定されていること
    QCOMPARE_EQ(imageDivision.horizontal.isSpecifiedWithSize, true);
    QCOMPARE_EQ(imageDivision.horizontal.value, 50);

    // verticalが変更されていないこと
    QCOMPARE_EQ(imageDivision.vertical.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageDivision.vertical.value, 1);
}

void TestImageDivision::test_setCellHeight()
{
    ImageDivision imageDivision;

    imageDivision.setCellHeight(50);
    // 想定通りの値が設定されていること
    QCOMPARE_EQ(imageDivision.vertical.isSpecifiedWithSize, true);
    QCOMPARE_EQ(imageDivision.vertical.value, 50);

    // horizontalが変更されていないこと
    QCOMPARE_EQ(imageDivision.horizontal.isSpecifiedWithSize, false);
    QCOMPARE_EQ(imageDivision.horizontal.value, 1);
}

void TestImageDivision::test_computedCellSize()
{
    ImageDivision imageDivision;

    // 画像が空の場合は(0, 0)を返すこと
    QCOMPARE_EQ(imageDivision.computedCellSize(), QSizeF(0, 0));

    imageDivision.ImageDivisionInterface::load(testDirPath + resourceNames[0]);
    if (imageDivision.current().isNull())
        QFAIL("image is empty");

    // 初期状態では元の画像サイズを返すこと
    QCOMPARE_EQ(imageDivision.computedCellSize(), size320);

    // 分割数を設定
    imageDivision.setHorizontalDivision(2);
    imageDivision.setVerticalDivision(5);
    QCOMPARE_EQ(imageDivision.computedCellSize(), QSize(size320.width() / 2, size320.height() / 5));

    // サイズ指定
    imageDivision.setCellWidth(25);
    imageDivision.setCellHeight(32);
    QCOMPARE_EQ(imageDivision.computedCellSize(), QSize(25, 32));
}

void TestImageDivision::test_numberOfHorizontalDivision()
{
    ImageDivision imageDivision;

    // 画像が空の場合は0を返すこと
    QCOMPARE_EQ(imageDivision.numberOfHorizontalDivision(), 0);

    imageDivision.ImageDivisionInterface::load(testDirPath + resourceNames[0]);
    if (imageDivision.current().isNull())
        QFAIL("image is empty");

    // 初期状態では1を返すこと
    QCOMPARE_EQ(imageDivision.numberOfHorizontalDivision(), 1);

    // 割り切れるをサイズ指定
    imageDivision.setCellWidth(32);
    QCOMPARE_EQ(imageDivision.numberOfHorizontalDivision(), size320.width() / 32);

    // 割り切れる場合は分割数が変わらないこと
    imageDivision.discardRemainders = false;
    QCOMPARE_EQ(imageDivision.numberOfHorizontalDivision(), size320.width() / 32);

    // 割り切れないサイズを指定
    imageDivision.discardRemainders = true;
    imageDivision.setCellWidth(30);
    QCOMPARE_EQ(imageDivision.numberOfHorizontalDivision(), size320.width() / 30);

    // 割り切れない場合は分割数が+1されること
    imageDivision.discardRemainders = false;
    QCOMPARE_EQ(imageDivision.numberOfHorizontalDivision(), 1 + size320.width() / 30);
}

void TestImageDivision::test_numberOfVerticalDivision()
{
    ImageDivision imageDivision;

    // 画像が空の場合は0を返すこと
    QCOMPARE_EQ(imageDivision.numberOfVerticalDivision(), 0);

    imageDivision.ImageDivisionInterface::load(testDirPath + resourceNames[0]);
    if (imageDivision.current().isNull())
        QFAIL("image is empty");

    // 初期状態では1を返すこと
    QCOMPARE_EQ(imageDivision.numberOfVerticalDivision(), 1);

    // 割り切れるをサイズ指定
    imageDivision.setCellHeight(47);
    QCOMPARE_EQ(imageDivision.numberOfVerticalDivision(), size320.height() / 47);

    // 割り切れる場合は分割数が変わらないこと
    imageDivision.discardRemainders = false;
    QCOMPARE_EQ(imageDivision.numberOfVerticalDivision(), size320.height() / 47);

    // 割り切れないサイズを指定
    imageDivision.discardRemainders = true;
    imageDivision.setCellHeight(30);
    QCOMPARE_EQ(imageDivision.numberOfVerticalDivision(), size320.height() / 30);

    // 割り切れない場合は分割数が+1されること
    imageDivision.discardRemainders = false;
    QCOMPARE_EQ(imageDivision.numberOfVerticalDivision(), 1 + size320.height() / 30);
}

void TestImageDivision::test_saveFilename()
{
    ImageDivision imageDivision;
    imageDivision.ImageDivisionInterface::load(testDirPath + resourceNames[0]);
    if (imageDivision.current().isNull())
        QFAIL("image is empty");

    RandomData rd;
    const unsigned int randomX = rd.nextInt(64);
    const unsigned int randomY = rd.nextInt(64);
    const QString randomLocation =
        rd.nextQString(48, RandomData::lowerAlphabets + RandomData::upperAlphabets +
                               RandomData::numbers + "/_-") +
        "/" +
        rd.nextQString(
            16, RandomData::lowerAlphabets + RandomData::upperAlphabets + RandomData::numbers);

    const QFileInfo result =
        QFileInfo(imageDivision.saveFilename(randomLocation, randomX, randomY));
    // qDebug() << result;

    // フォルダ名が想定通りであること
    QCOMPARE_EQ(result.dir(), randomLocation);
    // ファイル名が想定通りであること
    QCOMPARE_EQ(result.baseName(),
                (imageDivision.fileInfo().baseName() + "_%1_%2").arg(randomX).arg(randomY));
    // 拡張子が想定通りであること
    QCOMPARE_EQ(result.suffix(), imageDivision.fileInfo().suffix());
}

} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
QTEST_APPLESS_MAIN(Test::TestImageDivision)

#include "test_image_division.moc"

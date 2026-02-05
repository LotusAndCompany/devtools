#include "tests/test_util.h"

#include <QtTest>

#define _TEST_TestImageIO
#include "core/image/basic_image_io.h"
#undef _TEST_TestImageIO

#include <QFile>

namespace Test {
class TestImageIO : public QObject
{
    Q_OBJECT

    const QString testDirName = "test_basic_image_io_resources";
    const QString testDirPath = TEST_BIN_DIR + "/" + testDirName + "/";
    const QStringList resourceNames = {
        "320px-Qt_logo_2016.png",     // 0
        "320px-Qt_logo_2016.jpg",     // 1
        "Qt_logo_2016.svg",           // 2
        "320px-Qt_logo_2016_png.jpg", // 3
        "Qt_logo_2016_svg.png",       // 4
        "320px-Qt_logo_2016.svg.png", // 5
        "Qt_logo_2016",               // 6
        "578px-Qt_logo_2016.png",     // 7
    };

private slots:
    void init();    // will be called before each test function is executed.
    void cleanup(); // will be called after every test function.

    // Test cases:
    static void test_constructor();
    void test_load();
    void test_original();
    void test_originalFileInfo();
    void test_save();
    void test_overwriteSave();
};

void TestImageIO::init()
{
    QDir const dir(TEST_BIN_DIR);
    dir.mkpath(testDirName);

    for (const QString &src : resourceNames) {
        QFile::copy(TEST_SRC_DIR + "/core/image/" + src, testDirPath + src);
    }
}

void TestImageIO::cleanup()
{
    QDir testDir(testDirPath);
    testDir.removeRecursively();
}

void TestImageIO::test_constructor()
{
    ImageIO const io;

    // 画像が空であること
    QVERIFY(io.original().isNull());
    // ファイル情報が空であること
    QCOMPARE_EQ(io.originalFileInfo().filePath(), "");
}

void TestImageIO::test_load()
{
    {
        ImageIO io;
        // png読み込みが成功すること
        QVERIFY(io.load(testDirPath + resourceNames[0]));
    }

    {
        ImageIO io;
        // jpg読み込みが成功すること
        QVERIFY(io.load(testDirPath + resourceNames[1]));
    }

    {
        ImageIO io;
        // svg読み込みが成功すること
        QVERIFY(io.load(testDirPath + resourceNames[2]));
    }

    {
        ImageIO io;
        // png読み込み(拡張子jpg)が成功すること
        QVERIFY(io.load(testDirPath + resourceNames[3]));
    }

    {
        ImageIO io;
        // svg読み込み(拡張子gng)が成功すること
        QVERIFY(io.load(testDirPath + resourceNames[4]));
    }

    {
        ImageIO io;
        // svg読み込み(二重拡張子)が成功すること
        QVERIFY(io.load(testDirPath + resourceNames[5]));
    }

    {
        ImageIO io;
        // svg読み込み(拡張子無し)が成功すること
        QVERIFY(io.load(testDirPath + resourceNames[6]));
    }

    {
        ImageIO io;
        // 存在しないファイルで失敗すること
        QVERIFY(!io.load(testDirPath + "qwert_yui.op"));
    }

    {
        ImageIO io;
        io.load(testDirPath + resourceNames[0]);

        const QSize sizeBefore = io.original().size();
        const QString filePathBefore = io.originalFileInfo().filePath();

        // もう一度loadを呼び、読み込んだ画像データを上書き
        io.load(testDirPath + resourceNames[7]);

        // sizeが更新されていること
        QCOMPARE_NE(io.original().size(), sizeBefore);

        // filePathが更新されていること
        QCOMPARE_NE(io.originalFileInfo().filePath(), filePathBefore);
    }
}

void TestImageIO::test_original()
{
    ImageIO io;
    io.load(testDirPath + resourceNames[0]);

    // 読み込んだ画像のサイズが正しいこと
    QCOMPARE_EQ(io.original().size(), QSize(320, 235));
}

void TestImageIO::test_originalFileInfo()
{
    ImageIO io;
    const QString filePath = testDirPath + resourceNames[0];
    io.load(filePath);

    // 読み込んだ画像のファイルパスが正しいこと
    QCOMPARE_EQ(io.originalFileInfo().filePath(), filePath);
}

void TestImageIO::test_save()
{
    ImageIO io;
    const QString filePath = testDirPath + resourceNames[0];
    io.load(filePath);

    // 空の画像の保存が失敗すること
    QVERIFY(!ImageIO::save(testDirPath + "test_save.png", QImage()));

    // png形式の保存ができること
    QVERIFY(ImageIO::save(testDirPath + "test_save.png", io.original()));

    // jpg形式の保存ができること
    QVERIFY(ImageIO::save(testDirPath + "test_save.jpg", io.original()));

    // 同名ファイルの保存が失敗すること
    QVERIFY(!ImageIO::save(testDirPath + "test_save.png", io.original()));
}

void TestImageIO::test_overwriteSave()
{
    ImageIO io;
    const QString filePath = testDirPath + resourceNames[0];
    io.load(filePath);

    // 空の画像の保存が失敗すること
    QVERIFY(!ImageIO::overwriteSave(testDirPath + "test_overwriteSave.png", QImage()));

    // 保存が成功すること
    QVERIFY(ImageIO::overwriteSave(testDirPath + "test_overwriteSave.png", io.original()));

    // 同名ファイルの保存が成功すること
    QVERIFY(ImageIO::overwriteSave(testDirPath + "test_overwriteSave.png", io.original()));
}
} // namespace Test

QTEST_APPLESS_MAIN(Test::TestImageIO)

#include "test_basic_image_io.moc"

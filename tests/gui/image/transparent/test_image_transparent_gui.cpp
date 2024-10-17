#include <QtTest>
#include "tests/test_util.h"

#define _TEST_ImageTransparentGUI
#include "gui/image/transparent/image_transparent_gui.h"
#undef _TEST_ImageTransparentGUI

#include "core/image/transparent/image_transparent.h"

#include <QObject>

namespace Test {
// function mocks
FunctionMock<bool(const QString &, const char *, int)> mock_save, mock_overwriteSave;
FunctionMock<const QFileInfo &(unsigned int)> mock_fileInfo;
FunctionMock<const QImage &()> mock_original;
FunctionMock<bool(const QString &)> mock_loadImpl;
FunctionMock<void()> mock_resetImpl;
FunctionMock<bool()> mock_updateImpl;
FunctionMock<void(const QColor &)> mock_addTransparentColor;
FunctionMock<void(const QPoint &)> mock_addTransparentPixel;

class ImageTransparentMock : public ImageTransparentInterface
{
    Q_OBJECT

public:
    explicit ImageTransparentMock(QObject *parent = nullptr)
        : ImageTransparentInterface(parent)
    {}

    bool save(const QString &path, const char *format, int quality) const override
    {
        return mock_save(path, format, quality);
    }
    bool overwriteSave(const QString &path, const char *format, int quality) const override
    {
        return mock_overwriteSave(path, format, quality);
    }
    const QFileInfo &fileInfo(unsigned int index = 0) const override
    {
        return mock_fileInfo(index);
    }
    const QImage &original() const override { return mock_original(); }
    void addTransparentColor(const QColor &targetColor) override
    {
        mock_addTransparentColor(targetColor);
    }
    void addTransparentPixel(const QPoint &start) override { mock_addTransparentPixel(start); }

protected:
    bool loadImpl(const QString &path) override { return mock_loadImpl(path); }
    void resetImpl() override { mock_resetImpl(); }
    bool updateImpl() override { return mock_updateImpl(); }
};

class TestImageTransparentGUI : public QObject
{
    Q_OBJECT

    const QString testDirName = "test_image_transparent_gui_resources";
    const QString testDirPath = TEST_BIN_DIR + "/" + testDirName + "/";
    const QStringList resourceNames = {
        "320px-Qt_logo_2016.png", // 0
    };
    QImage image320;
    QSize size320;

    RandomData rd;

private slots:
    void initTestCase();    // will be called before the first test function is executed.
    void cleanupTestCase(); // will be called after the last test function was executed.
    void init();            // will be called before each test function is executed.

    // Test cases:
    void test_constructor();
    void test_onLoadImageSelected();
    void test_onSaveImageSelected();
    void test_onResetButtonClicked();
    void test_onColorModeTextChanged();
    void test_onPixelSelected();
    void test_onToleranceValueChanged();
    void test_onTransparencyValueChanged();
    void test_onContiguousAreaCheckStateChanged();
};

void TestImageTransparentGUI::initTestCase()
{
    QDir dir(TEST_BIN_DIR);
    dir.mkpath(testDirName);

    for (const QString &src : resourceNames)
        QFile::copy(TEST_SRC_DIR + "/core/image/" + src, testDirPath + src);

    image320 = QImage(testDirPath + resourceNames[0]);

    size320 = image320.size();
}

void TestImageTransparentGUI::cleanupTestCase()
{
    QDir testDir(testDirPath);
    testDir.removeRecursively();
}

void TestImageTransparentGUI::init()
{
    mock_save.clearFunction();
    mock_overwriteSave.clearFunction();
    mock_fileInfo.clearFunction();
    mock_original.clearFunction();
    mock_loadImpl.clearFunction();
    mock_resetImpl.clearFunction();
    mock_updateImpl.clearFunction();
    mock_addTransparentColor.clearFunction();
    mock_addTransparentPixel.clearFunction();
}

void TestImageTransparentGUI::test_constructor()
{ /*
    {
        ImageTransparentGUI gui(new ImageTransparentMock);

        // 親ウィジェットが設定されていなければ設定されること
        QCOMPARE_EQ(gui.imageTransparent->parent(), &gui);

        // imageViewが差し替えられていること
        QVERIFY(gui.ui->imageView->metaObject()->metaType().name()
                == QString("ImageViewForImageTransparent"));

        // 色空間の初期値のインデックスが0であること
        QCOMPARE_EQ(gui.ui->colorMode->currentIndex(), 0);
        // 色空間の選択肢が"RGB", "HSL", "HSV"であること
        QCOMPARE_EQ(gui.ui->colorMode->count(), 3);
        QCOMPARE_EQ(gui.ui->colorMode->itemText(0), "RGB");
        QCOMPARE_EQ(gui.ui->colorMode->itemText(1), "HSL");
        QCOMPARE_EQ(gui.ui->colorMode->itemText(2), "HSV");

        // 透明度の初期値が1.0であること
        QCOMPARE_EQ(gui.ui->transparencyValue->value(), 1.0);

        // 透明度の範囲が[0.0, 1.0]であること
        QCOMPARE_EQ(gui.ui->transparencyValue->minimum(), 0.0);
        QCOMPARE_EQ(gui.ui->transparencyValue->maximum(), 1.0);

        // 閾値の初期値が0.1であること
        QVERIFY(isEqaulApprox(gui.ui->toleranceValue->value(), 0.1));

        // 閾値の範囲が[0.0, 1.0]であること
        QCOMPARE_EQ(gui.ui->toleranceValue->minimum(), 0.0);
        QCOMPARE_EQ(gui.ui->toleranceValue->maximum(), 1.0);

        // 連続領域の初期値がtrueであること
        QCOMPARE_EQ(gui.ui->contiguousArea->checkState(), Qt::CheckState::Checked);
        QCOMPARE_EQ(gui.onlyContiguousArea, true);
    }
*/
    {
        ImageTransparentGUI gui(new ImageTransparentMock(this));

        // 親ウィジェットが設定されていれば変更されないこと
        QCOMPARE_EQ(gui.imageTransparent->parent(), this);
    }
}

void TestImageTransparentGUI::test_onLoadImageSelected()
{
    ImageTransparentGUI gui(new ImageTransparentMock);

    gui.onLoadImageSelected(rd.nextQString());

    // load() が呼ばれていること
    QVERIFY(mock_loadImpl.isInvoked());
}

void TestImageTransparentGUI::test_onSaveImageSelected()
{
    ImageTransparentGUI gui(new ImageTransparentMock);

    gui.onSaveImageSelected(rd.nextQString());

    // overwriteSave() が呼ばれていること
    QVERIFY(mock_overwriteSave.isInvoked());
}

void TestImageTransparentGUI::test_onResetButtonClicked()
{
    ImageTransparentGUI gui(new ImageTransparentMock);

    gui.onResetButtonClicked();

    // reset() が呼ばれていること
    QVERIFY(mock_resetImpl.isInvoked());
}

void TestImageTransparentGUI::test_onColorModeTextChanged()
{
    ImageTransparentGUI gui(new ImageTransparentMock);

    gui.onColorModeTextChanged("RGB");
    // QColor::Spec::Rgbに設定されること
    QCOMPARE_EQ(gui.imageTransparent->colorSpec, QColor::Spec::Rgb);

    gui.onColorModeTextChanged("HSV");
    // QColor::Spec::Hsvに設定されること
    QCOMPARE_EQ(gui.imageTransparent->colorSpec, QColor::Spec::Hsv);

    gui.onColorModeTextChanged("HSL");
    // QColor::Spec::Hslに設定されること
    QCOMPARE_EQ(gui.imageTransparent->colorSpec, QColor::Spec::Hsl);

    QString dummy;
    do {
        dummy = rd.nextQString(3, RandomData::upperAlphabets);
    } while (dummy == "RGB" || dummy == "HSV" || dummy == "HSL");
    // "RGB", "HSL", "HSL"以外の場合は例外が発生すること
    QVERIFY_THROWS_EXCEPTION(InvalidArgumentException<QString>, gui.onColorModeTextChanged(dummy));
}

void TestImageTransparentGUI::test_onPixelSelected()
{ /*
    ImageTransparentGUI gui(new ImageTransparentMock);

    QPoint point = QPoint(rd.nextInt(), rd.nextInt());
    QColor color = QColor(rd.nextInt(256), rd.nextInt(256), rd.nextInt(256));

    gui.onPixelSelected(point, color);
    // 色が設定されること
    QCOMPARE_EQ(gui.ui->colorSample->color(), color);
    // addTransparentPixelが呼ばれること
    QVERIFY(mock_addTransparentPixel.isInvoked());
    QVERIFY(!mock_addTransparentColor.isInvoked());

    gui.onlyContiguousArea = false;
    point = QPoint(rd.nextInt(), rd.nextInt());
    color = QColor(rd.nextInt(256), rd.nextInt(256), rd.nextInt(256));
    mock_addTransparentPixel.resetCount();
    mock_addTransparentColor.resetCount();
    gui.onPixelSelected(point, color);
    // 色が設定されること
    QCOMPARE_EQ(gui.ui->colorSample->color(), color);
    // addTransparentColorが呼ばれること
    QVERIFY(!mock_addTransparentPixel.isInvoked());
    QVERIFY(mock_addTransparentColor.isInvoked());*/
}

void TestImageTransparentGUI::test_onToleranceValueChanged()
{
    ImageTransparentGUI gui(new ImageTransparentMock);

    const double randomTolerance = rd.nextDouble();
    gui.onToleranceValueChanged(randomTolerance);
    // 閾値が設定されること
    QCOMPARE_EQ(gui.imageTransparent->tolerance, randomTolerance);

    // 例外が発生すること
    QVERIFY_THROWS_EXCEPTION(InvalidArgumentException<double>, gui.onToleranceValueChanged(-1));
    QVERIFY_THROWS_EXCEPTION(InvalidArgumentException<double>, gui.onToleranceValueChanged(2));
}

void TestImageTransparentGUI::test_onTransparencyValueChanged()
{
    ImageTransparentGUI gui(new ImageTransparentMock);

    const double randomTransparency = rd.nextDouble();
    gui.onTransparencyValueChanged(randomTransparency);
    // [0, 256)の不透明度が設定されること
    QCOMPARE_EQ(gui.imageTransparent->opacity, static_cast<uchar>(255 * (1.0 - randomTransparency)));

    // 例外が発生すること
    QVERIFY_THROWS_EXCEPTION(InvalidArgumentException<double>,
                             gui.onTransparencyValueChanged(-0.01));
    QVERIFY_THROWS_EXCEPTION(InvalidArgumentException<double>, gui.onTransparencyValueChanged(1.01));
}

void TestImageTransparentGUI::test_onContiguousAreaCheckStateChanged()
{
    ImageTransparentGUI gui(new ImageTransparentMock);

    // 連続領域フラグがtrueに設定されること
    gui.onContiguousAreaCheckStateChanged(Qt::CheckState::Checked);
    QCOMPARE_EQ(gui.onlyContiguousArea, true);

    // 連続領域フラグがfalseに設定されること
    gui.onContiguousAreaCheckStateChanged(Qt::CheckState::Unchecked);
    QCOMPARE_EQ(gui.onlyContiguousArea, false);
}
} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
//QTEST_APPLESS_MAIN(Test::TestImageTransparentGUI)

// QCoreApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
//QTEST_GUILESS_MAIN(Test::TestImageTransparentGUI)

// QApplicationもしくはQGuiApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
QTEST_MAIN(Test::TestImageTransparentGUI)

#include "test_image_transparent_gui.moc"

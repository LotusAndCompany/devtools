#include "tests/test_util.h"

#include <QtTest>

#define _TEST_ImageRotationGUI
#include "gui/image/rotation/image_rotation_gui.h"
#undef _TEST_ImageRotationGUI

#include "core/image/rotation/image_rotation.h"

#include <QObject>
#include <QSignalSpy>

namespace Test {

// function mocks
FunctionMock<bool(const QString &, const char *format, int)> mock_save, mock_overwriteSave;
FunctionMock<const QFileInfo &(unsigned int)> mock_fileInfo;
FunctionMock<void(double)> mock_rotateDegrees;
FunctionMock<void()> mock_flipHorizontal, mock_flipVertical;
FunctionMock<void(bool)> mock_setSmoothTransformationEnabled;
FunctionMock<const QImage &()> mock_original;
FunctionMock<bool(const QString &)> mock_loadImpl;
FunctionMock<void()> mock_resetImpl;
FunctionMock<bool()> mock_updateImpl;

class ImageRotationMock : public ImageRotationInterface
{
    Q_OBJECT

public:
    explicit ImageRotationMock(QObject *parent = nullptr) : ImageRotationInterface(parent) {}

    bool save(const QString &path, const char *format = nullptr, int quality = -1) const override
    {
        return mock_save(path, format, quality);
    }
    bool overwriteSave(const QString &path, const char *format = nullptr,
                       int quality = -1) const override
    {
        return mock_overwriteSave(path, format, quality);
    }
    const QFileInfo &fileInfo(unsigned int index = 0) const override
    {
        return mock_fileInfo(index);
    }

    void rotateDegrees(double deg) override { mock_rotateDegrees(deg); }
    void flipHorizontal() override { mock_flipHorizontal(); }
    void flipVertical() override { mock_flipVertical(); }
    const QImage &original() const override { return mock_original(); }
    void setSmoothTransformationEnabled(bool value = true) override
    {
        return mock_setSmoothTransformationEnabled(value);
    }

protected:
    bool loadImpl(const QString &path) override { return mock_loadImpl(path); }
    void resetImpl() override { mock_resetImpl(); }
    bool updateImpl() override { return mock_updateImpl(); }
};

class TestImageRotationGUI : public QObject
{
    Q_OBJECT

    const QString testDirName = "test_image_rotation_gui_resources";
    const QString testDirPath = TEST_BIN_DIR + "/" + testDirName + "/";
    const QStringList resourceNames = {
        "320px-Qt_logo_2016.png", // 0
    };
    QImage image320;
    QSize size320;

    const std::function<const QImage &(void)> original320 = [this]() {
        return this->image320;
    };

private slots:
    void initTestCase();    // will be called before the first test function is executed.
    void cleanupTestCase(); // will be called after the last test function was executed.
    void init();            // will be called before each test function is executed.

    // Test cases:
    void test_constructor();
    void test_onLoadImageSelected();
    void test_onSaveImageSelected();
    void test_onResetButtonClicked();
    void test_onRotateRightButtonClicked();
    void test_onRotateLeftButtonClicked();
    void test_onFlipHorizontalButtonClicked();
    void test_onFlipVerticalButtonClicked();
};

void TestImageRotationGUI::initTestCase()
{
    QDir dir(TEST_BIN_DIR);
    dir.mkpath(testDirName);

    for (const QString &src : resourceNames)
        QFile::copy(TEST_SRC_DIR + "/core/image/" + src, testDirPath + src);

    image320 = QImage(testDirPath + resourceNames[0]);

    size320 = image320.size();
}

void TestImageRotationGUI::cleanupTestCase()
{
    QDir testDir(testDirPath);
    testDir.removeRecursively();
}

void TestImageRotationGUI::init()
{
    mock_save.clearFunction();
    mock_overwriteSave.clearFunction();
    mock_fileInfo.clearFunction();
    mock_rotateDegrees.clearFunction();
    mock_flipHorizontal.clearFunction();
    mock_flipVertical.clearFunction();
    mock_setSmoothTransformationEnabled.clearFunction();
    mock_original.clearFunction();
    mock_loadImpl.clearFunction();
    mock_resetImpl.clearFunction();
    mock_updateImpl.clearFunction();
}

void TestImageRotationGUI::test_constructor()
{
    {
        ImageRotationGUI gui(new ImageRotationMock);

        // 親オブジェクトが設定されていない場合は自身が設定されること
        QCOMPARE_EQ(gui.imageRotation->parent(), &gui);
    }

    {
        ImageRotationGUI gui(new ImageRotationMock(this));

        // 親オブジェクトが設定されている場合はそれが変わらないこと
        QCOMPARE_EQ(gui.imageRotation->parent(), this);
    }
}

void TestImageRotationGUI::test_onLoadImageSelected()
{
    ImageRotationGUI gui(new ImageRotationMock);

    gui.onLoadImageSelected("");

    // load() が呼ばれていること
    QVERIFY(mock_loadImpl.isInvoked());
}

void TestImageRotationGUI::test_onSaveImageSelected()
{
    ImageRotationGUI gui(new ImageRotationMock);

    gui.onSaveImageSelected("");

    // overwriteSave() が呼ばれていること
    QVERIFY(mock_overwriteSave.isInvoked());
}

void TestImageRotationGUI::test_onResetButtonClicked()
{
    ImageRotationGUI gui(new ImageRotationMock);

    gui.onResetButtonClicked();

    // reset() が呼ばれていること
    QVERIFY(mock_resetImpl.isInvoked());
}

void TestImageRotationGUI::test_onRotateRightButtonClicked()
{
    ImageRotationGUI gui(new ImageRotationMock);

    // 画像が設定されていない場合も何もしないこと
    gui.onRotateRightButtonClicked();
    QVERIFY(!mock_rotateDegrees.isInvoked());

    // 画像が設定されていればrotateDegreesが呼ばれること
    mock_original.setFunction(original320);
    gui.onRotateRightButtonClicked();
    QVERIFY(mock_rotateDegrees.isInvoked());
    // -90が渡されること
    QCOMPARE_EQ(std::get<0>(mock_rotateDegrees.argumentsHistory()), -90.0);
}

void TestImageRotationGUI::test_onRotateLeftButtonClicked()
{
    ImageRotationGUI gui(new ImageRotationMock);

    // 画像が設定されていない場合も何もしないこと
    gui.onRotateLeftButtonClicked();
    QVERIFY(!mock_rotateDegrees.isInvoked());

    // 画像が設定されていればrotateDegreesが呼ばれること
    mock_original.setFunction(original320);
    gui.onRotateLeftButtonClicked();
    QVERIFY(mock_rotateDegrees.isInvoked());
    // 90が渡されること
    QCOMPARE_EQ(std::get<0>(mock_rotateDegrees.argumentsHistory()), 90.0);
}

void TestImageRotationGUI::test_onFlipHorizontalButtonClicked()
{
    ImageRotationGUI gui(new ImageRotationMock);

    // 画像が設定されていない場合も何もしないこと
    gui.onFlipHorizontalButtonClicked();
    QVERIFY(!mock_flipHorizontal.isInvoked());

    // 画像が設定されていればflipHorizontalが呼ばれること
    mock_original.setFunction(original320);
    gui.onFlipHorizontalButtonClicked();
    QVERIFY(mock_flipHorizontal.isInvoked());
}

void TestImageRotationGUI::test_onFlipVerticalButtonClicked()
{
    ImageRotationGUI gui(new ImageRotationMock);

    // 画像が設定されていない場合も何もしないこと
    gui.onFlipVerticalButtonClicked();
    QVERIFY(!mock_flipVertical.isInvoked());

    // 画像が設定されていればflipVerticalが呼ばれること
    mock_original.setFunction(original320);
    gui.onFlipVerticalButtonClicked();
    QVERIFY(mock_flipVertical.isInvoked());
}
} // namespace Test

// WARNING: gui/gui_application.hの機能は使えない
QTEST_MAIN(Test::TestImageRotationGUI)

#include "test_image_rotation_gui.moc"

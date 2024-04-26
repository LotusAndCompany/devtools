#include <QtTest>
#include "tests/test_util.h"

#define _TEST_ImageResizeGUI
#include "gui/image/resize/image_resize_gui.h"
#undef _TEST_ImageResizeGUI

#include <QObject>
#include <QSignalSpy>
#include "core/image/resize/image_resize.h"

namespace Test {

// function mocks
FunctionMock<bool, const QString &, const char *, int> mock_save, mock_overwriteSave;
FunctionMock<const QFileInfo &, unsigned int> mock_fileInfo;
FunctionMock<void, double, double> mock_setScale;
FunctionMock<void, double> mock_setScaleX, mock_setScaleY;
FunctionMock<void, const QSize &> mock_setSize;
FunctionMock<void, unsigned int, bool> mock_setWidth, mock_setHeight;
FunctionMock<QSize> mock_computedSize;
FunctionMock<double> mock_computedScaleX, mock_computedScaleY;
FunctionMock<void, bool> mock_setSmoothTransformationEnabled;
FunctionMock<const QImage &> mock_original;
FunctionMock<bool, const QString &> mock_loadImpl;
FunctionMock<> mock_resetImpl;
FunctionMock<bool> mock_updateImpl;

class ImageResizeMock : public ImageResizeInterface
{
    Q_OBJECT

public:
    explicit ImageResizeMock(QObject *parent = nullptr)
        : ImageResizeInterface(parent)
    {}

    bool save(const QString &path, const char *format, int quality) const override
    {
        return mock_save(path, format, quality);
    }
    bool overwriteSave(const QString &path, const char *format, int quality) const override
    {
        return mock_overwriteSave(path, format, quality);
    }
    const QFileInfo &fileInfo(unsigned int index) const override { return mock_fileInfo(index); }

    void setScale(double sx, double sy) noexcept(false) override { mock_setScale(sx, sy); }
    void setScaleX(double sx) noexcept(false) override { mock_setScaleX(sx); }
    void setScaleY(double sy) noexcept(false) override { mock_setScaleY(sy); }
    void setSize(const QSize &size) noexcept(false) override { mock_setSize(size); }
    void setWidth(unsigned int w, bool keepAspectRatio) override
    {
        mock_setWidth(w, keepAspectRatio);
    }
    void setHeight(unsigned int h, bool keepAspectRatio) override
    {
        mock_setHeight(h, keepAspectRatio);
    }
    QSize computedSize() const override { return mock_computedSize(); }
    double computedScaleX() const override { return mock_computedScaleX(); }
    double computedScaleY() const override { return mock_computedScaleY(); }
    void setSmoothTransformationEnabled(bool value) override
    {
        mock_setSmoothTransformationEnabled(value);
    }
    const QImage &original() const override { return mock_original(); }

protected:
    bool loadImpl(const QString &path) override { return mock_loadImpl(path); }
    void resetImpl() override { mock_resetImpl(); }
    bool updateImpl() override { return mock_updateImpl(); }
};

class TestImageResizeGUI : public QObject
{
    Q_OBJECT

    const QString testDirName = "test_image_resize_gui_resources";
    const QString testDirPath = TEST_BIN_DIR + "/" + testDirName + "/";
    const QStringList resourceNames = {
        "320px-Qt_logo_2016.png", // 0
        "578px-Qt_logo_2016.png", // 1
    };
    QImage image320, image578;
    //QSize size320, size578;

    const std::function<const QImage &(void)> original320 = [this]() { return this->image320; };

    RandomData rd;

private slots:
    void initTestCase(); // will be called before the first test function is executed.
    void cleanupTestCase(); // will be called after the last test function was executed.
    void init();         // will be called before each test function is executed.
    void cleanup();      // will be called after every test function.

    // Test cases:
    void test_constructor();
    void test_onLoadImageSelected();
    void test_onSaveImageSelected();
    void test_onResetButtonClicked();
    void test_onWidthValueChanged();
    void test_onHeightValueChanged();
    void test_onHScaleValueChanged();
    void test_onVScaleValueChanged();
    void test_onKeepAspectRatioChanged();
    void test_onSmoothTransformationChanged();
    void test_updateUIValues();
};

void TestImageResizeGUI::initTestCase()
{
    QDir dir(TEST_BIN_DIR);
    dir.mkpath(testDirName);

    for (const QString &src : resourceNames)
        QFile::copy(TEST_SRC_DIR + "/core/image/" + src, testDirPath + src);

    image320 = QImage(testDirPath + resourceNames[0]);
    image578 = QImage(testDirPath + resourceNames[1]);

    //size320 = QImage(TEST_SRC_DIR + "/core/image/" + resourceNames[0]).size();
    //size578 = QImage(TEST_SRC_DIR + "/core/image/" + resourceNames[1]).size();
}

void TestImageResizeGUI::cleanupTestCase()
{
    QDir testDir(testDirPath);
    testDir.removeRecursively();
}

void TestImageResizeGUI::init()
{
    mock_save.clearFunction();
    mock_overwriteSave.clearFunction();
    mock_fileInfo.clearFunction();
    mock_setScale.clearFunction();
    mock_setScaleX.clearFunction();
    mock_setScaleY.clearFunction();
    mock_setSize.clearFunction();
    mock_setWidth.clearFunction();
    mock_setHeight.clearFunction();
    mock_computedSize.clearFunction();
    mock_computedScaleX.clearFunction();
    mock_computedScaleY.clearFunction();
    mock_setSmoothTransformationEnabled.clearFunction();
    mock_original.clearFunction();
    mock_loadImpl.clearFunction();
    mock_resetImpl.clearFunction();
    mock_updateImpl.clearFunction();

    /*
    QDir dir(TEST_BIN_DIR);
    dir.mkpath(testDirName);

    for (const QString &src : resourceNames)
        QFile::copy(TEST_SRC_DIR + "/core/image/" + src, testDirPath + src);
    */
}

void TestImageResizeGUI::cleanup()
{
    /*
    QDir testDir(testDirPath);
    testDir.removeRecursively();
    */
}

void TestImageResizeGUI::test_constructor()
{
    {
        ImageResizeGUI gui(new ImageResizeMock);

        // 親オブジェクトが設定されていない場合は自身が設定されること
        QVERIFY(gui.imageResize->parent() == &gui);

        // 初期値がfalseであること
        QVERIFY(gui.keepAspectRatio == false);

        // 初期状態でOFFであること
        QVERIFY(gui.ui->keepAspectRatio->checkState() == Qt::Unchecked);
        QVERIFY(gui.ui->smoothScaling->checkState() == Qt::Unchecked);

        // 初期値が0であること
        QVERIFY(gui.ui->widthValue->value() == 0);
        QVERIFY(gui.ui->heightValue->value() == 0);

        // 初期値が100.0であること
        QVERIFY(gui.ui->hScaleValue->value() == 100.0);
        QVERIFY(gui.ui->vScaleValue->value() == 100.0);
    }

    {
        ImageResizeGUI gui(new ImageResizeMock(this));

        // 親オブジェクトが設定されている場合はそれが変わらないこと
        QVERIFY(gui.imageResize->parent() == this);
    }
}

void TestImageResizeGUI::test_onLoadImageSelected()
{
    ImageResizeGUI gui(new ImageResizeMock);

    gui.onLoadImageSelected("");

    // load() が呼ばれていること
    QVERIFY(mock_loadImpl.isInvoked());
}

void TestImageResizeGUI::test_onSaveImageSelected()
{
    ImageResizeGUI gui(new ImageResizeMock);

    gui.onSaveImageSelected("");

    // overwriteSave() が呼ばれていること
    QVERIFY(mock_overwriteSave.isInvoked());
}

void TestImageResizeGUI::test_onResetButtonClicked()
{
    ImageResizeGUI gui(new ImageResizeMock);

    gui.onResetButtonClicked();

    // reset() が呼ばれていること
    QVERIFY(mock_resetImpl.isInvoked());
}

void TestImageResizeGUI::test_onWidthValueChanged()
{
    ImageResizeGUI gui(new ImageResizeMock);

    // 0未満の値が渡された場合は何もしないこと
    gui.onWidthValueChanged(-1);
    QVERIFY(mock_setWidth.isInvoked() == false);

    // 画像が設定されていない場合も何もしないこと
    gui.onWidthValueChanged(1);
    QVERIFY(mock_setWidth.isInvoked() == false);

    // 画像が設定されている場合、setWidthが呼ばれること
    mock_original.setFunction(original320);
    gui.onWidthValueChanged(1);
    QVERIFY(mock_setWidth.isInvoked() == true);
}

void TestImageResizeGUI::test_onHeightValueChanged()
{
    ImageResizeGUI gui(new ImageResizeMock);

    // 0未満の値が渡された場合は何もしないこと
    gui.onHeightValueChanged(-1);
    QVERIFY(mock_setHeight.isInvoked() == false);

    // 画像が設定されていない場合も何もしないこと
    gui.onHeightValueChanged(1);
    QVERIFY(mock_setHeight.isInvoked() == false);

    // 画像が設定されている場合、setHeightが呼ばれること
    mock_original.setFunction(original320);
    gui.onHeightValueChanged(1);
    QVERIFY(mock_setHeight.isInvoked() == true);
}

void TestImageResizeGUI::test_onHScaleValueChanged()
{
    ImageResizeGUI gui(new ImageResizeMock);

    // 0未満の値が渡された場合は何もしないこと
    gui.onHorizontalScaleChanged(-1);
    QVERIFY(mock_setScaleX.isInvoked() == false);
    QVERIFY(mock_setScale.isInvoked() == false);

    // 画像が設定されていない場合も何もしないこと
    gui.onHorizontalScaleChanged(2);
    QVERIFY(mock_setScaleX.isInvoked() == false);
    QVERIFY(mock_setScale.isInvoked() == false);

    // 画像が設定されていてkeepAspectRatio == falseの場合、setScaleXが呼ばれること
    mock_original.setFunction(original320);
    gui.onHorizontalScaleChanged(2);
    QVERIFY(mock_setScaleX.isInvoked() == true);
    QVERIFY(mock_setScale.isInvoked() == false);

    mock_setScaleX.resetCount();

    // 画像が設定されていてkeepAspectRatio == trueの場合、setScaleが呼ばれること
    gui.keepAspectRatio = true;
    gui.onHorizontalScaleChanged(2);
    QVERIFY(mock_setScale.isInvoked() == true);
}

void TestImageResizeGUI::test_onVScaleValueChanged()
{
    ImageResizeGUI gui(new ImageResizeMock);

    // 0未満の値が渡された場合は何もしないこと
    gui.onVerticalScaleChanged(-1);
    QVERIFY(mock_setScaleY.isInvoked() == false);
    QVERIFY(mock_setScale.isInvoked() == false);

    // 画像が設定されていない場合も何もしないこと
    gui.onVerticalScaleChanged(2);
    QVERIFY(mock_setScaleY.isInvoked() == false);
    QVERIFY(mock_setScale.isInvoked() == false);

    // 画像が設定されていてkeepAspectRatio == falseの場合、setScaleYが呼ばれること
    mock_original.setFunction(original320);
    gui.onVerticalScaleChanged(2);
    QVERIFY(mock_setScaleY.isInvoked() == true);
    QVERIFY(mock_setScale.isInvoked() == false);

    mock_setScaleY.resetCount();

    // 画像が設定されていてkeepAspectRatio == trueの場合、setScaleが呼ばれること
    gui.keepAspectRatio = true;
    gui.onVerticalScaleChanged(2);
    QVERIFY(mock_setScale.isInvoked() == true);
}

void TestImageResizeGUI::test_onKeepAspectRatioChanged()
{
    ImageResizeGUI gui(new ImageResizeMock);

    // チェックされた時はkeepAspectRatioがtrueになること
    gui.onKeepAspectRatioChanged(Qt::Checked);
    QVERIFY(gui.keepAspectRatio == true);

    // チェックが外された時はkeepAspectRatioがfalseになること
    gui.onKeepAspectRatioChanged(Qt::Unchecked);
    QVERIFY(gui.keepAspectRatio == false);
}

void TestImageResizeGUI::test_onSmoothTransformationChanged()
{
    ImageResizeGUI gui(new ImageResizeMock);

    // チェックされた時はsetSmoothTransformationEnabledにtrueが渡されること
    gui.onSmoothTransformationChanged(Qt::Checked);
    QVERIFY(mock_setSmoothTransformationEnabled.isInvoked() == true);
    QVERIFY(std::get<0>(mock_setSmoothTransformationEnabled.argumentsHistory()) == true);

    mock_setSmoothTransformationEnabled.resetCount();

    // チェックが外された時はsetSmoothTransformationEnabledにfalseが渡されること
    gui.onSmoothTransformationChanged(Qt::Unchecked);
    QVERIFY(mock_setSmoothTransformationEnabled.isInvoked() == true);
    QVERIFY(std::get<0>(mock_setSmoothTransformationEnabled.argumentsHistory()) == false);
}

void TestImageResizeGUI::test_updateUIValues()
{
    QSKIP("WIP");

    /*
    QSignalSpy spyWidthValueChanged(gui.ui->widthValue, &QSpinBox::valueChanged),
        spyHeightValueChanged(gui.ui->heightValue, &QSpinBox::valueChanged),
        spyHScaleValueChanged(gui.ui->hScaleValue, &QDoubleSpinBox::valueChanged),
        spyVScaleValueChanged(gui.ui->vScaleValue, &QDoubleSpinBox::valueChanged);

    const double randomScaleX = rd.nextDouble(0.1, 10.0);
    const double randomScaleY = rd.nextDouble(0.1, 10.0);
    mock_computedScaleX.setFunction([randomScaleX]() { return randomScaleX; });
    mock_computedScaleY.setFunction([randomScaleY]() { return randomScaleY; });

    const QSize randomSize = QSize(rd.nextInt(3200), rd.nextInt(3200));
    mock_computedSize.setFunction([&randomSize]() { return randomSize; });

    // 各シグナルが発せられないこと
    QVERIFY(spyWidthValueChanged.count() == 0);
    QVERIFY(spyHeightValueChanged.count() == 0);
    QVERIFY(spyHScaleValueChanged.count() == 0);
    QVERIFY(spyVScaleValueChanged.count() == 0);

    // originalが空の場合はサイズ0、拡大率100になること
    QVERIFY(gui.ui->widthValue->value() == 0);
    QVERIFY(gui.ui->heightValue->value() == 0);
    QVERIFY(gui.ui->hScaleValue->value() == 100.0);
    QVERIFY(gui.ui->vScaleValue->value() == 100.0);

    QImage original(TEST_SRC_DIR + "/core/image/" + resourceNames[0]);
    if (original.isNull())
        QFAIL("image is empty");
    mock_original.setFunction([&original]() { return original; });
    gui.onResetButtonClicked();

    // computedSize, computedScaleが設定されていること
    */
}
} // namespace Test

// WARNING: gui/gui_application.hの機能は使えない
QTEST_MAIN(Test::TestImageResizeGUI)

#include "test_image_resize_gui.moc"

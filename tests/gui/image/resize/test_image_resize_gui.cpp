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
FunctionMock<bool(const QString &, const char *, int)> mock_save, mock_overwriteSave;
FunctionMock<const QFileInfo &(unsigned int)> mock_fileInfo;
FunctionMock<void(double, double)> mock_setScale;
FunctionMock<void(double)> mock_setScaleX, mock_setScaleY;
FunctionMock<void(const QSize &)> mock_setSize;
FunctionMock<void(unsigned int, bool)> mock_setWidth, mock_setHeight;
FunctionMock<QSize()> mock_computedSize;
FunctionMock<double()> mock_computedScaleX, mock_computedScaleY;
FunctionMock<void(bool)> mock_setSmoothTransformationEnabled;
FunctionMock<const QImage &()> mock_original;
FunctionMock<bool(const QString &)> mock_loadImpl;
FunctionMock<void()> mock_resetImpl;
FunctionMock<bool()> mock_updateImpl;

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
    };
    QImage image320;
    QSize size320;

    const std::function<const QImage &(void)> original320 = [this]() { return this->image320; };

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

    size320 = image320.size();
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
}

void TestImageResizeGUI::test_constructor()
{
    {
        ImageResizeGUI gui(new ImageResizeMock);

        // 親オブジェクトが設定されていない場合は自身が設定されること
        QCOMPARE_EQ(gui.imageResize->parent(), &gui);

        // 初期値がfalseであること
        QCOMPARE_EQ(gui.keepAspectRatio, false);

        // 初期状態でOFFであること
        QCOMPARE_EQ(gui.ui->keepAspectRatio->checkState(), Qt::Unchecked);
        QCOMPARE_EQ(gui.ui->smoothScaling->checkState(), Qt::Unchecked);

        // 初期値が0であること
        QCOMPARE_EQ(gui.ui->widthValue->value(), 0);
        QCOMPARE_EQ(gui.ui->heightValue->value(), 0);

        // 初期値が100.0であること
        QCOMPARE_EQ(gui.ui->hScaleValue->value(), 100.0);
        QCOMPARE_EQ(gui.ui->vScaleValue->value(), 100.0);
    }

    {
        ImageResizeGUI gui(new ImageResizeMock(this));

        // 親オブジェクトが設定されている場合はそれが変わらないこと
        QCOMPARE_EQ(gui.imageResize->parent(), this);
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

    gui.ui->keepAspectRatio->setCheckState(Qt::Checked);
    gui.ui->smoothScaling->setCheckState(Qt::Checked);

    // reset() によりチェックボックスが戻らないこと
    QCOMPARE_EQ(gui.keepAspectRatio, true);
    QCOMPARE_EQ(gui.ui->keepAspectRatio->checkState(), Qt::Checked);
    QCOMPARE_EQ(gui.ui->smoothScaling->checkState(), Qt::Checked);
}

void TestImageResizeGUI::test_onWidthValueChanged()
{
    ImageResizeGUI gui(new ImageResizeMock);

    // 0未満の値が渡された場合は何もしないこと
    gui.onWidthValueChanged(-1);
    QVERIFY(!mock_setWidth.isInvoked());

    // 画像が設定されていない場合も何もしないこと
    gui.onWidthValueChanged(1);
    QVERIFY(!mock_setWidth.isInvoked());

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
    QVERIFY(!mock_setHeight.isInvoked());

    // 画像が設定されていない場合も何もしないこと
    gui.onHeightValueChanged(1);
    QVERIFY(!mock_setHeight.isInvoked());

    // 画像が設定されている場合、setHeightが呼ばれること
    mock_original.setFunction(original320);
    gui.onHeightValueChanged(1);
    QVERIFY(mock_setHeight.isInvoked());
}

void TestImageResizeGUI::test_onHScaleValueChanged()
{
    ImageResizeGUI gui(new ImageResizeMock);

    // 0未満の値が渡された場合は何もしないこと
    gui.onHorizontalScaleChanged(-1);
    QVERIFY(!mock_setScaleX.isInvoked());
    QVERIFY(!mock_setScale.isInvoked());

    // 画像が設定されていない場合も何もしないこと
    gui.onHorizontalScaleChanged(2);
    QVERIFY(!mock_setScaleX.isInvoked());
    QVERIFY(!mock_setScale.isInvoked());

    // 画像が設定されていてkeepAspectRatio == falseの場合、setScaleXが呼ばれること
    mock_original.setFunction(original320);
    gui.onHorizontalScaleChanged(2);
    QVERIFY(mock_setScaleX.isInvoked());
    QVERIFY(!mock_setScale.isInvoked());

    mock_setScaleX.resetCount();

    // 画像が設定されていてkeepAspectRatio == trueの場合、setScaleが呼ばれること
    gui.keepAspectRatio = true;
    gui.onHorizontalScaleChanged(2);
    QVERIFY(mock_setScale.isInvoked());
}

void TestImageResizeGUI::test_onVScaleValueChanged()
{
    ImageResizeGUI gui(new ImageResizeMock);

    // 0未満の値が渡された場合は何もしないこと
    gui.onVerticalScaleChanged(-1);
    QVERIFY(!mock_setScaleY.isInvoked());
    QVERIFY(!mock_setScale.isInvoked());

    // 画像が設定されていない場合も何もしないこと
    gui.onVerticalScaleChanged(2);
    QVERIFY(!mock_setScaleY.isInvoked());
    QVERIFY(!mock_setScale.isInvoked());

    // 画像が設定されていてkeepAspectRatio == falseの場合、setScaleYが呼ばれること
    mock_original.setFunction(original320);
    gui.onVerticalScaleChanged(2);
    QVERIFY(mock_setScaleY.isInvoked());
    QVERIFY(!mock_setScale.isInvoked());

    mock_setScaleY.resetCount();

    // 画像が設定されていてkeepAspectRatio == trueの場合、setScaleが呼ばれること
    gui.keepAspectRatio = true;
    gui.onVerticalScaleChanged(2);
    QVERIFY(mock_setScale.isInvoked());
}

void TestImageResizeGUI::test_onKeepAspectRatioChanged()
{
    ImageResizeGUI gui(new ImageResizeMock);

    // チェックされた時はkeepAspectRatioがtrueになること
    gui.onKeepAspectRatioChanged(Qt::Checked);
    QCOMPARE_EQ(gui.keepAspectRatio, true);

    // チェックが外された時はkeepAspectRatioがfalseになること
    gui.onKeepAspectRatioChanged(Qt::Unchecked);
    QCOMPARE_EQ(gui.keepAspectRatio, false);
}

void TestImageResizeGUI::test_onSmoothTransformationChanged()
{
    ImageResizeGUI gui(new ImageResizeMock);

    // チェックされた時はsetSmoothTransformationEnabledにtrueが渡されること
    gui.onSmoothTransformationChanged(Qt::Checked);
    QVERIFY(mock_setSmoothTransformationEnabled.isInvoked());
    auto history = mock_setSmoothTransformationEnabled.argumentsHistory();
    QCOMPARE_EQ(std::get<0>(history), true);

    mock_setSmoothTransformationEnabled.resetCount();

    // チェックが外された時はsetSmoothTransformationEnabledにfalseが渡されること
    gui.onSmoothTransformationChanged(Qt::Unchecked);
    QVERIFY(mock_setSmoothTransformationEnabled.isInvoked());
    history = mock_setSmoothTransformationEnabled.argumentsHistory();
    QCOMPARE_EQ(std::get<0>(history), false);
}

void TestImageResizeGUI::test_updateUIValues()
{
    ImageResizeGUI gui(new ImageResizeMock);

    QSignalSpy spyWidthValueChanged(gui.ui->widthValue, &QSpinBox::valueChanged),
        spyHeightValueChanged(gui.ui->heightValue, &QSpinBox::valueChanged),
        spyHScaleValueChanged(gui.ui->hScaleValue, &QDoubleSpinBox::valueChanged),
        spyVScaleValueChanged(gui.ui->vScaleValue, &QDoubleSpinBox::valueChanged);

    const double randomScaleX = rd.nextDouble(10.0, 1000.0);
    const double randomScaleY = rd.nextDouble(10.0, 1000.0);
    mock_computedScaleX.setFunction([randomScaleX]() { return randomScaleX / 100.0; });
    mock_computedScaleY.setFunction([randomScaleY]() { return randomScaleY / 100.0; });

    const QSize randomSize = QSize(rd.nextInt(3200), rd.nextInt(3200));
    mock_computedSize.setFunction([&randomSize]() { return randomSize; });

    gui.updateUIValues();

    // 各シグナルが発せられないこと
    QCOMPARE_EQ(spyWidthValueChanged.count(), 0);
    QCOMPARE_EQ(spyHeightValueChanged.count(), 0);
    QCOMPARE_EQ(spyHScaleValueChanged.count(), 0);
    QCOMPARE_EQ(spyVScaleValueChanged.count(), 0);

    // originalが空の場合はサイズ0、拡大率100になること
    QCOMPARE_EQ(gui.ui->widthValue->value(), 0);
    QCOMPARE_EQ(gui.ui->heightValue->value(), 0);
    QCOMPARE_EQ(gui.ui->hScaleValue->value(), 100.0);
    QCOMPARE_EQ(gui.ui->vScaleValue->value(), 100.0);

    QImage original(testDirPath + resourceNames[0]);
    if (original.isNull())
        QFAIL("image is empty");
    mock_original.setFunction([&original]() { return original; });

    gui.onLoadImageSelected(testDirPath + resourceNames[0]);

    // width, height, hScale, vScaleが設定されていること
    gui.updateUIValues();
    QCOMPARE_EQ(gui.ui->widthValue->value(), randomSize.width());
    QCOMPARE_EQ(gui.ui->heightValue->value(), randomSize.height());
    QVERIFY(isEqaulApprox(gui.ui->hScaleValue->value(), randomScaleX));
    QVERIFY(isEqaulApprox(gui.ui->vScaleValue->value(), randomScaleY));
}
} // namespace Test

// WARNING: gui/gui_application.hの機能は使えない
QTEST_MAIN(Test::TestImageResizeGUI)

#include "test_image_resize_gui.moc"

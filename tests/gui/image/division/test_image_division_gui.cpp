#include "tests/test_util.h"

#include <QtTest>

#define _TEST_ImageDivisionGUI
#include "gui/image/division/image_division_gui.h"
#undef _TEST_ImageDivisionGUI

#include "core/image/division/image_division.h"

#include <QObject>
#include <QSignalSpy>

namespace Test {
// function mocks
FunctionMock<bool(const QString &, const char *, int)> mock_save, mock_overwriteSave;
FunctionMock<const QFileInfo &(unsigned int)> mock_fileInfo;
FunctionMock<const QImage &()> mock_original;
FunctionMock<bool(const QString &)> mock_loadImpl;
FunctionMock<void()> mock_resetImpl;
FunctionMock<bool()> mock_updateImpl;
FunctionMock<QString(const QDir &, unsigned int, unsigned int)> mock_saveFilename;
FunctionMock<QSizeF(void)> mock_computedCellSize;
FunctionMock<void(unsigned int)> mock_setHorizontalDivision, mock_setVerticalDivision,
    mock_setCellWidth, mock_setCellHeight;
FunctionMock<unsigned int(void)> mock_numberOfHorizontalDivision, mock_numberOfVerticalDivision;

class ImageDivisionMock : public ImageDivisionInterface
{
    Q_OBJECT

public:
    explicit ImageDivisionMock(QObject *parent = nullptr) : ImageDivisionInterface(parent) {}

    bool save(const QString &path, const char *format, int quality) const override
    {
        return mock_save(path, format, quality);
    }
    bool overwriteSave(const QString &path, const char *format, int quality) const override
    {
        return mock_overwriteSave(path, format, quality);
    }
    [[nodiscard]] const QFileInfo &fileInfo(unsigned int index = 0) const override
    {
        return mock_fileInfo(index);
    }
    [[nodiscard]] const QImage &original() const override { return mock_original(); }

    void setHorizontalDivision(unsigned int n) override { mock_setHorizontalDivision(n); }
    void setVerticalDivision(unsigned int m) override { mock_setVerticalDivision(m); }
    void setCellWidth(unsigned int width) override { mock_setCellWidth(width); }
    void setCellHeight(unsigned int height) override { mock_setCellHeight(height); }
    [[nodiscard]] QString saveFilename(const QDir &location, unsigned int x,
                                       unsigned int y) const override
    {
        return mock_saveFilename(location, x, y);
    }
    [[nodiscard]] QSizeF computedCellSize() const override { return mock_computedCellSize(); }
    [[nodiscard]] unsigned int numberOfHorizontalDivision() const override
    {
        return mock_numberOfHorizontalDivision();
    }
    [[nodiscard]] unsigned int numberOfVerticalDivision() const override
    {
        return mock_numberOfVerticalDivision();
    }

protected:
    bool loadImpl(const QString &path) override { return mock_loadImpl(path); }
    void resetImpl() override { mock_resetImpl(); }
    bool updateImpl() override { return mock_updateImpl(); }
};

class TestImageDivisionGUI : public QObject
{
    Q_OBJECT

    const QString testDirName = "test_image_division_gui_resources";
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
    static void init();     // will be called before each test function is executed.

    // Test cases:
    void test_constructor();
    void test_onLoadImageSelected();
    void test_onSaveLocationSelected();
    static void test_onResetButtonClicked();
    void test_onDivisionModeClicked();
    void test_onHorizontalDivisionValueChanged();
    void test_onVerticalDivisionValueChanged();
    void test_onWidthValueChanged();
    void test_onHeightValueChanged();
    void test_onDiscardRemainderToggled();
    void test_updateUI();
};

void TestImageDivisionGUI::initTestCase()
{
    QDir const dir(TEST_BIN_DIR);
    dir.mkpath(testDirName);

    for (const QString &src : resourceNames) {
        QFile::copy(TEST_SRC_DIR + "/core/image/" + src, testDirPath + src);
    }

    image320 = QImage(testDirPath + resourceNames[0]);

    size320 = image320.size();
}

void TestImageDivisionGUI::cleanupTestCase()
{
    QDir testDir(testDirPath);
    testDir.removeRecursively();
}

void TestImageDivisionGUI::init()
{
    mock_save.clearFunction();
    mock_overwriteSave.clearFunction();
    mock_fileInfo.clearFunction();
    mock_original.clearFunction();
    mock_loadImpl.clearFunction();
    mock_resetImpl.clearFunction();
    mock_updateImpl.clearFunction();
    mock_saveFilename.clearFunction();
    mock_computedCellSize.clearFunction();
    mock_setHorizontalDivision.clearFunction();
    mock_setVerticalDivision.clearFunction();
    mock_setCellWidth.clearFunction();
    mock_setCellHeight.clearFunction();
    mock_numberOfHorizontalDivision.clearFunction();
    mock_numberOfVerticalDivision.clearFunction();
}

void TestImageDivisionGUI::test_constructor()
{
    {
        ImageDivisionGUI gui(new ImageDivisionMock());

        // 親ウィジェットが設定されていなければ設定されること
        QCOMPARE_EQ(gui.imageDivision->parent(), &gui);

        /*
        // imageViewが差し替えられていること
        QVERIFY(gui.ui->imageView->metaObject()->metaType().name()
                == QString("ImageViewForImageDivision"));

        // 残りを切り捨てるフラグがtrueになっていること
        QCOMPARE_EQ(gui.ui->ignoreRemainders->checkState(), Qt::CheckState::Checked);

        // 分割数指定の方が選択されていること
        QCOMPARE_EQ(gui.ui->useDivisionButton->isChecked(), true);
        QCOMPARE_EQ(gui.ui->hDivValue->isEnabled(), true);
        QCOMPARE_EQ(gui.ui->vDivValue->isEnabled(), true);

        QCOMPARE_EQ(gui.ui->useSizeButton->isChecked(), false);
        QCOMPARE_EQ(gui.ui->widthValue->isEnabled(), false);
        QCOMPARE_EQ(gui.ui->heightValue->isEnabled(), false);

        // 想定通りの文字列が設定されていること
        QCOMPARE_EQ(gui.ui->sizeLabel->text(), tr("%1 x %2").arg(0).arg(0));

        // 分割数が1x1に設定されていること
        QCOMPARE_EQ(gui.ui->hDivValue->value(), 1);
        QCOMPARE_EQ(gui.ui->hDivValue->value(), 1);

        // 画像サイズが0x0に設定されていること
        QCOMPARE_EQ(gui.ui->widthValue->value(), 0);
        QCOMPARE_EQ(gui.ui->heightValue->value(), 0);
        */
    }

    {
        ImageDivisionGUI const gui(new ImageDivisionMock(this));

        // 親ウィジェットが設定されていれば変更されないこと
        QCOMPARE_EQ(gui.imageDivision->parent(), this);
    }
}

void TestImageDivisionGUI::test_onLoadImageSelected()
{
    /*
    ImageDivisionGUI gui(new ImageDivisionMock());
    QSignalSpy spyHDiv(gui.ui->hDivValue, &QSpinBox::valueChanged),
        spyVDiv(gui.ui->vDivValue, &QSpinBox::valueChanged),
        spyWidth(gui.ui->widthValue, &QSpinBox::valueChanged),
        spyHeight(gui.ui->heightValue, &QSpinBox::valueChanged);

    const QString path = rd.nextQString();
    gui.onLoadImageSelected(path);

    // loadImplが呼び出されること
    QVERIFY(mock_loadImpl.isInvoked());
    QCOMPARE_EQ(std::get<0>(mock_loadImpl.argumentsHistory()), path);

    // シグナルが発せられないこと
    QCOMPARE_EQ(spyHDiv.count(), 0);
    QCOMPARE_EQ(spyVDiv.count(), 0);
    QCOMPARE_EQ(spyWidth.count(), 0);
    QCOMPARE_EQ(spyHeight.count(), 0);
    */
}

void TestImageDivisionGUI::test_onSaveLocationSelected()
{
    ImageDivisionGUI gui(new ImageDivisionMock());

    const QString path = rd.nextQString();
    gui.onSaveLocationSelected(path);

    // overwriteSaveが呼び出されること
    QVERIFY(mock_overwriteSave.isInvoked());
    QCOMPARE_EQ(std::get<0>(mock_overwriteSave.argumentsHistory()), path);
}

void TestImageDivisionGUI::test_onResetButtonClicked()
{
    ImageDivisionGUI gui(new ImageDivisionMock());

    gui.onResetButtonClicked();

    // resetImplが呼び出されること
    QVERIFY(mock_resetImpl.isInvoked());
}

void TestImageDivisionGUI::test_onDivisionModeClicked()
{
    /*
    ImageDivisionGUI gui(new ImageDivisionMock());

    QTest::mouseClick(gui.ui->useSizeButton, Qt::MouseButton::LeftButton);
    // サイズ指定の方が選択されること
    QCOMPARE_EQ(gui.ui->useDivisionButton->isChecked(), false);
    QCOMPARE_EQ(gui.ui->hDivValue->isEnabled(), false);
    QCOMPARE_EQ(gui.ui->vDivValue->isEnabled(), false);

    QCOMPARE_EQ(gui.ui->useSizeButton->isChecked(), true);
    QCOMPARE_EQ(gui.ui->widthValue->isEnabled(), true);
    QCOMPARE_EQ(gui.ui->heightValue->isEnabled(), true);

    QTest::mouseClick(gui.ui->useDivisionButton, Qt::MouseButton::LeftButton);
    // 分割数指定の方が選択されること
    QCOMPARE_EQ(gui.ui->useDivisionButton->isChecked(), true);
    QCOMPARE_EQ(gui.ui->hDivValue->isEnabled(), true);
    QCOMPARE_EQ(gui.ui->vDivValue->isEnabled(), true);

    QCOMPARE_EQ(gui.ui->useSizeButton->isChecked(), false);
    QCOMPARE_EQ(gui.ui->widthValue->isEnabled(), false);
    QCOMPARE_EQ(gui.ui->heightValue->isEnabled(), false);
    */
}

void TestImageDivisionGUI::test_onHorizontalDivisionValueChanged()
{
    ImageDivisionGUI gui(new ImageDivisionMock());

    const auto v = rd.nextInt();
    gui.onHorizontalDivisionValueChanged(v);
    // setHorizontalDivisionが呼び出されること
    QVERIFY(mock_setHorizontalDivision.isInvoked());
    QCOMPARE_EQ(std::get<0>(mock_setHorizontalDivision.argumentsHistory()), v);
}

void TestImageDivisionGUI::test_onVerticalDivisionValueChanged()
{
    ImageDivisionGUI gui(new ImageDivisionMock());

    const auto v = rd.nextInt();
    gui.onVerticalDivisionValueChanged(v);
    // setVerticalDivisionが呼び出されること
    QVERIFY(mock_setVerticalDivision.isInvoked());
    QCOMPARE_EQ(std::get<0>(mock_setVerticalDivision.argumentsHistory()), v);
}

void TestImageDivisionGUI::test_onWidthValueChanged()
{
    /*
    ImageDivisionGUI gui(new ImageDivisionMock());

    gui.ui->useSizeButton->setChecked(true);

    const auto v = rd.nextInt();
    gui.onWidthValueChanged(v);
    // setCellWidthが呼び出されること
    QVERIFY(mock_setCellWidth.isInvoked());
    QCOMPARE_EQ(std::get<0>(mock_setCellWidth.argumentsHistory()), v);
    */
}

void TestImageDivisionGUI::test_onHeightValueChanged()
{
    /*
    ImageDivisionGUI gui(new ImageDivisionMock());

    gui.ui->useSizeButton->setChecked(true);

    const auto v = rd.nextInt();
    gui.onHeightValueChanged(v);
    // setCellHeightが呼び出されること
    QVERIFY(mock_setCellHeight.isInvoked());
    QCOMPARE_EQ(std::get<0>(mock_setCellHeight.argumentsHistory()), v);
    */
}

void TestImageDivisionGUI::test_onDiscardRemainderToggled()
{
    /*
    ImageDivisionGUI gui(new ImageDivisionMock());

    QTest::mouseClick(gui.ui->ignoreRemainders, Qt::MouseButton::LeftButton);
    // discardRemaindersがfalseに設定されること
    QCOMPARE_EQ(gui.imageDivision->discardRemainders, false);

    QTest::mouseClick(gui.ui->ignoreRemainders, Qt::MouseButton::LeftButton);
    // discardRemaindersがtrueに設定されること
    QCOMPARE_EQ(gui.imageDivision->discardRemainders, true);
    */
}

void TestImageDivisionGUI::test_updateUI()
{
    /*
    ImageDivisionGUI gui(new ImageDivisionMock());
    QSignalSpy spyHDiv(gui.ui->hDivValue, &QSpinBox::valueChanged),
        spyVDiv(gui.ui->vDivValue, &QSpinBox::valueChanged),
        spyWidth(gui.ui->widthValue, &QSpinBox::valueChanged),
        spyHeight(gui.ui->heightValue, &QSpinBox::valueChanged);

    const auto hDiv = rd.nextInt(1, 8), vDiv = rd.nextInt(1, 8);
    // NOTE: SpinBoxの上限値は画像サイズに合わせて設定されるが、初期状態では99が上限
    const QSizeF cellSize = QSizeF(rd.nextDouble(8, 80), rd.nextDouble(8, 80));

    mock_numberOfHorizontalDivision.setFunction([hDiv]() { return hDiv; });
    mock_numberOfVerticalDivision.setFunction([vDiv]() { return vDiv; });
    mock_computedCellSize.setFunction([cellSize]() { return cellSize; });

    gui.updateUI();

    // numberOfHorizontalDivisionの返り値が設定されていること
    QCOMPARE_EQ(gui.ui->hDivValue->value(), hDiv);
    // numberOfVerticalDivisionの返り値が設定されていること
    QCOMPARE_EQ(gui.ui->vDivValue->value(), vDiv);
    // computedCellSizeの返り値が設定されていること
    QCOMPARE_EQ(gui.ui->widthValue->value(), (int) cellSize.width());
    QCOMPARE_EQ(gui.ui->heightValue->value(), (int) cellSize.height());

    // シグナルが発せられないこと
    QCOMPARE_EQ(spyHDiv.count(), 0);
    QCOMPARE_EQ(spyVDiv.count(), 0);
    QCOMPARE_EQ(spyWidth.count(), 0);
    QCOMPARE_EQ(spyHeight.count(), 0);
    */
}
} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
// QTEST_APPLESS_MAIN(Test::TestImageDivisionGUI)

// QCoreApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
// QTEST_GUILESS_MAIN(Test::TestImageDivisionGUI)

// QApplicationもしくはQGuiApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
QTEST_MAIN(Test::TestImageDivisionGUI)

#include "test_image_division_gui.moc"

#include <QtTest>
#include "tests/mock_helper.h"

#define _TEST_BasicImageEditInterface
#include "core/image/basic_image_edit_interface.h"
#undef _TEST_BasicImageEditInterface

#include <QFileInfo>
#include <QObject>
#include <QScopedPointer>
#include <QString>

namespace Test {

FunctionMock<bool, const QString &, const char *, int> mock_save;
FunctionMock<bool, const QString &, const char *, int> mock_overwriteSave;
FunctionMock<const QFileInfo &, unsigned int> mock_fileInfo;
FunctionMock<bool, const QString &> mock_loadImpl;
FunctionMock<void> mock_resetImpl;
FunctionMock<bool> mock_updateImpl;

class TestBasicImageEditInterface : public QObject
{
    Q_OBJECT

    class BasicImageEditInterfaceMock : public BasicImageEditInterface
    {
    public:
        bool save(const QString &path, const char *format, int quality) const override
        {
            return mock_save(path, format, quality);
        }

        bool overwriteSave(const QString &path, const char *format, int quality) const override
        {
            return mock_overwriteSave(path, format, quality);
        }

        const QFileInfo &fileInfo(unsigned int index) const override
        {
            return mock_fileInfo(index);
        }

    protected:
        bool loadImpl(const QString &path) override { return mock_loadImpl(path); }
        void resetImpl() override { mock_resetImpl(); }
        bool updateImpl() override { return mock_updateImpl(); }
    };

    // NOTE: 長いのでエイリアスを作る
    using ImageEdit = QScopedPointer<BasicImageEditInterface>;

private slots:
    void init(); // will be called before each test function is executed.

    // Test cases:
    void test_constructor();
    void test_load();
    void test_reset();
    void test_update();
};

void TestBasicImageEditInterface::init()
{
    mock_save.clearFunction();
    mock_overwriteSave.clearFunction();
    mock_fileInfo.clearFunction();
    mock_loadImpl.clearFunction();
    mock_resetImpl.clearFunction();
    mock_updateImpl.clearFunction();
}

void TestBasicImageEditInterface::test_constructor()
{
    const ImageEdit imageEdit(new BasicImageEditInterfaceMock);

    // outdatedの初期値がfalseであること
    QVERIFY(imageEdit->isOutdated() == false);

    // 画像が空であること
    QVERIFY(imageEdit->current().isNull());
}

void TestBasicImageEditInterface::test_load()
{
    const ImageEdit imageEdit(new BasicImageEditInterfaceMock);

    imageEdit->load("");
    // loadImplが呼ばれていること
    QVERIFY(mock_loadImpl.count() == 1);

    // outdatedがtrueになっていること
    QVERIFY(imageEdit->isOutdated() == true);

    const auto &returnsTrue = [](const QString &) { return true; };
    const auto &returnsFalse = [](const QString &) { return false; };

    // loadImplの返り値とload()の返り値が一致していること
    mock_loadImpl.setFunction(returnsTrue);
    QVERIFY(imageEdit->load("") == true);
    mock_loadImpl.setFunction(returnsFalse);
    QVERIFY(imageEdit->load("") == false);
}

void TestBasicImageEditInterface::test_reset()
{
    const ImageEdit imageEdit(new BasicImageEditInterfaceMock);

    imageEdit->reset();

    // outdatedがfalseになっていること
    QVERIFY(imageEdit->isOutdated() == false);
    // resetImplが呼ばれていること
    QVERIFY(mock_resetImpl.count() == 1);
    // updateImplが呼ばれていること
    QVERIFY(mock_updateImpl.count() == 1);

    const auto &returnsTrue = []() { return true; };
    const auto &returnsFalse = []() { return false; };

    // updateResultの値がupdateImplの返り値と一致していること
    mock_updateImpl.setFunction(returnsTrue);
    imageEdit->reset();
    QVERIFY(imageEdit->updateResult == true);
    mock_updateImpl.setFunction(returnsFalse);
    imageEdit->reset();
    QVERIFY(imageEdit->updateResult == false);
}

void TestBasicImageEditInterface::test_update()
{
    const ImageEdit imageEdit(new BasicImageEditInterfaceMock);
    imageEdit->outdated = true;

    imageEdit->update();

    // outdatedがfalseになっていること
    QVERIFY(imageEdit->isOutdated() == false);

    // updateImplが呼ばれていること
    QVERIFY(mock_updateImpl.count() == 1);

    // outdatedがfalseの場合はupdateImplが呼ばれないこと
    imageEdit->update();
    QVERIFY(mock_updateImpl.count() == 1);

    const auto &returnsTrue = []() { return true; };
    const auto &returnsFalse = []() { return false; };

    // 返り値がupdateImplの返り値と一致していること
    imageEdit->outdated = true;
    mock_updateImpl.setFunction(returnsTrue);
    QVERIFY(imageEdit->update() == true);
    // もう一度updateを呼ぶと同じ結果を返すこと
    QVERIFY(imageEdit->update() == true);

    imageEdit->outdated = true;
    mock_updateImpl.setFunction(returnsFalse);
    QVERIFY(imageEdit->update() == false);
    // もう一度updateを呼ぶと同じ結果を返すこと
    QVERIFY(imageEdit->update() == false);
}

} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
QTEST_APPLESS_MAIN(Test::TestBasicImageEditInterface)

#include "test_basic_image_edit_interface.moc"

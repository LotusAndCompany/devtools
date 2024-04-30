#include <QtTest>

#define _TEST_Tool
#include "core/tool/tool.h"
#undef _TEST_Tool

#include "core/exception/invalid_argument_exception.h"

namespace Test {
class TestTool : public QObject
{
    Q_OBJECT

    using exception_type = InvalidArgumentException<int>;

private slots:
    // Test cases:
    void test_trasnlatable();
    void test_validateID();
};

void TestTool::test_trasnlatable()
{
    // Tool::ID::MINが失敗すること
    QVERIFY_THROWS_EXCEPTION(exception_type, Tool::validateID(Tool::ID::MIN));

    // Tool::ID::IMAGE_RESIZEが成功すること
    QVERIFY_THROWS_NO_EXCEPTION(Tool::validateID(Tool::ID::IMAGE_RESIZE));

    // Tool::ID::IMAGE_ROTATIONが成功すること
    QVERIFY_THROWS_NO_EXCEPTION(Tool::validateID(Tool::ID::IMAGE_ROTATION));

    // Tool::ID::MAXが失敗すること
    QVERIFY_THROWS_EXCEPTION(exception_type, Tool::validateID(Tool::ID::MAX));
}

void TestTool::test_validateID()
{
    // Tool::ID::MINが失敗すること
    QVERIFY_THROWS_EXCEPTION(exception_type, Tool::translatable(Tool::ID::MIN));

    try {
        const auto imageResize = Tool::translatable(Tool::ID::IMAGE_RESIZE);

        // Translatableの内容が正しいこと
        QCOMPARE_EQ(imageResize.name, tr("Image Resize"));
        QCOMPARE_EQ(imageResize.description, tr("Image resizing and scaling"));

        const auto imageRotation = Tool::translatable(Tool::ID::IMAGE_ROTATION);

        // Translatableの内容が正しいこと
        QCOMPARE_EQ(imageRotation.name, tr("Image Rotation"));
        QCOMPARE_EQ(imageRotation.description, tr("Image rotation and flipping"));
    } catch (CommonException &e) {
        // InvalidArgumentException<int>, UnderDevelopmentExceptionが発生する可能性がある
        QFAIL("Tool::translatable(Tool::ID::IMAGE_RESIZE) failed unexpectedly");
    }

    // Tool::ID::MAXが失敗すること
    QVERIFY_THROWS_EXCEPTION(exception_type, Tool::translatable(Tool::ID::MAX));
}
} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
QTEST_APPLESS_MAIN(Test::TestTool)

#include "test_tool.moc"

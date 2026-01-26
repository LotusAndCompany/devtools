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
    static void test_validateID();
};

void TestTool::test_validateID()
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
} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
QTEST_APPLESS_MAIN(Test::TestTool)

#include "test_tool.moc"

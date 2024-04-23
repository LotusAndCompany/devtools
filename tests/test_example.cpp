#include <QtTest>
#include "tests/test_util.h"

#define _TEST_Example
// necessary includes here
#include "test_example_external.h"

/*
 * 参考: https://doc.qt.io/qt-6/qtest-overview.html
 *      https://doc.qt.io/qt-6/qtest.html
 */

// NOTE: 非公開関数のテストを行う場合はTest::TestExampleをfriend classにする必要がある
// マクロにできるかも?
//#include "example.h"

// example.h内
/*
#ifdef _TEST_Example
namespace Test {
class TestExample;
}
#endif
*/

// class Example内
/*
#ifdef _TEST_Example
    friend class Test::TestExample;
#endif
*/
#undef _TEST_Example

namespace Test {
class TestExample : public QObject
{
    Q_OBJECT

public:
    TestExample() {}
    ~TestExample() {}

private slots:
    void initTestCase() {}      // will be called before the first test function is executed.
    void initTestCase_data() {} // will be called to create a global test data table.
    void cleanupTestCase() {}   // will be called after the last test function was executed.
    void init() {}              // will be called before each test function is executed.
    void cleanup() {}           // will be called after every test function.

    // Test cases:
    void test_case1();
};

void TestExample::test_case1()
{
    f();
}
} // namespace Test

// QCoreApplicationもQApplicationも不要な場合
QTEST_APPLESS_MAIN(Test::TestExample)

// QCoreApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
//QTEST_GUILESS_MAIN(Test::TestExample)

// QApplicationもしくはQGuiApplicationが必要な場合
// WARNING: gui/gui_application.hの機能は使えない
//QTEST_MAIN(Test::TestExample)

#include "test_example.moc"

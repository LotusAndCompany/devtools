#include <QtTest>

// necessary includes here

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

void TestExample::test_case1() {}
} // namespace Test

QTEST_APPLESS_MAIN(Test::TestExample)

#include "test_example.moc"

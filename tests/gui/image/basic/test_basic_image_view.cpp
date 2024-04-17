#include <QtTest>

#define _TEST_BasicImageView
#include "gui/image/basic/image_view.h"
#undef _TEST_BasicImageView

namespace Test {
class TestBasicImageView : public QObject
{
    Q_OBJECT

public:
    TestBasicImageView() {}
    ~TestBasicImageView() {}

private slots:
    void initTestCase() {}      // will be called before the first test function is executed.
    void initTestCase_data() {} // will be called to create a global test data table.
    void cleanupTestCase() {}   // will be called after the last test function was executed.
    void init() {}              // will be called before each test function is executed.
    void cleanup() {}           // will be called after every test function.

    // Test cases:
};

} // namespace Test

QTEST_MAIN(Test::TestBasicImageView)

#include "test_basic_image_view.moc"

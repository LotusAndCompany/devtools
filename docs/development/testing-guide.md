# Testing Guide

This guide explains how to write and run tests for DevTools.

## Overview

DevTools uses Google Test (gtest) for unit testing. Tests are organized by module and cover both core logic and GUI components.

## Test Structure

```
tests/
├── DevToolsTests.cmake       # Test configuration
├── core/                     # Core logic tests
│   ├── data_conversion/
│   ├── exception/
│   └── image/
├── gui/                      # GUI tests
└── helpers/                  # Test utilities
    ├── mock_helper.h
    └── random_data.h
```

## Enabling Tests

Tests are disabled by default. Enable them with CMake:

```bash
# Configure with tests enabled
cmake .. -DENABLE_UNIT_TEST=ON

# Build
make

# Run all tests
ctest

# Run with verbose output
ctest --output-on-failure
```

## Writing Tests

### Basic Test Structure

```cpp
#include <gtest/gtest.h>
#include "core/module/class.h"

namespace devtools {

// Test fixture for shared setup
class MyClassTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup before each test
        instance_ = std::make_unique<MyClass>();
    }
    
    void TearDown() override {
        // Cleanup after each test
    }
    
    std::unique_ptr<MyClass> instance_;
};

// Test case using fixture
TEST_F(MyClassTest, MethodReturnsExpectedValue) {
    EXPECT_EQ(instance_->method(), expected_value);
}

// Standalone test (no fixture)
TEST(MyClassSimpleTest, BasicFunctionality) {
    MyClass obj;
    ASSERT_TRUE(obj.isValid());
}

}  // namespace devtools
```

### Test Macros

| Macro | Description |
|-------|-------------|
| `EXPECT_EQ(a, b)` | Expect equal (continue on failure) |
| `EXPECT_NE(a, b)` | Expect not equal |
| `EXPECT_TRUE(x)` | Expect true |
| `EXPECT_FALSE(x)` | Expect false |
| `EXPECT_THROW(stmt, exception)` | Expect exception |
| `ASSERT_*` | Same as EXPECT but stops on failure |

### Testing Exceptions

```cpp
#include "core/exception/invalid_argument_exception.h"

TEST(ValidationTest, ThrowsOnInvalidInput) {
    MyClass obj;
    EXPECT_THROW(obj.process(-1), InvalidArgumentException);
}

TEST(ValidationTest, DoesNotThrowOnValidInput) {
    MyClass obj;
    EXPECT_NO_THROW(obj.process(1));
}
```

### Testing with Qt Types

```cpp
#include <QString>
#include <QImage>

TEST(ImageTest, LoadsValidImage) {
    QImage image("test_image.png");
    ASSERT_FALSE(image.isNull());
    EXPECT_EQ(image.width(), 100);
    EXPECT_EQ(image.height(), 100);
}

TEST(StringTest, ProcessesQString) {
    QString input = "test";
    QString result = processString(input);
    EXPECT_EQ(result.toStdString(), "TEST");
}
```

## Test Categories

### Core Logic Tests

Test business logic without GUI dependencies:

```cpp
// tests/core/data_conversion/test_json_parser.cpp
#include <gtest/gtest.h>
#include "core/data_conversion/parser/json_parser.h"

namespace devtools {

TEST(JsonParserTest, ParsesValidJson) {
    JsonParser parser;
    auto result = parser.parse("{\"key\": \"value\"}");
    ASSERT_TRUE(result.has_value());
}

TEST(JsonParserTest, FailsOnInvalidJson) {
    JsonParser parser;
    auto result = parser.parse("not json");
    ASSERT_FALSE(result.has_value());
}

}  // namespace devtools
```

### GUI Tests

Test GUI components (limited without display):

```cpp
// tests/gui/test_main_window.cpp
#include <gtest/gtest.h>
#include <QApplication>
#include "gui/main_window.h"

namespace devtools {

class MainWindowTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        // QApplication needed for widget tests
        int argc = 0;
        app_ = new QApplication(argc, nullptr);
    }
    
    static void TearDownTestSuite() {
        delete app_;
    }
    
    static QApplication* app_;
};

QApplication* MainWindowTest::app_ = nullptr;

TEST_F(MainWindowTest, CanBeCreated) {
    MainWindow window;
    EXPECT_NE(window.windowTitle(), QString());
}

}  // namespace devtools
```

## Registering Tests

Add tests to `tests/DevToolsTests.cmake`:

```cmake
# Macro definition
macro(DevTools_add_test test_name)
    set(options)
    set(one_value_args)
    set(multi_value_args SOURCES LIBRARIES)
    cmake_parse_arguments(TEST "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})
    
    add_executable(${test_name} ${TEST_SOURCES})
    target_link_libraries(${test_name} PRIVATE
        GTest::gtest
        GTest::gtest_main
        ${MODULE_LIST}
        ${TEST_LIBRARIES}
    )
    add_test(NAME ${test_name} COMMAND ${test_name})
endmacro()

# Register your test
DevTools_add_test(test_your_module
    SOURCES
    tests/core/your_module/test_your_module.cpp
)
```

## Test Helpers

### Mock Helper

```cpp
// tests/helpers/mock_helper.h
#include <QString>

namespace devtools::test {

QString createTempFile(const QString& content);
void deleteTempFile(const QString& path);

}  // namespace devtools::test
```

### Random Data Generator

```cpp
// tests/helpers/random_data.h
#include <QString>

namespace devtools::test {

QString randomString(int length);
QByteArray randomBytes(int size);

}  // namespace devtools::test
```

Usage:

```cpp
#include "tests/helpers/random_data.h"

TEST(ProcessorTest, HandlesRandomInput) {
    QString input = test::randomString(100);
    MyProcessor processor;
    EXPECT_NO_THROW(processor.process(input));
}
```

## Running Tests

### Run All Tests

```bash
cd build
ctest
```

### Run Specific Test

```bash
# By test name
ctest -R test_json_parser

# Run executable directly for more control
./test_json_parser
```

### Run with Filters

```bash
# Run tests matching pattern
./test_your_module --gtest_filter="*Parser*"

# Run single test
./test_your_module --gtest_filter="JsonParserTest.ParsesValidJson"
```

### Verbose Output

```bash
# CMake verbose
ctest --output-on-failure

# GTest verbose
./test_your_module --gtest_output=xml:results.xml
```

## Best Practices

### Test Naming

```cpp
// Pattern: TestSuite, DescriptiveTestName
TEST(ImageResizer, ScalesDownLargeImages)
TEST(ImageResizer, PreservesAspectRatioWhenRequested)
TEST(ImageResizer, ThrowsOnNegativeDimensions)
```

### Test Organization

1. **One assertion per test** when possible
2. **Test one behavior** per test function
3. **Use fixtures** for shared setup
4. **Keep tests independent** - no order dependencies

### What to Test

- **Public interfaces**: All public methods
- **Edge cases**: Empty input, max values, null
- **Error conditions**: Invalid input, exceptions
- **Happy paths**: Normal operation

### What Not to Test

- **Private methods**: Test through public interface
- **Third-party code**: Qt, yaml-cpp, etc.
- **Trivial getters/setters**: Unless they have logic

## Continuous Integration

Tests run automatically on:

- Pull request creation
- Push to develop/main branches

GitHub Actions workflow checks:
1. Build succeeds
2. All tests pass
3. Code formatting is correct

## Troubleshooting

### Tests Not Found

1. Check test is registered in DevToolsTests.cmake
2. Rebuild: `make clean && make`
3. Verify ENABLE_UNIT_TEST=ON

### Qt Tests Crash

1. Ensure QApplication is created
2. Use test fixture with SetUpTestSuite
3. Check for missing Qt plugins

### Flaky Tests

1. Remove test order dependencies
2. Reset state in SetUp/TearDown
3. Avoid timing-dependent tests

## Related Documentation

- [Architecture](architecture.md) - System architecture
- [Coding Standards](coding-standards.md) - Code style
- [Adding New Tools](adding-new-tools.md) - Module creation

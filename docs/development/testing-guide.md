# Testing Guide

This guide explains how to write and run tests for DevTools.

## Overview

DevTools uses Qt Test. Tests are organized by feature and cover core services,
processing, and GUI behavior. Test cases are methods on a `QObject` test class;
`QTEST_GUILESS_MAIN` starts a `QCoreApplication` for tests without widgets, and
`QTEST_MAIN` starts a `QApplication` for widget tests.

## Test Structure

Test sources live alongside each feature under `features/{feature}/tests/`.
Shared test helpers and test data live under the top-level `tests/` directory.

```mermaid
flowchart TD
    tests["tests/"]
    tests --> registration["DevToolsTests.cmake<br/>Test registration"]
    tests --> helpers["test_util.h/.cpp<br/>random_data.h/.cpp<br/>mock_helper.h"]
    tests --> fixtures["core/<br/>Test fixtures"]
    fixtures --> conversion["data_conversion/"]
    fixtures --> image["image/"]
    feature["features/{feature}/"] --> feature_tests["tests/<br/>Per-feature test sources"]
    feature_tests --> test_file["test_*.cpp"]
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
#include "features/your_module/core/your_class.h"

#include <QtTest>

class TestMyClass : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void methodReturnsExpectedValue();
};

void TestMyClass::init()
{
    // Set up state before each test method.
}

void TestMyClass::cleanup()
{
    // Release state after each test method when needed.
}

void TestMyClass::methodReturnsExpectedValue()
{
    MyClass object;
    QCOMPARE(object.method(), expected_value);
}

QTEST_GUILESS_MAIN(TestMyClass)
#include "test_my_class.moc"
```

### UI Consistency Tests

Shared UI behavior is tested in
`features/framework/tests/test_design_system.cpp`. When a design-system helper
changes, test the observable contract rather than implementation details: size
policies, margins, spacing roles, frame configuration, item-view selection,
table headers, splitter configuration, and dialog-footer alignment are suitable
examples.

GUI tests should cover both the default widget state and the state that matters
to the user, such as disabled controls, empty results, validation errors, and
read-only output. Keep feature-specific workflow tests in that feature's test
directory and keep reusable widget rules in the framework test.

For a UI-wide change, perform a manual smoke pass with the application running:

1. Open every registered tool from the side menu.
2. Check the default, empty, error, and result states relevant to each tool.
3. Resize the main window through the supported range and check splitter and
   scroll behavior.
4. Verify both light and dark themes and both English and Japanese text.
5. Open Settings, About, and DB connection windows and confirm the same pane,
   field, action-bar, and footer rules.

Record any visual regression with the affected screen and state before changing
the shared helper or adding a screen-specific exception.

### Test Macros

| Qt Test macro | Description |
|---------------|-------------|
| `QCOMPARE(actual, expected)` | Compare values and report both values on failure |
| `QVERIFY(condition)` | Check a condition; stop the current test method on failure |
| `QVERIFY2(condition, message)` | Check a condition with a custom failure message |
| `QVERIFY_EXCEPTION_THROWN(expression, Exception)` | Check that an expression throws the given exception |
| `QSKIP(message)` | Skip the current test method |

### Testing Exceptions

```cpp
#include "features/framework/core/exception/invalid_argument_exception.h"

#include <QtTest>

class TestValidation : public QObject
{
    Q_OBJECT

private slots:
    void rejectsInvalidInput();
};

void TestValidation::rejectsInvalidInput()
{
    MyClass object;
    QVERIFY_EXCEPTION_THROWN(object.process(-1), InvalidArgumentException);
}

QTEST_GUILESS_MAIN(TestValidation)
#include "test_validation.moc"
```

### Testing with Qt Types

```cpp
#include <QString>
#include <QImage>
#include <QtTest>

class TestImage : public QObject
{
    Q_OBJECT

private slots:
    void loadsValidImage();
};

void TestImage::loadsValidImage()
{
    const QImage image(100, 50, QImage::Format_ARGB32);
    QVERIFY(!image.isNull());
    QCOMPARE(image.width(), 100);
    QCOMPARE(image.height(), 50);
}

QTEST_GUILESS_MAIN(TestImage)
#include "test_image.moc"
```

## Test Categories

### Core Logic Tests

Test business logic without GUI dependencies:

```cpp
// features/data_conversion/tests/test_json_parser.cpp
#include "features/data_conversion/core/parser/json_parser.h"

#include <QtTest>

class TestJsonParser : public QObject
{
    Q_OBJECT

private slots:
    void parsesValidJson();
    void rejectsInvalidJson();
};

void TestJsonParser::parsesValidJson()
{
    JsonParser parser;
    auto result = parser.parse("{\"key\": \"value\"}");
    QVERIFY(result.has_value());
}

void TestJsonParser::rejectsInvalidJson()
{
    JsonParser parser;
    auto result = parser.parse("not json");
    QVERIFY(!result.has_value());
}

QTEST_GUILESS_MAIN(TestJsonParser)
#include "test_json_parser.moc"
```

### GUI Tests

Test GUI components (limited without display):

```cpp
// features/framework/tests/test_main_window.cpp
#include "features/framework/gui/main_window.h"

#include <QtTest>

class TestMainWindow : public QObject
{
    Q_OBJECT

private slots:
    void canBeCreated();
};

void TestMainWindow::canBeCreated()
{
    MainWindow window;
    QVERIFY(!window.windowTitle().isEmpty());
}

QTEST_MAIN(TestMainWindow)
#include "test_main_window.moc"
```

Set `QT_QPA_PLATFORM=offscreen` when running widget tests in an environment
without a display.

## Registering Tests

Add tests to `tests/DevToolsTests.cmake`:

```cmake
# A test that uses feature libraries and shared test helpers.
DevTools_add_test(test_your_module
    SOURCES
    features/your_module/tests/test_your_module.cpp
)

# A service test compiled without the shared GUI/test library.
DevTools_add_test(test_your_service CORE_ONLY
    SOURCES
    features/your_module/tests/test_your_service.cpp
    features/your_module/core/your_service.cpp
)

# Add only the Qt module required by the service implementation, if needed.
target_link_libraries(test_your_service PRIVATE Qt6::Network)
```

`DevTools_add_test` is defined in `tests/DevToolsTests.cmake`. The default
path links the shared test library and application feature modules. Use
`CORE_ONLY` for a test that compiles service source files directly without that
library; the helper then links only Qt Core and Qt Test. Add any other required
Qt module, such as Qt Gui, Network, or Sql, with `target_link_libraries()` after
the registration. Include the service implementation source in `SOURCES`.

## Test Helpers

### Mock Helper

`tests/mock_helper.h` provides `Test::FunctionMock<Signature>`. It can replace
a function's behavior and record its call count and arguments:

```cpp
#include "tests/mock_helper.h"

#include <QtTest>

Test::FunctionMock<int(int)> mock([](int value) { return value + 1; });
QCOMPARE(mock(2), 3);
QCOMPARE(mock.count(), size_t(1));
```

### Random Data Generator

`tests/random_data.h` provides `Test::RandomData`. Pass a seed when repeatable
input is useful:

```cpp
#include "tests/random_data.h"

#include <QtTest>

class TestProcessor : public QObject
{
    Q_OBJECT

private slots:
    void handlesRandomInput();
};

void TestProcessor::handlesRandomInput()
{
    Test::RandomData random(123);
    const QString input = random.nextQString(100);
    QCOMPARE(input.size(), 100);
}

QTEST_GUILESS_MAIN(TestProcessor)
#include "test_processor.moc"
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

### Run Selected Test Methods

```bash
# List test methods in a Qt Test executable
./test_your_module -functions

# Run one test method
./test_your_module parsesValidJson
```

### Verbose Output

```bash
# CMake verbose
ctest --output-on-failure

# Qt Test verbose output
./test_your_module -v1

# Write a JUnit XML report
./test_your_module -o results.xml,junitxml
```

## Best Practices

### Test Naming

```cpp
// Use a Test-prefixed QObject class and descriptive test method names.
class TestImageResizer : public QObject
{
    Q_OBJECT

private slots:
    void scalesDownLargeImages();
    void preservesAspectRatioWhenRequested();
    void rejectsNegativeDimensions();
};
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

1. Use `QTEST_MAIN` for widget tests and `QTEST_GUILESS_MAIN` for tests without widgets
2. Set `QT_QPA_PLATFORM=offscreen` when no display is available
3. Check for missing Qt plugins

### Flaky Tests

1. Remove test order dependencies
2. Reset state in SetUp/TearDown
3. Avoid timing-dependent tests

## Related Documentation

- [Architecture](architecture.md) - System architecture
- [Coding Standards](coding-standards.md) - Code style
- [Adding New Tools](adding-new-tools.md) - Module creation

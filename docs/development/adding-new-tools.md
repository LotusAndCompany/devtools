# Adding New Tools

This guide explains how to add new tool modules to DevTools.

## Overview

DevTools organizes each feature under `features/{your_tool}/` with `core/`,
`gui/`, and `tests/` subdirectories. The application framework shared by all
tools lives under `features/framework/` and is compiled as `DevTools_core`.

Adding a new tool involves:

1. Creating the feature directory structure under `features/{your_tool}/`
2. Implementing core logic (no Qt Widgets dependencies)
3. Creating GUI components (Qt Widgets)
4. Registering the module in CMake (target declaration + `target_sources` + `add_subdirectory`)
5. Adding a tool ID in the framework
6. Registering in the side menu and contents area
7. Adding translations
8. Writing tests in `features/{your_tool}/tests/`

## Step-by-Step Guide

### 1. Plan Your Tool

Before coding, determine:

- **Purpose**: What does the tool do?
- **Core Logic**: What algorithms or processing is needed?
- **User Interface**: What inputs and outputs does the user need?
- **Dependencies**: Does it need external libraries?

### 2. Create Directory Structure

Create the feature directory with `core/`, `gui/`, and `tests/` subdirectories
(`CMakeLists.txt` is required; `tests/` can be added later):

```bash
mkdir -p features/your_tool/core
mkdir -p features/your_tool/gui
mkdir -p features/your_tool/tests
touch features/your_tool/CMakeLists.txt
```

### 3. Implement Core Logic

Create the business logic without any Qt GUI dependencies.

#### features/your_tool/core/your_tool.h

```cpp
#ifndef DEVTOOLS_FEATURES_YOUR_TOOL_CORE_YOUR_TOOL_H
#define DEVTOOLS_FEATURES_YOUR_TOOL_CORE_YOUR_TOOL_H

#include <QString>

namespace devtools {

class YourTool {
public:
    YourTool();
    
    /**
     * @brief Process input and return result
     * @param input The input to process
     * @return Processed result
     */
    QString process(const QString& input);

private:
    // Private implementation details
};

}  // namespace devtools

#endif  // DEVTOOLS_FEATURES_YOUR_TOOL_CORE_YOUR_TOOL_H
```

#### features/your_tool/core/your_tool.cpp

```cpp
#include "features/your_tool/core/your_tool.h"

namespace devtools {

YourTool::YourTool() = default;

QString YourTool::process(const QString& input) {
    // Implement your logic here
    return input.toUpper();  // Example
}

}  // namespace devtools
```

### 4. Create GUI Components

#### features/your_tool/gui/your_tool_gui.h

```cpp
#ifndef DEVTOOLS_FEATURES_YOUR_TOOL_GUI_YOUR_TOOL_GUI_H
#define DEVTOOLS_FEATURES_YOUR_TOOL_GUI_YOUR_TOOL_GUI_H

#include "features/framework/gui/gui_tool.h"
#include "features/your_tool/core/your_tool.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

namespace devtools {

class YourToolGui : public GuiTool {
    Q_OBJECT

public:
    explicit YourToolGui(QWidget* parent = nullptr);
    ~YourToolGui() override;

private slots:
    void onProcessClicked();

private:
    QLineEdit* inputEdit_;
    QPushButton* processButton_;
    QLabel* outputLabel_;
    YourTool tool_;
};

}  // namespace devtools

#endif  // DEVTOOLS_FEATURES_YOUR_TOOL_GUI_YOUR_TOOL_GUI_H
```

#### features/your_tool/gui/your_tool_gui.cpp

```cpp
#include "features/your_tool/gui/your_tool_gui.h"

#include <QVBoxLayout>

namespace devtools {

YourToolGui::YourToolGui(QWidget* parent)
    : GuiTool(parent)
    , inputEdit_(new QLineEdit(this))
    , processButton_(new QPushButton(tr("Process"), this))
    , outputLabel_(new QLabel(this)) {
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(inputEdit_);
    layout->addWidget(processButton_);
    layout->addWidget(outputLabel_);

    inputEdit_->setPlaceholderText(tr("Enter input..."));
    
    connect(processButton_, &QPushButton::clicked,
            this, &YourToolGui::onProcessClicked);
}

YourToolGui::~YourToolGui() = default;

void YourToolGui::onProcessClicked() {
    QString input = inputEdit_->text();
    QString result = tool_.process(input);
    outputLabel_->setText(result);
}

}  // namespace devtools
```

### 5. Register in CMake

The tool's CMake registration involves three files: the root `CMakeLists.txt`
(target declaration and `MODULE_LIST`), `features/your_tool/CMakeLists.txt`
(source file list via `target_sources()`), and `features/CMakeLists.txt`
(`add_subdirectory()`).

#### 5a. Declare the target in the root `CMakeLists.txt`

```cmake
# Add after other module definitions (next to the existing qt_add_library calls)
qt_add_library(${PROJECT_NAME}_your_tool STATIC)
```

#### 5b. List sources in `features/your_tool/CMakeLists.txt`

```cmake
# features/your_tool/CMakeLists.txt
target_sources(${PROJECT_NAME}_your_tool PRIVATE
    core/your_tool.h
    core/your_tool.cpp
    gui/your_tool_gui.h
    gui/your_tool_gui.cpp
)
```

#### 5c. Add to `MODULE_LIST` in the root `CMakeLists.txt`

```cmake
set(MODULE_LIST
    # ... existing modules
    ${PROJECT_NAME}_your_tool
)
```

#### 5d. Register the feature in `features/CMakeLists.txt`

```cmake
# features/CMakeLists.txt
add_subdirectory(your_tool)
```

If your module has external dependencies:

```cmake
# Link external libraries
target_link_libraries(${PROJECT_NAME}_your_tool PUBLIC
    external_library::external_library
)
```

### 6. Add Tool ID

Add a new tool ID in `features/framework/core/tool/tool_id_fields.h`:

```cpp
#define TOOL_ID_FIELDS() \
    ... \
    YOUR_TOOL_ID
```

`Tool::ID` and `Sidemenu::ID` expand the same macro, keeping the two IDs
aligned.

### 7. Register in Side Menu

Register the tool in `Sidemenu::Sidemenu()`:

```cpp
registerItem(ID::YOUR_TOOL_ID);
```

Add its icon names in `Sidemenu::icon()` and its translated name and
description in `Tool::translatable()`.

### 8. Register in Contents Area

Include your GUI and add it to `ContentsArea::changeContent`:

```cpp
#include "features/your_tool/gui/your_tool_gui.h"

case Sidemenu::ID::YOUR_TOOL_ID:
    content = new YourToolGui(this);
    break;
```

### 9. Add Translations

#### Mark Strings for Translation

```cpp
// Use tr() for user-visible strings
processButton_->setText(tr("Process"));
```

#### Update Translation Files

```bash
# Update .ts files when translatable strings changed
cmake --build build --target update_devtools_translations
```

### 10. Add an Icon

Prefer an existing `QIcon::fromTheme` name. If a new asset is needed, add
matching SVGs under the light and dark icon resource directories, list them in
`res/light_icons.qrc` and `res/dark_icons.qrc`, and add the fallback names in
`Sidemenu::icon()`.

```
res/light/material/your_tool.svg
res/dark/material/your_tool.svg
```

### 11. Write Tests

Create test files under `features/your_tool/tests/`:

#### features/your_tool/tests/test_your_tool.cpp

```cpp
#include <gtest/gtest.h>
#include "features/your_tool/core/your_tool.h"

namespace devtools {

TEST(YourToolTest, ProcessConvertsToUppercase) {
    YourTool tool;
    EXPECT_EQ(tool.process("hello"), "HELLO");
}

TEST(YourToolTest, ProcessHandlesEmptyInput) {
    YourTool tool;
    EXPECT_EQ(tool.process(""), "");
}

}  // namespace devtools
```

#### Register Tests in CMake

Add to `tests/DevToolsTests.cmake`:

```cmake
DevTools_add_test(test_your_tool
    SOURCES
    features/your_tool/tests/test_your_tool.cpp
)
```

### 12. Build and Test

```bash
# Build with tests enabled
cmake .. -DENABLE_UNIT_TEST=ON
make

# Run tests
ctest --output-on-failure

# Run specific test
./test_your_tool

# Build and launch the app to verify
cmake --build . --target run
```

## Checklist

- [ ] Core logic implemented in `features/your_tool/core/`
- [ ] GUI implemented in `features/your_tool/gui/`
- [ ] `qt_add_library` declaration added to root `CMakeLists.txt`
- [ ] Sources listed in `features/your_tool/CMakeLists.txt` via `target_sources()`
- [ ] `add_subdirectory(your_tool)` added to `features/CMakeLists.txt`
- [ ] Module added to `MODULE_LIST` in root `CMakeLists.txt`
- [ ] Tool ID added to `features/framework/core/tool/tool_id_fields.h`
- [ ] Registered in `features/framework/gui/sidemenu.cpp`
- [ ] Registered in `features/framework/gui/contents_area.cpp`
- [ ] Strings marked for translation
- [ ] Translations added
- [ ] Icons added
- [ ] Unit tests written in `features/your_tool/tests/`
- [ ] Test registered in `tests/DevToolsTests.cmake`
- [ ] Documentation updated

## Example: Complete Module

For a complete example, examine an existing module under `features/`:

- **Simple**: `features/phrase_generation/` - Minimal dependencies
- **Medium**: `features/data_conversion/` - External library (yaml-cpp, toml11)
- **Complex**: `features/image/` - Unified tools depending on `DevTools_image_core`

## Tips

1. **Start Simple**: Get basic functionality working first
2. **Test Early**: Write tests as you develop
3. **Follow Patterns**: Look at existing modules for guidance
4. **Keep Core Separate**: GUI should not be in core/
5. **Document APIs**: Use Doxygen comments

## Related Documentation

- [Architecture](architecture.md) - System architecture
- [Coding Standards](coding-standards.md) - Code style
- [Testing Guide](testing-guide.md) - Writing tests
- [Localization](localization.md) - Translation guide

# Adding New Tools

This guide explains how to add new tool modules to DevTools.

## Overview

Adding a new tool involves:

1. Creating core logic (business logic)
2. Creating GUI components (user interface)
3. Registering the module in CMake
4. Adding to the side menu
5. Adding translations
6. Writing tests

## Step-by-Step Guide

### 1. Plan Your Tool

Before coding, determine:

- **Purpose**: What does the tool do?
- **Core Logic**: What algorithms or processing is needed?
- **User Interface**: What inputs and outputs does the user need?
- **Dependencies**: Does it need external libraries?

### 2. Create Directory Structure

Create directories for your tool:

```bash
# Core logic
mkdir -p core/your_tool

# GUI components  
mkdir -p gui/your_tool
```

### 3. Implement Core Logic

Create the business logic without any Qt GUI dependencies.

#### core/your_tool/your_tool.h

```cpp
#ifndef DEVTOOLS_CORE_YOUR_TOOL_YOUR_TOOL_H
#define DEVTOOLS_CORE_YOUR_TOOL_YOUR_TOOL_H

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

#endif  // DEVTOOLS_CORE_YOUR_TOOL_YOUR_TOOL_H
```

#### core/your_tool/your_tool.cpp

```cpp
#include "core/your_tool/your_tool.h"

namespace devtools {

YourTool::YourTool() = default;

QString YourTool::process(const QString& input) {
    // Implement your logic here
    return input.toUpper();  // Example
}

}  // namespace devtools
```

### 4. Create GUI Components

#### gui/your_tool/your_tool_gui.h

```cpp
#ifndef DEVTOOLS_GUI_YOUR_TOOL_YOUR_TOOL_GUI_H
#define DEVTOOLS_GUI_YOUR_TOOL_YOUR_TOOL_GUI_H

#include "gui/gui_tool.h"
#include "core/your_tool/your_tool.h"

#include <memory>

namespace Ui {
class YourToolGui;
}

namespace devtools {

class YourToolGui : public GuiTool {
    Q_OBJECT

public:
    explicit YourToolGui(QWidget* parent = nullptr);
    ~YourToolGui() override;

private slots:
    void onProcessClicked();

private:
    std::unique_ptr<Ui::YourToolGui> ui_;
    YourTool tool_;
};

}  // namespace devtools

#endif  // DEVTOOLS_GUI_YOUR_TOOL_YOUR_TOOL_GUI_H
```

#### gui/your_tool/your_tool_gui.cpp

```cpp
#include "gui/your_tool/your_tool_gui.h"
#include "ui_your_tool_gui.h"

namespace devtools {

YourToolGui::YourToolGui(QWidget* parent)
    : GuiTool(parent)
    , ui_(std::make_unique<Ui::YourToolGui>()) {
    ui_->setupUi(this);
    
    connect(ui_->processButton, &QPushButton::clicked,
            this, &YourToolGui::onProcessClicked);
}

YourToolGui::~YourToolGui() = default;

void YourToolGui::onProcessClicked() {
    QString input = ui_->inputEdit->text();
    QString result = tool_.process(input);
    ui_->outputLabel->setText(result);
}

}  // namespace devtools
```

#### gui/your_tool/your_tool_gui.ui

Create a UI file using Qt Designer or manually:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>YourToolGui</class>
 <widget class="QWidget" name="YourToolGui">
  <layout class="QVBoxLayout">
   <item>
    <widget class="QLineEdit" name="inputEdit">
     <property name="placeholderText">
      <string>Enter input...</string>
     </property>
    </widget>
   </item>
   <item>
    <widget class="QPushButton" name="processButton">
     <property name="text">
      <string>Process</string>
     </property>
    </widget>
   </item>
   <item>
    <widget class="QLabel" name="outputLabel"/>
   </item>
  </layout>
 </widget>
</ui>
```

### 5. Register in CMakeLists.txt

Add your module to the main CMakeLists.txt:

```cmake
# Add after other module definitions

# your_tool
qt_add_library(${PROJECT_NAME}_your_tool STATIC
    core/your_tool/your_tool.h
    core/your_tool/your_tool.cpp
    gui/your_tool/your_tool_gui.h
    gui/your_tool/your_tool_gui.cpp
    gui/your_tool/your_tool_gui.ui
)

# Add to MODULE_LIST
set(MODULE_LIST
    # ... existing modules
    ${PROJECT_NAME}_your_tool
)
```

If your module has external dependencies:

```cmake
# Link external libraries
target_link_libraries(${PROJECT_NAME}_your_tool PUBLIC
    external_library::external_library
)
```

### 6. Add Tool ID

Add a new tool ID in `core/tool/tool_id_fields.h`:

```cpp
enum class ToolId {
    // ... existing IDs
    YourTool,
};
```

### 7. Register in Side Menu

Update `gui/sidemenu.cpp` to include your tool:

```cpp
// In the constructor or initialization
addToolItem(ToolId::YourTool, tr("Your Tool"), QIcon(":/icons/your_tool.png"));
```

### 8. Register in Contents Area

Update `gui/contents_area.cpp` to create your tool widget:

```cpp
#include "gui/your_tool/your_tool_gui.h"

// In the widget creation method
case ToolId::YourTool:
    return new YourToolGui(this);
```

### 9. Add Translations

#### Mark Strings for Translation

```cpp
// Use tr() for user-visible strings
ui_->processButton->setText(tr("Process"));
```

#### Update Translation Files

```bash
# Update .ts files
cmake --build build --target update_translations

# Edit translations using Qt Linguist
linguist res/dev-tools_ja_JP.ts

# Compile .qm files
cmake --build build --target release_translations
```

### 10. Add Icons

Place icons in the resource directories:

```
res/light/your_tool.png
res/dark/your_tool.png
```

Update resource files:

```xml
<!-- res/light_icons.qrc -->
<RCC>
  <qresource prefix="/light">
    <file>your_tool.png</file>
  </qresource>
</RCC>
```

### 11. Write Tests

Create test files:

#### tests/core/your_tool/test_your_tool.cpp

```cpp
#include <gtest/gtest.h>
#include "core/your_tool/your_tool.h"

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
    tests/core/your_tool/test_your_tool.cpp
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
```

## Checklist

- [ ] Core logic implemented in `core/your_tool/`
- [ ] GUI implemented in `gui/your_tool/`
- [ ] Module added to CMakeLists.txt
- [ ] Tool ID added to `tool_id_fields.h`
- [ ] Registered in side menu
- [ ] Registered in contents area
- [ ] Strings marked for translation
- [ ] Translations added
- [ ] Icons added
- [ ] Unit tests written
- [ ] Documentation updated

## Example: Complete Module

For a complete example, examine an existing module:

- **Simple**: `phrase_generation` - Minimal dependencies
- **Medium**: `data_conversion` - External library (yaml-cpp)
- **Complex**: `image_resize` - Depends on image_core

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

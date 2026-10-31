# Adding New Tools

Regular application tools are React screens hosted by Qt WebEngine. Use the
React workflow below to add a tool to the application. The native Qt Widgets
module workflow later in this document applies only when adding a native or
legacy widget screen; registering a C++ menu entry alone does not add a React
tool to the application.

## Overview

The React screen and its feature-specific components live under
`frontend/src/features/{tool}/`. Native operation support remains under
`features/web_frontend/`, and reusable processing lives in the existing C++
feature services under `features/{feature}/core/`. Shared non-Widget framework code lives in
`features/framework/core/` and is compiled as `DevTools_core`. The application
shell and shared widgets mostly live in `features/framework/gui/` and are
compiled as `DevTools_framework_gui`; `ContentsArea` remains in the application
executable. Feature libraries currently list both core and GUI sources in one
target; keep their source dependencies one-way, from GUI to core.

Adding a React tool involves:

1. Creating a feature directory under `frontend/src/features/{tool}/`
2. Implementing the screen and feature-specific components in that directory
3. Adding its route, labels, icon, native ID, and screen to
   `frontend/src/app/tool-registry.ts`
4. Adding a typed bridge operation only when the screen needs native work
5. Adding or extending tests beside the affected feature

## React Tool Workflow

### 1. Add the screen

Create a feature folder with an `index.ts` public entry point, the screen
component, and feature-local styles or components as needed. Keep feature-only
state and UI inside the folder. Put reusable UI in `frontend/src/shared/` only
when more than one feature needs it. Follow the dependency direction and the
Base UI-backed shadcn components described in
[Coding Standards](coding-standards.md).

### 2. Register the tool

Add the screen import and one `ToolDefinition` to
`frontend/src/app/tool-registry.ts`. The registry drives the sidebar, home
screen, screen title, icon, and native tool ID. Set `keepAlive: true` only
when leaving the screen must preserve its mounted state. Do not add a separate
app-level tab or register the React screen in the legacy Qt menu.

### 3. Add native operations when needed

Use the existing typed operation runner from the screen. Add the operation's
input and output to `OperationContracts` in
`frontend/src/shared/bridge/types.ts`, then implement and register the native
handler under `features/web_frontend/`. Register its route in
`features/web_frontend/gui/bridge/operation_registry.cpp` and list any new C++
source in `features/web_frontend/CMakeLists.txt`. Reuse the relevant C++ core service;
keep file paths and native dialogs in C++, and do not call Qt WebChannel
directly from a feature. Update the bridge contract version only when the
wire contract changes. See [System Architecture](architecture.md) and the
existing bridge tests for the current boundary.

### 4. Add tests and verify

Keep React interaction tests next to the feature. Add native bridge or service
tests under the corresponding C++ test directory when native behavior changes.
Run the frontend checks from `frontend/`, then build and run the C++ tests as
described in the [Testing Guide](testing-guide.md).

## Legacy Native Qt Widgets Module Workflow

The steps below describe the existing C++ static-library and Qt Widgets path.
Use them only for native/legacy screens or native feature modules; regular
React tool navigation is managed by the React registry above.

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

Put processing and reusable feature behavior in `core/`. Core code must not
include Qt Widgets, shared GUI framework headers, feature GUI headers, or dialog
objects. It may use other Qt modules and value types required by the processing
it performs. Return values or error codes to the GUI; let the GUI display
localized messages and dialogs.

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

Build the GUI from the shared UI contract before adding screen-specific
composition:

- Start the root layout with `DevTools::Ui::applyPageLayout()`.
- Use `createPane()` for titled content regions, `configureCompactPane()` for
  form/control panes that should size to their contents, and
  `configureToolbarPane()` for compact toolbars.
- Configure every single-line field, combo box, code editor, status view,
  list, and table with the matching `DevTools::Ui` helper immediately after
  construction.
- All text fields use the standard system font through the shared helpers.
  Use `configureCodeEditor()` / `configureCodeLineEdit()` when structured or
  machine-readable content needs code-specific behavior; these helpers do not
  introduce a separate font.
- Use `configureEqualSplitter()`, `configureMainSideSplitter()`, or
  `configureSideMainSplitter()` instead of repeating splitter handle widths
  and stretch factors.
- Use `configureActionBar()` for action placement and
  `configureDialogButtonBox()` for dialog footers.
- Keep all visible strings in `tr()` and provide a `LanguageChange` path when
  the screen has dynamic or re-translatable labels.

Do not add a local stylesheet, hard-coded font, color, spacing, or control size
to compensate for a missing shared role. Extend `features/framework/gui/design_system.*`
and its focused tests when the role is genuinely reusable.

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

protected:
    void changeEvent(QEvent* event) override;

private slots:
    void onProcessClicked();

private:
    void retranslateUi();

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
#include "features/framework/gui/design_system.h"

#include <QVBoxLayout>

namespace devtools {

YourToolGui::YourToolGui(QWidget* parent)
    : GuiTool(parent)
    , inputEdit_(new QLineEdit(this))
    , processButton_(new QPushButton(this))
    , outputLabel_(new QLabel(this)) {
    auto* layout = new QVBoxLayout(this);
    DevTools::Ui::applyPageLayout(layout);
    DevTools::Ui::configureFormField(inputEdit_);
    DevTools::Ui::configurePrimaryButton(processButton_);
    layout->addWidget(inputEdit_);
    layout->addWidget(processButton_);
    layout->addWidget(outputLabel_);

    retranslateUi();

    connect(processButton_, &QPushButton::clicked,
            this, &YourToolGui::onProcessClicked);
}

YourToolGui::~YourToolGui() = default;

void YourToolGui::onProcessClicked() {
    QString input = inputEdit_->text();
    QString result = tool_.process(input);
    outputLabel_->setText(result);
}

void YourToolGui::changeEvent(QEvent* event) {
    GuiTool::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
}

void YourToolGui::retranslateUi() {
    processButton_->setText(tr("Process"));
    inputEdit_->setPlaceholderText(tr("Enter input..."));
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
// In Sidemenu::icon()
case ID::YOUR_TOOL_ID:
    return IconUtils::themedIcon(QStringLiteral("your_material_symbol"));
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

Use a named glyph from the bundled Material Symbols Outlined font. Do not add
light/dark SVG files or update an icon resource file.

1. Choose the icon name from the [Material Symbols catalog](https://fonts.google.com/icons).
2. Use the snake_case glyph name with `IconUtils::themedIcon()` and choose an
   appropriate `QStyle::StandardPixmap` fallback.
3. Verify the name with `QIcon::hasThemeIcon()` when adding a new icon.

For example:

```cpp
button->setIcon(
    IconUtils::themedIcon(QStringLiteral("download"), QStyle::SP_ArrowDown));
```

The font is registered by `GuiApplication` and requires Qt 6.9 or later.

### 11. Write Tests

Create test files under `features/your_tool/tests/`:

#### features/your_tool/tests/test_your_tool.cpp

```cpp
#include "features/your_tool/core/your_tool.h"

#include <QtTest>

class TestYourTool : public QObject
{
    Q_OBJECT

private slots:
    void convertsToUppercase();
    void handlesEmptyInput();
};

void TestYourTool::convertsToUppercase()
{
    YourTool tool;
    QCOMPARE(tool.process("hello"), QString("HELLO"));
}

void TestYourTool::handlesEmptyInput()
{
    YourTool tool;
    QCOMPARE(tool.process(""), QString());
}

QTEST_GUILESS_MAIN(TestYourTool)
#include "test_your_tool.moc"
```

Use Qt Test for feature tests. `QTEST_GUILESS_MAIN` is suitable for tests that
do not create widgets; widget tests use `QTEST_MAIN`. See the [Testing
Guide](testing-guide.md) for registration and `CORE_ONLY` service tests.

#### Register Tests in CMake

Add to `tests/DevToolsTests.cmake`:

```cmake
DevTools_add_test(test_your_tool
    SOURCES
    features/your_tool/tests/test_your_tool.cpp
)
```

For a service test that compiles without the shared GUI test library, register
it with `CORE_ONLY`, list the service implementation in `SOURCES`, and link any
additional Qt or project libraries with `target_link_libraries()`. See the
[Testing Guide](testing-guide.md) for an example.

### 12. Build and Test the Native Module

```bash
# Configure with C++ tests enabled from the repository root
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=arm64-osx \
  -DENABLE_UNIT_TEST=ON
cmake --build build --parallel

# Run tests
ctest --test-dir build --output-on-failure

# Run specific test
./build/test_your_tool

# Build and launch the app to verify
cmake --build build --target run
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
- [ ] Reused `DevTools::Ui` helpers and `DevTools::Ui::Metrics` where applicable
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

# System Architecture

This document describes the architecture of DevTools, including module structure, dependencies, and design patterns.

## Overview

DevTools is a Qt6-based desktop application built with C++17. The architecture uses a modular static library design to reduce compilation time and improve code organization.

## Technology Stack

| Component | Technology |
|-----------|------------|
| Language | C++17 |
| GUI Framework | Qt 6.x (Widgets) |
| Build System | CMake 3.21.1+ |
| Package Manager | vcpkg |
| Code Formatting | clang-format |
| Static Analysis | clang-tidy |

## Directory Structure

Each feature is a self-contained module under `features/{feature}/` with
`core/`, `gui/`, and `tests/` subdirectories. Shared application infrastructure
lives in `features/framework/` and is compiled as the `DevTools_core` library.

```mermaid
flowchart TD
    root["devtools/"]
    root --> features["features/<br/>Feature modules<br/>(core/, gui/, tests/)"]
    features --> framework["framework/<br/>Core framework<br/>DevTools_core"]
    features --> conversion["data_conversion/<br/>JSON/YAML/TOML conversion"]
    features --> qr["qr_code/<br/>QR code generation"]
    features --> image["image/<br/>Resize, rotation, division, transparency"]
    features --> db["db_tool/<br/>Database tool"]
    features --> http["http_request/<br/>HTTP API testing"]
    features --> command["command/<br/>Shell command execution"]
    features --> phrase["phrase_generation/<br/>Random phrase generation"]
    root --> main["main/<br/>Application entry point"]
    root --> resources["res/"]
    resources --> qrc["*.qrc<br/>Qt resource files"]
    resources --> themes["themes/<br/>qlementine theme JSON files"]
    resources --> translations["*.ts<br/>Translation files"]
    root --> tests["tests/<br/>Shared test helpers<br/>(per-feature tests live in features/{feature}/tests/)"]
    root --> distribution["distribution/<br/>Packaging files"]
```

## Module Architecture

### Module Diagram

```mermaid
flowchart TD
    app["DevTools<br/>(executable)"]
    image["DevTools_image_tools_unified"]
    conversion["DevTools_data_conversion"]
    qr["DevTools_qr_code_generation"]
    image_core["DevTools_image_core"]
    core["DevTools_core<br/>(main framework,<br/>features/framework)"]
    app --> image
    app --> conversion
    app --> qr
    image --> image_core
    image_core --> core
    conversion --> core
    qr --> core
```

Other feature modules (`DevTools_http_request`, `DevTools_command`,
`DevTools_phrase_generation`, `DevTools_db_tool`) depend directly on
`DevTools_core` without intermediate shared modules.

### Module List

| Module | Description | Dependencies |
|--------|-------------|--------------|
| `DevTools_core` | Main framework (application, main window, side menu, tool base, exceptions). Sources live under `features/framework/`. | Qt6, qlementine |
| `DevTools_image_core` | Image I/O and editing base classes | DevTools_core |
| `DevTools_image_tools_unified` | Unified image processing tools (resize, rotation, division, transparency) | DevTools_image_core |
| `DevTools_data_conversion` | JSON/YAML/TOML conversion | DevTools_core, yaml-cpp, toml11 |
| `DevTools_qr_code_generation` | QR code generation | DevTools_core |
| `DevTools_http_request` | HTTP API testing | DevTools_core |
| `DevTools_command` | Shell command execution | DevTools_core |
| `DevTools_phrase_generation` | Random phrase generation | DevTools_core |
| `DevTools_db_tool` | Database tool | DevTools_core |

### Core Module Structure

`DevTools_core` encapsulates application infrastructure and is implemented in
`features/framework/`. Its sources are listed via `target_sources()` in
`features/framework/CMakeLists.txt`. Key components:

- **Application Framework**: `features/framework/gui/gui_application.cpp`, `features/framework/core/application_mixin.cpp`
- **Main Window**: `features/framework/gui/main_window.cpp`
- **Navigation**: `features/framework/gui/sidemenu.cpp`, `features/framework/gui/contents_area.cpp`
- **Dialogs**: `features/framework/gui/menubar/about_devtools_dialog.cpp`, `settings_dialog.cpp`
- **Shared UI Design System**: `features/framework/gui/design_system.h`, `design_system.cpp`
- **Tool Base**: `features/framework/gui/gui_tool.cpp`, `features/framework/core/tool/tool.cpp`
- **Exceptions**: `features/framework/core/exception/` (custom exception classes)

## Design Patterns

### MVC-like Architecture

Each tool follows a similar pattern:

```mermaid
flowchart LR
    subgraph module["Tool Module"]
        core_layer["core/<br/>Logic"]
        gui_layer["gui/<br/>Interface<br/>(Widgets + .cpp)"]
    end
    gui_layer --> core_layer
```

- **core/**: Business logic, algorithms, no UI dependencies
- **gui/**: User interface, uses Qt Widgets

### Static Library Pattern

Each feature is compiled as a static library (`STATIC`). The library target is
declared in the root `CMakeLists.txt` with `qt_add_library(${PROJECT_NAME}_your_module STATIC)`
and the source files are listed in `features/your_module/CMakeLists.txt` via
`target_sources()`. The feature is then registered in `features/CMakeLists.txt`
with `add_subdirectory()`.

```cmake
# Root CMakeLists.txt
qt_add_library(${PROJECT_NAME}_your_module STATIC)

# features/your_module/CMakeLists.txt
target_sources(${PROJECT_NAME}_your_module PRIVATE
    core/your_module.h core/your_module.cpp
    gui/your_module_gui.h gui/your_module_gui.cpp
)
```

Benefits:
- Reduced compilation time (only changed modules recompile)
- Clear module boundaries
- Easier testing

## Data Flow

### Tool Selection Flow

```mermaid
flowchart TD
    user["User clicks side menu"] --> side["Sidemenu<br/>(sidemenu.h)"]
    side -->|"signal: itemSelected(Sidemenu::ID id)"| main["MainWindow<br/>(main_window.h)"]
    main -->|"MainWindow::onSidemenuItemSelected"| main
    side -->|"signal: itemSelected(Sidemenu::ID id)"| area["ContentsArea<br/>(contents_area.h)"]
    area -->|"ContentsArea::onSidemenuItemChanged"| area
    area -->|"shows appropriate widget"| tool["Tool GUI<br/>(e.g., QR)"]
```

### Image Processing Flow

```mermaid
flowchart TD
    user["User loads image"] --> gui["GUI Layer<br/>(image_gui.h)"]
    gui <-->|"open/save"| dialog["File Dialog<br/>(Qt native)"]
    gui --> view["ImageView<br/>(image_view.h)<br/>displays image"]
    view -->|"user configures options"| core["Core Layer<br/>(image_*.cpp)<br/>processes image"]
    core --> output["Output<br/>save dialog"]
```

## External Dependencies

### Qt6 Modules

| Module | Purpose |
|--------|---------|
| Qt6::Core | Core functionality |
| Qt6::Widgets | GUI components |
| Qt6::Network | HTTP requests |
| Qt6::Sql | Database operations |
| Qt6::LinguistTools | Translations |
| Qt6::Svg | SVG support required by qlementine |

### vcpkg Libraries

| Library | Purpose | Linking |
|---------|---------|---------|
| toml11 | TOML parsing | Header-only |
| yaml-cpp | YAML parsing | Static |

### Bundled Libraries

| Library | Purpose | Source |
|---------|---------|--------|
| qrcodegen | QR code generation | Project Nayuki |

### FetchContent Libraries

| Library | Purpose | Source |
|---------|---------|--------|
| qlementine v1.4.2 | Modern Qt Widgets `QStyle` and JSON theme support | `https://github.com/oclero/qlementine.git` |

qlementine is fetched by CMake instead of vcpkg because it is built against the active
system Qt6 installation. The project disables qlementine's sandbox and showcase targets and
links the style library into `DevTools_core`.

## Application Styling and Themes

DevTools uses qlementine as the application-wide `QStyle`:

1. `GuiApplication::setup()` creates `oclero::qlementine::QlementineStyle` and installs it
   with `QApplication::setStyle()`.
2. `oclero::qlementine::ThemeManager` loads JSON themes from the Qt resource prefix
   `:/themes`.
3. `GuiApplication::applyColorScheme()` maps the system color scheme to the qlementine
   `Light` or `Dark` theme and also switches the icon theme between `light` and `dark`.

Theme JSON files live in `res/themes/` and are registered in `res/application.qrc`. When
adding or renaming a theme, keep the `meta.name` value unique because qlementine's
`ThemeManager` uses it as the theme identifier.

Feature GUIs use `DevTools::Ui` for shared layout, widget configuration, and metrics.
Colors, borders, and text roles come from the active qlementine style.

## Build System

### CMake Structure

```mermaid
flowchart TD
    cmake["CMakeLists.txt<br/>(main)"]
    cmake --> modules["Module definitions<br/>(qt_add_library)"]
    cmake --> dependencies["Build-order dependencies<br/>(add_dependencies)"]
    cmake --> linking["Link dependencies + usage requirements<br/>(target_link_libraries)"]
    cmake --> tests["Tests<br/>(tests/DevToolsTests.cmake)"]
    cmake --> docs["Docs<br/>(doxygen/DevToolsDocs.cmake)"]
```

### Build Targets

| Target | Description |
|--------|-------------|
| `DevTools` | Main executable |
| `format` | Run clang-format |
| `format-check` | Check formatting |
| `lint` | Run clang-tidy |
| `lint-fix` | Run clang-tidy with fixes |
| `run` | Build and launch DevTools |
| `quality-check` | Run all quality checks |
| `DevTools_docs` | Generate Doxygen docs |

## Translation System

DevTools supports multiple languages using Qt translation tools:

Tracked translation sources:

```mermaid
flowchart TD
    res["res/"] --> ts["dev-tools_ja_JP.ts<br/>Japanese translations"]
```

Generated translation artifacts:

```mermaid
flowchart TD
    build["build/"] --> qm["*.qm<br/>Generated during normal builds"]
```

Translation workflow:
1. Write source strings in English and mark them with `tr()` in code
2. Run `cmake --build build --target update_devtools_translations` when strings change
3. Update the tracked `.ts` translations
4. Normal builds run `lrelease` to compile `.qm` files

## Testing Architecture

Per-feature tests live alongside the feature under `features/{feature}/tests/`.
Shared test helpers (mock utilities, random data generators) live under
`tests/`.

```mermaid
flowchart TD
    tests["tests/"]
    tests --> registration["DevToolsTests.cmake<br/>Test registration"]
    tests --> helpers["test_util.h/.cpp<br/>random_data.h/.cpp<br/>mock_helper.h"]
    feature["features/{feature}/"] --> feature_tests["tests/<br/>Per-feature test sources"]
    feature_tests --> test_file["test_*.cpp"]
```

Tests are registered in `tests/DevToolsTests.cmake` via `DevTools_add_test()`.
See [Testing Guide](testing-guide.md) for details.

## Related Documentation

- [Adding New Tools](adding-new-tools.md) - How to add new tool modules
- [Coding Standards](coding-standards.md) - Code style guidelines
- [Testing Guide](testing-guide.md) - How to write tests

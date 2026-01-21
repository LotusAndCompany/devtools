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

```
devtools/
├── core/               # Business logic and algorithms
│   ├── data_conversion/    # JSON/YAML/TOML conversion
│   ├── exception/          # Custom exceptions
│   ├── image/              # Image processing algorithms
│   ├── qr_tool/            # QR code generation
│   └── tool/               # Tool base classes
├── gui/                # GUI components
│   ├── command/            # Command execution UI
│   ├── data_conversion/    # Data conversion UI
│   ├── db_tool/            # Database tool UI
│   ├── image/              # Image processing UI
│   ├── menubar/            # Menu dialogs
│   ├── phrase_generation/  # Phrase generation UI
│   └── qr_code/            # QR code UI
├── main/               # Application entry point
├── res/                # Resources
│   ├── *.qrc               # Qt resource files
│   └── *.ts                # Translation files
├── tests/              # Unit tests
└── distribution/       # Packaging files
```

## Module Architecture

### Module Diagram

```
                    ┌──────────────────┐
                    │    DevTools      │
                    │   (executable)   │
                    └────────┬─────────┘
                             │
        ┌────────────────────┼────────────────────┐
        │                    │                    │
        ▼                    ▼                    ▼
┌───────────────┐  ┌─────────────────┐  ┌────────────────┐
│  DevTools_    │  │   DevTools_     │  │   DevTools_    │
│  image_*      │  │  data_conversion│  │  qr_code_gen   │
└───────┬───────┘  └─────────────────┘  └────────────────┘
        │                    │                    │
        ▼                    │                    │
┌───────────────┐            │                    │
│  DevTools_    │            │                    │
│  image_core   │            │                    │
└───────┬───────┘            │                    │
        │                    │                    │
        └────────────────────┼────────────────────┘
                             │
                             ▼
                    ┌──────────────────┐
                    │  DevTools_core   │
                    │  (main framework)│
                    └──────────────────┘
```

### Module List

| Module | Description | Dependencies |
|--------|-------------|--------------|
| `DevTools_core` | Main framework, GUI base classes, main window | Qt6 |
| `DevTools_image_core` | Image I/O and editing base classes | DevTools_core |
| `DevTools_image_resize` | Image resize functionality | DevTools_image_core |
| `DevTools_image_rotation` | Image rotation functionality | DevTools_image_core |
| `DevTools_image_division` | Image splitting functionality | DevTools_image_core |
| `DevTools_image_transparent` | Image transparency functionality | DevTools_image_core |
| `DevTools_data_conversion` | JSON/YAML/TOML conversion | DevTools_core, yaml-cpp |
| `DevTools_qr_code_generation` | QR code generation | DevTools_core |
| `DevTools_http_request` | HTTP API testing | DevTools_core |
| `DevTools_command` | Shell command execution | DevTools_core |
| `DevTools_phrase_generation` | Random phrase generation | DevTools_core |
| `DevTools_db_tool` | Database tool | DevTools_core |

### Core Module Structure

The `DevTools_core` module contains:

- **Application Framework**: `gui_application.cpp`, `application_mixin.cpp`
- **Main Window**: `main_window.cpp`
- **Navigation**: `sidemenu.cpp`, `contents_area.cpp`
- **Dialogs**: `about_devtools_dialog.cpp`, `settings_dialog.cpp`
- **Tool Base**: `gui_tool.cpp`, `tool.cpp`
- **QR Code Base**: `qrcodegen.cpp`
- **Exceptions**: Custom exception classes

## Design Patterns

### MVC-like Architecture

Each tool follows a similar pattern:

```
┌─────────────────────────────────────┐
│            Tool Module              │
├─────────────────────────────────────┤
│  ┌─────────┐      ┌─────────────┐   │
│  │  core/  │      │    gui/     │   │
│  │         │◄────►│             │   │
│  │ Logic   │      │ Interface   │   │
│  │         │      │ (.ui + .cpp)│   │
│  └─────────┘      └─────────────┘   │
└─────────────────────────────────────┘
```

- **core/**: Business logic, algorithms, no UI dependencies
- **gui/**: User interface, uses Qt Widgets

### Static Library Pattern

Each module is compiled as a static library (`STATIC`):

```cmake
qt_add_library(${PROJECT_NAME}_module_name STATIC
    core/module_name/logic.cpp
    gui/module_name/gui.cpp
    gui/module_name/gui.ui
)
```

Benefits:
- Reduced compilation time (only changed modules recompile)
- Clear module boundaries
- Easier testing

## Data Flow

### Tool Selection Flow

```
User clicks side menu
        │
        ▼
┌───────────────┐
│   SideMenu    │
│  (sidemenu.h) │
└───────┬───────┘
        │ signal: toolSelected(ToolId)
        ▼
┌───────────────┐
│  MainWindow   │
│(main_window.h)│
└───────┬───────┘
        │
        ▼
┌───────────────┐
│ ContentsArea  │
│(contents_area)│
└───────┬───────┘
        │ shows appropriate widget
        ▼
┌───────────────┐
│   Tool GUI    │
│ (e.g., QR)    │
└───────────────┘
```

### Image Processing Flow

```
User loads image
        │
        ▼
┌───────────────┐     ┌───────────────┐
│  GUI Layer    │────►│  File Dialog  │
│ (image_gui.h) │◄────│  (Qt native)  │
└───────┬───────┘     └───────────────┘
        │
        ▼
┌───────────────┐
│  ImageView    │ displays image
│(image_view.h) │
└───────┬───────┘
        │ user configures options
        ▼
┌───────────────┐
│  Core Layer   │ processes image
│(image_*.cpp)  │
└───────┬───────┘
        │
        ▼
┌───────────────┐
│    Output     │ save dialog
└───────────────┘
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

### vcpkg Libraries

| Library | Purpose | Linking |
|---------|---------|---------|
| toml11 | TOML parsing | Header-only |
| yaml-cpp | YAML parsing | Static |

### Bundled Libraries

| Library | Purpose | Source |
|---------|---------|--------|
| qrcodegen | QR code generation | Project Nayuki |

## Build System

### CMake Structure

```
CMakeLists.txt (main)
├── Module definitions (qt_add_library)
├── Dependencies (add_dependencies)
├── Linking (target_link_libraries)
├── Tests (tests/DevToolsTests.cmake)
└── Docs (doxygen/DevToolsDocs.cmake)
```

### Build Targets

| Target | Description |
|--------|-------------|
| `DevTools` | Main executable |
| `format` | Run clang-format |
| `format-check` | Check formatting |
| `lint` | Run clang-tidy |
| `lint-fix` | Run clang-tidy with fixes |
| `quality-check` | Run all quality checks |
| `DevTools_docs` | Generate Doxygen docs |

## Translation System

DevTools supports multiple languages using Qt Linguist:

```
res/
├── dev-tools_ja_JP.ts    # Japanese translations
├── dev-tools_en.ts       # English translations
└── dev-tools.qrc         # Resource compilation
```

Translation workflow:
1. Mark strings with `tr()` in code
2. Run `lupdate` to update .ts files
3. Use Qt Linguist to translate
4. Run `lrelease` to compile .qm files

## Testing Architecture

Tests are organized by module:

```
tests/
├── DevToolsTests.cmake       # Test configuration
├── core/                     # Core logic tests
│   ├── data_conversion/
│   ├── exception/
│   └── image/
├── gui/                      # GUI tests
└── helpers/                  # Test utilities
```

See [Testing Guide](testing-guide.md) for details.

## Related Documentation

- [Adding New Tools](adding-new-tools.md) - How to add new tool modules
- [Coding Standards](coding-standards.md) - Code style guidelines
- [Testing Guide](testing-guide.md) - How to write tests

# DevTools - AI Agent Guidelines

macOS desktop application built with C++17 and Qt6, providing integrated developer utilities.

## Build

```bash
# Configure with CMake (vcpkg toolchain)
cd build && cmake .. -DCMAKE_TOOLCHAIN_FILE=/opt/homebrew/share/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
make -j8

# Enable and run unit tests
cmake .. -DENABLE_UNIT_TEST=ON
ctest

# Format check
cmake --build build --target format-check

# Static analysis
cmake --build build --target lint
```

## Code Formatting

- All C++ code MUST conform to `.clang-format` (Google base + Qt adaptations)
- All files MUST conform to `.editorconfig` rules:
  - Files MUST end with a newline (`insert_final_newline = true`)
  - Use LF line endings
  - Trim trailing whitespace
  - Indent with spaces (4 for C++, 2 for XML/Qt UI files)
- Column limit: 100 characters
- Braces: Allman style for functions/classes/structs, K&R for control statements
- Pointer/reference alignment: right-aligned (`int *ptr`, `int &ref`)
- Include order: local → Qt → third-party → standard library → system
- See `.clang-format` and `.clang-tidy` for full details

## Naming Conventions

- Classes/Structs/Enums: `PascalCase`
- Functions: `camelCase`
- Variables/Members: `snake_case`
- Constants/Macros: `SCREAMING_SNAKE_CASE`
- Files: `snake_case`

## Architecture

- `core/`: business logic (no Qt Widgets dependency; QtCore, QtSql, QtNetwork allowed)
- `gui/`: Qt6 Widgets UI layer
- `tests/`: Google Test based unit tests
- Each feature is a static library module

### Adding a New Module

1. Define `qt_add_library(${PROJECT_NAME}_module_name STATIC ...)` in `CMakeLists.txt`
2. Add to `MODULE_LIST`
3. Place business logic in `core/`, UI components in `gui/`

## Qt Conventions

- Use new signal/slot syntax: `&Class::method`
- Set parent for `new`-allocated widgets (Qt parent-child ownership)
- Wrap UI text with `tr()` for i18n
- Qt macros (Q_OBJECT, Q_PROPERTY, Q_SIGNALS, Q_SLOTS, emit) are used as-is

## Git Workflow

### Conventional Commits

```
<type>(<scope>): <subject>
```

type: feat, fix, docs, style, refactor, test, chore, build, ci, perf

- Subject starts lowercase, use present tense imperative mood
- release-please determines version automatically based on type

### Branch Naming

`<type>/<description>` (lowercase, hyphen-separated):
- feature/, fix/, doc/, refactor/, hotfix/
- With issue number: `feature/123-description`

### Pull Requests

- PR title must follow Conventional Commits format (validated by CI)
- Follow `.github/PULL_REQUEST_TEMPLATE.md`

## i18n

- Qt Linguist for English/Japanese support (`res/dev-tools_ja_JP.ts`, `res/dev-tools_en.ts`)
- Update: `/opt/homebrew/bin/lupdate -recursive . -ts res/dev-tools_en.ts res/dev-tools_ja_JP.ts`
- Compile: `/opt/homebrew/bin/lrelease res/dev-tools_ja_JP.ts`
- Wrap all UI text with `tr()`

## Dependencies

- vcpkg: toml11, yaml-cpp (see `vcpkg.json`)
- Qt6: Core, Widgets, LinguistTools, Sql, Network
- Bundled: qrcodegen (`core/qr_tool/qrcodegen/`)

## Pre-commit Hooks

Setup: `pre-commit install --install-hooks -t pre-commit -t commit-msg -t pre-push`

Runs: clang-format, trailing-whitespace, EOF fixer, large file check, commit-msg validation, pre-push build.

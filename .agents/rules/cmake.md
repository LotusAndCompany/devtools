---
paths:
  - "**/CMakeLists.txt"
  - "**/*.cmake"
---

# CMake Rules

- Declare dependencies explicitly with `add_dependencies()` when target build
  order matters.
- Prefer target-based CMake commands (`target_link_libraries`,
  `target_include_directories`, `target_sources`) over global include/link
  settings.
- Add new static feature modules with `qt_add_library(...)` and register them
  in `MODULE_LIST`.
- Place business logic in `core/` and UI components in `gui/`.
- Update `vcpkg.json` when adding vcpkg-managed packages.
- Add tests in `tests/DevToolsTests.cmake` using `DevTools_add_test()`.
- Qt AUTOMOC, AUTORCC, and AUTOUIC are enabled; do not add manual moc/uic/rcc
  generation unless there is a specific build-system reason.

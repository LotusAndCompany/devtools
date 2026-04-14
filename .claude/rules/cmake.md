---
paths:
  - "**/CMakeLists.txt"
  - "**/*.cmake"
---

# CMake Rules

- Define new modules with `qt_add_library(${PROJECT_NAME}_name STATIC ...)` and add to `MODULE_LIST`
- Declare dependencies explicitly with `add_dependencies()`
- Update `vcpkg.json` when adding vcpkg packages
- Add tests in `tests/DevToolsTests.cmake` using `DevTools_add_test()`
- Qt AUTOMOC, AUTORCC, AUTOUIC are enabled (no manual setup needed)

---
paths:
  - "**/CMakeLists.txt"
  - "**/*.cmake"
---

# CMake Rules

- Declare dependencies explicitly with `add_dependencies()`
- Update `vcpkg.json` when adding vcpkg packages
- Add tests in `tests/DevToolsTests.cmake` using `DevTools_add_test()`
- Qt AUTOMOC, AUTORCC, AUTOUIC are enabled (no manual setup needed)

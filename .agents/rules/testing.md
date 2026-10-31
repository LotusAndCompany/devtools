---
paths:
  - "tests/**"
  - "**/test_*.cpp"
  - "**/test_*.h"
---

# Testing Rules

- C++ tests are disabled by default. Configure from the repository root with
  `VCPKG_ROOT` set, the vcpkg toolchain, `arm64-osx`, and
  `-DENABLE_UNIT_TEST=ON` before running them.
- Register new tests in `tests/DevToolsTests.cmake` with `DevTools_add_test()`.
- Run native tests with `ctest --test-dir build --output-on-failure`.
- Keep core tests focused on business logic and avoid GUI dependencies unless
  the component under test is explicitly in a `gui/` subdirectory.
- Use `ASSERT_*` only when continuing would make the rest of the test invalid;
  otherwise prefer `EXPECT_*`.

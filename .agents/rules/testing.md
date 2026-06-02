---
paths:
  - "tests/**"
  - "**/test_*.cpp"
  - "**/test_*.h"
---

# Testing Rules

- Tests are disabled by default; configure with `cmake .. -DENABLE_UNIT_TEST=ON`
  before running them.
- Register new tests in `tests/DevToolsTests.cmake` with `DevTools_add_test()`.
- Prefer `ctest --output-on-failure` when diagnosing failures.
- Keep core tests focused on business logic and avoid GUI dependencies unless
  the component under test is explicitly in `gui/`.
- Use `ASSERT_*` only when continuing would make the rest of the test invalid;
  otherwise prefer `EXPECT_*`.

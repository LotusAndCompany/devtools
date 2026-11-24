# M1 final verification record

## Environment

- macOS 26.6.2, arm64
- CMake 4.0.3
- Homebrew Qt 6.9.2
- Existing `build` directory configured with unit tests enabled
- `QT_QPA_PLATFORM=offscreen`, `TMPDIR=/private/tmp`

## Commands and results

| Command | Result |
| --- | --- |
| `cmake --build build --target format` | Passed; CMake reconfigured and clang-format ran. |
| `cmake --build build --target test_local_file_selection_service test_regex_evaluation_service test_regex_tool test_settings_dialog DevTools -j 8` | Passed after separating `DevTools_core` from `DevTools_framework_gui`. |
| `cmake --build build --target test_data_conversion test_application_settings_service test_local_file_selection_service DevTools -j 8` | Passed after reducing `DevTools_core` to Qt Core and making the data-conversion core test `CORE_ONLY`. |
| `cmake --build build -j 8` | Passed for the application and every registered test target. |
| `QT_QPA_PLATFORM=offscreen TMPDIR=/private/tmp ctest --test-dir build --output-on-failure` | 56/56 passed in 29.56 seconds. HTTP tests used loopback sockets and ran with local socket access. |
| `cmake --build build --target format-check` | Passed. |
| `git diff --check` | Passed after the code and documentation edits. |

`otool -L` inspection of the `CORE_ONLY` service tests confirmed no Qt Widgets
linkage, including the new `test_diff_tool` and `test_timestamp_conversion`.
Core-only targets that require image or document APIs link Qt Gui; HTTP links
Qt Network, and database service tests link Qt SQL. Regex evaluation, settings,
file selection, command, template, application settings, HTTP, database, data
conversion, QR, image, Markdown, diff and timestamp service tests compile and
run without a Widget test harness. The full build reported duplicate
`dbMain` translation entries and two ignored `nodiscard` results in the basic
emitter test; neither stopped the build or test run.

## Issue #132 acceptance map

- Eleven feature areas and the shared application shell are listed in
  [the architecture inventory](m1-architecture-inventory.md), with the
  project-owned UI classes, service responsibilities, dependency diagram,
  Bridge input/output proposals, Rust candidates and residual Qt boundaries.
  The inventory includes the diff and timestamp tools added on latest `main`.
- Service and error results are values/enums; presentation dialogs and
  localized messages remain in Widgets. No Core source references a Widget,
  dialog, message box or feature GUI header.
- `DevTools_core` no longer links Qt Widgets. Shared UI code is in
  `DevTools_framework_gui`, which depends on Core. UI-to-service source
  dependencies point one way.
- Async HTTP and regex lifecycle, request IDs, error/timeout behavior,
  cancellation, stale-result handling and cleanup ownership are recorded in
  the architecture inventory and feature records.
- Existing Widget regressions and UI-free service tests ran in the complete
  56-test suite. No tests were disabled or weakened.
- Application and test executables built successfully. Exact commands,
  environment and results are listed above.

## Limits

The run used macOS and Qt 6.9.2 only. Qt Widgets tests used the offscreen
platform; no manual visual review was performed. Loopback HTTP behavior was
covered, but external DNS/TLS/proxy behavior was not. SQL integration was
covered with SQLite; additional driver configurations and signed-app bookmark
flows need their platform environments. Qt regex interruption remains
cooperative, HTTP retains Qt's default timeout policy, and its shared elapsed
time accounting for overlapping requests is a documented existing defect.

The feature branch locally merged `origin/main` at `855f9d3` (merge commit
`ad718b6`). No issue state change, push or PR update was made as part of this
verification.

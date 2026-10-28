# M1 regex worker extraction

## Scope and contract

- Objective: make the existing asynchronous regex evaluation callable without a
  Widget, while retaining its inputs, output, and interruption behavior.
- Changed files: regex worker, its GUI caller, test registration, and a
  core-only test. The regex engine, replacement syntax, UI and persisted
  settings are unchanged.
- Input: request ID, pattern, text, replacement, Qt pattern options and global
  flag. Output: the same request ID, match and capture offsets, replacement,
  validity flag and Qt error string via `finishedMatching`.
- Empty pattern returns no matches and the original text. An invalid pattern
  returns the original text, a false validity flag and an error. Single mode
  uses the first match; global mode collects all matches. Offsets use Qt string
  positions. Named captures and replacement behavior remain Qt-dependent.
- Qt dependencies: Qt Core (`QThread`, `QObject`, `QRegularExpression` and
  string/container types). The GUI still owns debouncing, the 500 ms watchdog,
  display and rejection of stale request IDs. A future bridge must explicitly
  encode those request IDs, capture offsets, validity and errors.

## Lifecycle and remaining work

Interruption remains cooperative: collection checks between matches and the
worker checks again before replacement. A running individual regex operation
is not forcibly interrupted. The evaluation service tracks timed-out and
superseded workers, disconnects and interrupts them during destruction, and
does not wait for an in-progress regex call. Workers have no service parent and
schedule their own deletion when they finish. The service's shutdown path is
covered by `destroyingServiceDoesNotWaitForActiveWorker`.

The remaining M1 scope includes the rest of the feature/UI audit, extraction,
service contracts, integration and existing-UI regression checks. QR
validation and image processing have since moved to services; PNG file writing
was also extracted in [m1-qr.md](m1-qr.md). Database table operations,
template persistence and other feature processing remain to be audited. HTTP
transport was subsequently extracted; see [m1-http-service.md](m1-http-service.md).
This slice alone does not complete issue #132.

## Verification

Environment: macOS 26.6.2 arm64, Homebrew Qt 6.9.2, existing `build` directory
with `ENABLE_UNIT_TEST=ON`.

- `cmake --build build --target format`: passed.
- `cmake --build build --target format-check`: passed.
- `cmake --build build --target test_regex_worker test_regex_tool -j 8`: passed.
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build -R
  '^test_regex_(tool|worker)$' --output-on-failure`: 2/2 passed. The new
  worker test covers global named captures with Unicode, first-only mode,
  empty and invalid patterns, and request ID and replacement output.
- `otool -L build/test_regex_worker`: Qt Core and Qt Test, no Qt Widgets or
  Qt Gui linkage. The test uses `QTEST_GUILESS_MAIN`.
- `cmake --build build -j 8`: passed after serializing CMake regeneration;
  two concurrent build invocations had caused a transient FetchContent
  reconfiguration failure. The successful build also emitted the two existing
  `nodiscard` warnings in the basic emitter test.
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`:
  30/30 passed in 15.02 seconds. This reused the existing Unix Makefiles build
  with Homebrew yaml-cpp and toml11; it does not establish that the M0 clean
  Ninja build failures have been resolved.
- No manual UI interaction or visual regression was performed in this slice.

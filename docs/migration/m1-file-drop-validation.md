# M1 file-drop validation

## Scope and responsibility

Image drag acceptance now calls `ImageProcessingService::isReadableImageFile`
for the file and image-format check. SQLite URL selection now uses
`SQLitePaths::firstValidFilePath`, which shares the existing database path
policy while preserving the original local path string used by the main
window. `SQLitePaths::firstValidLocalFile` continues to return an absolute path
for the connection window's security-scoped file flow.

The Widgets still own drag events, local URL selection from `QMimeData`, and
loading the chosen file. The validation rules themselves are UI-independent.

## Compatibility and Bridge proposal

Image drops continue to accept only existing regular files recognized by
`QImageReader`; directories, missing paths, and unrecognized files are
rejected. SQLite drops continue to accept existing regular files with a
case-insensitive `.db`, `.sqlite`, or `.sqlite3` suffix and select the first
valid local URL. The main-window path is returned exactly as supplied by the
local URL. Connection-window paths remain absolute, as before.

A future Bridge can expose `validateDroppedImage({path}) -> {accepted,
format, error}` and `selectDroppedDatabase({urls}) -> {path, accepted,
error}`. The host owns local-file permissions and security-scoped access. Qt
image-format plugins and platform bookmark handling remain in native services.

Rust candidates include URL-list validation and file-type policy. Qt's image
reader plugins and macOS security-scoped bookmarks remain behind these
services until format and permission compatibility is specified.

## Verification and limits

Environment: macOS 26.6.2 arm64, Homebrew Qt 6.9.2, existing `build` directory
with `ENABLE_UNIT_TEST=ON`.

- `cmake --build build --target test_image_processing_service
  test_basic_image_view test_database_connection DevTools`: passed.
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build -R
  '^test_(image_processing_service|basic_image_view|database_connection)$
  --output-on-failure`: 3/3 passed. Headless coverage checks recognized and
  unrecognized image files, missing paths, directories, SQLite suffix and
  file validity, URL filtering, and both original-path and absolute-path
  results. The image Widget regression exercises accepted and rejected drops.
- `otool -L build/test_image_processing_service`: Qt Core, Gui and Test; no Qt
  Widgets.
- `cmake --build build --target format-check` and `git diff --check`: passed.
- The main-window drag-to-open flow was not manually exercised, and no
  non-macOS platform was tested.

The duplicate path checks are now in their feature services. Remaining work
includes regex worker lifecycle coordination, the complete class/dependency
inventory, and the final all-target/all-test verification for Issue #132.

## Execution record

At 2026-10-27 18:45 JST, the Issue #132 goal runtime was 4,452 seconds. Runtime
includes work and waits across the task, not model execution time. Human
intervention and per-command Codex execution time were not independently
measured. No PR or issue state change was made.

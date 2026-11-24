# M1 file-drop validation

## Scope and responsibility

Image drag acceptance now calls `ImageProcessingService::isReadableImageFile`
for the file and image-format check. SQLite URL selection now uses
`LocalFileSelectionService` through `SQLitePaths`, sharing the existing
database path policy while preserving the original local path string used by
the main window. The shared service also validates dropped SQLite paths in
`MainWindow` without a dependency on the database feature.
`SQLitePaths::firstValidLocalFile` continues to return an absolute path for
the connection window's security-scoped file flow.

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

- `cmake --build build --target test_local_file_selection_service
  test_image_processing_service test_basic_image_view test_database_connection
  DevTools`: passed.
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build -R
  '^test_(local_file_selection_service|image_processing_service|basic_image_view|database_connection)$
  --output-on-failure`: 4/4 passed. Headless coverage checks case-insensitive
  suffixes, local URL selection, recognized and unrecognized image files,
  missing paths, directories, SQLite file validity, and original-path versus
  absolute-path results. The image Widget regression exercises accepted and
  rejected drops.
- `otool -L build/test_image_processing_service`: Qt Core, Gui and Test; no Qt
  Widgets.
- `cmake --build build --target format-check` and `git diff --check`: passed.
- The main-window drag-to-open flow was not manually exercised, and no
  non-macOS platform was tested.

The path selection and suffix policy live in `LocalFileSelectionService` and
the image readability check lives in `ImageProcessingService`. The final
class/dependency inventory and full verification are recorded in
[m1-architecture-inventory.md](m1-architecture-inventory.md) and
[m1-final-verification.md](m1-final-verification.md).

## Execution record

At 2026-10-27 18:45 JST, the Issue #132 goal runtime was 4,452 seconds. Runtime
includes work and waits across the task, not model execution time. Human
intervention and per-command Codex execution time were not independently
measured. No PR or issue state change was made.

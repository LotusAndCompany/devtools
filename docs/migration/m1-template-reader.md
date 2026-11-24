# M1 template reader extraction

## Scope and contract

- Objective: move template listing and reading out of the Widget without
  changing the on-disk format or working-directory behavior.
- Changed files: phrase reader, its GUI caller, test registration and a
  core-only test. Saving, deletion, validation, dialogs and clipboard remain
  in the GUI and are outside this slice.
- The reader lists `content/*.txt`, relative to the process working directory,
  in QDir's existing order. A file's first line is its title and the remainder
  is its body. A missing or unreadable file returns an empty body and leaves
  the provided title unchanged, as before.
- Qt Core handles directory enumeration, files, streams and strings. A future
  bridge must validate externally supplied names before calling the reader;
  the existing internal API does not restrict filenames to that directory.

## Remaining work

The Widget still performs save and delete operations. It removes an old
file before attempting to create its replacement and does not report write
failure; this existing data-loss risk must be handled as a separately reviewed
behavior change, not silently treated as fixed by this extraction. File errors,
read-only storage, duplicate titles, malformed files, UI regression and
working-directory changes need further coverage. No user template files were
used by the new test.

### Follow-up

The later [template repository extraction](m1-template-repository.md) moved
validation, dirty comparison, save and delete operations out of the Widget and
added core and Widget tests. The write-failure behavior described above remains
unchanged and is still tracked as a compatibility issue.

## Verification

Environment: macOS 26.6.2 arm64, Homebrew Qt 6.9.2, existing `build` directory
with `ENABLE_UNIT_TEST=ON`.

- `cmake --build build --target format`: passed.
- `cmake --build build --target test_template_reader DevTools_phrase_generation -j 8`:
  passed.
- `ctest --test-dir build -R '^test_template_reader$' --output-on-failure`:
  1/1 passed. The test covers a missing directory, `.txt` filtering, UTF-8
  title and body, and missing-file behavior in an isolated temporary directory.
- `cmake --build build -j 8`: passed.
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`:
  31/31 passed in 9.11 seconds. This reused the existing build and does not
  resolve the separate clean Ninja baseline discrepancies.
- Manual UI and persistence-write tests were not performed in this slice.

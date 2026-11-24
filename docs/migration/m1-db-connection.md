# M1 database connection extraction

## Scope and contract

- Objective: move shared connection construction, SQLite file validation and
  bookmark fallback from Widgets into callable non-Widget services.
- Scope: database connection, SQLite path/access helpers, existing DB
  Widgets, CMake, and focused core and Widget tests. Driver selection, data
  format, file-dialog behavior and user-visible strings are unchanged.
- `DatabaseConnection::open` accepts driver type, host, database, username,
  password and an optional Qt connection name. It creates/configures and opens
  a Qt SQL connection; errors are returned through the resulting connection's
  `lastError`. For SQLite, only the database name is configured. A retry sets
  a replacement database name and opens the same connection. Connection info
  retains the existing display-name and bookmark encoding and omits password.
- `SQLitePaths` checks file existence/type and the db/sqlite/sqlite3 extension,
  and selects the first valid local file from a URL list. Bookmark restoration
  attempts a decoded bookmark first and falls back to the stored path, retaining
  the existing security-scope object until the consuming UI releases it.
- Qt Core and SQL remain dependencies; macOS access also uses Foundation.
  The connection and its queries remain on their creation thread. Opening is
  synchronous; no cancellation or application timeout existed previously.
  Default-named connections and the temporary unique name used for direct
  SQLite files retain their existing behavior and cleanup responsibilities.

## Limits and remaining work

The file picker, confirmation/error dialogs and password prompt remain in the
GUI. Native picker cancellation, security-scoped access under a signed sandbox,
stale valid bookmarks, MySQL/PostgreSQL connections, shutdown and connection
replacement with active query models were not verified in this slice. Tests do
not prove these behaviors on other macOS versions. In particular, replacing a
Qt default connection can invalidate existing users of that connection, and
failed direct connections still use the original cleanup behavior; either
change would require a separate compatibility review.

Table listing, table models and the direct SQLite validation query still need
further separation. Other features, bridge contracts and full UI regression
also remain. No password is written into the connection-history object by the
new connection-info helper. Existing history may still contain legacy secrets.

## Verification

Environment: macOS 26.6.2 arm64, Qt 6.9.2 and an existing test-enabled build.
Tests use isolated temporary SQLite files and isolated QSettings, not user data.

- `cmake --build build --target format` and `format-check`: passed.
- The full `cmake --build build -j 8` completed successfully.
- `QT_QPA_PLATFORM=offscreen TMPDIR=/private/tmp ctest --test-dir build -R
  '^test_(database_connection|db_main_connection|db_connection_window|connection_selector)$'
  --output-on-failure`: 4/4 passed. The tests cover real SQLite open, SQL and
  reopen, failed open, missing driver, connection metadata, Unicode paths,
  extension checks, invalid-bookmark fallback, direct connection, input/drop
  connection, and reconnect from stored history. No native picker is invoked.
- A sandboxed native-platform run without offscreen aborted because no screen
  was available. Offscreen is used only for these focused non-network tests.
- The existing connection-window assertions still fail with the default
  `/var` temporary-directory spelling; `/private/tmp` avoids that baseline
  mismatch without editing or disabling the test.
- `ctest --test-dir build --output-on-failure`: 38/39 passed in 18.84 seconds;
  the only failure was the existing connection-window path assertion above.
- `otool -L build/test_database_connection` listed Qt Core, SQL and Test
  without Qt Widgets or Gui.
- No manual UI or visual review was performed.

## Execution record

At 2026-10-27 10:37 JST the goal's elapsed time was 933 seconds, including
prior slices and waits. Model execution time and human intervention time were
not separately measured. The next work is table operations, remaining feature
services and comprehensive regression checks.

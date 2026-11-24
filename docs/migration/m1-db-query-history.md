# M1 database query and history extraction

## Scope and contract

- Objective: move SQL execution and connection-history persistence from the
  Widgets into services while preserving the existing query model and settings.
- Scope: query service, history service, existing DB Widgets, CMake and tests.
  No database drivers, SQL syntax, settings keys, UI text or dialogs changed.
- `QueryService::execute` accepts a Qt SQL connection and SQL text, executes
  synchronously on the caller's thread, and returns the executed query, its
  success flag and the original Qt SQL error. The GUI transfers the query to
  `QSqlQueryModel` as before, retaining Qt's incremental result fetching and
  existing error dialogs. A failed execution does not replace the old model.
- `ConnectionHistory` reads and writes `db_tool/connectionHistory` through
  QSettings using the existing compact JSON strings. Adding preserves invalid
  raw entries, removes the first duplicate by type, host and database, adds the
  new entry first and limits the list to ten. Reading filters invalid entries;
  writing a filtered list preserves the prior delete behavior. Startup's empty
  check still considers the raw list. Unknown JSON fields are preserved.
- Dependencies are Qt Core and Qt SQL. The caller must keep the connection
  valid for the query/model's lifetime and use it on the connection's thread.
  Closing or replacing the connection while results remain active is subject
  to the existing Qt driver behavior.

## Limits and remaining work

SQL remains synchronous: there is no application-level timeout, cancellation,
request ID or background worker in the original workflow. A long query can
block the UI, and this extraction does not change that behavior. A bridge
cannot serialize the returned `QSqlQuery`; it must define a bounded result
and incremental-fetch contract, preserving NULL, binary values, errors and
connection lifetime, before exposing this service outside Qt. The tests use
SQLite; MySQL, PostgreSQL, driver errors, locking, long-running requests and
shutdown behavior remain unverified.

Connection creation, driver configuration, SQLite bookmark restoration and
file-path validation were subsequently extracted; see
[m1-db-connection.md](m1-db-connection.md). File-picker fallback, table
listing and table models remain in Widgets. Moving them requires preserving
macOS security-scoped file access and existing error/cancellation behavior. History entries may contain
legacy unknown fields, including secrets; preserve them for compatibility but
never log them or automatically expose them through a bridge.

## Verification

Environment: macOS 26.6.2 arm64, Homebrew Qt 6.9.2, existing `build` with
`ENABLE_UNIT_TEST=ON`. Tests use an in-memory SQLite database or a temporary
settings file, not the user's database or settings.

- `cmake --build build --target format`: passed.
- `cmake --build build --target test_query_service test_connection_history
  test_query_page -j 8`: passed.
- `ctest --test-dir build -R
  '^test_(query_service|connection_history|query_page)$' --output-on-failure`:
  3/3 passed. Coverage includes SQL mutation, Unicode, invalid SQL, closed
  connection, 600-row incremental fetch, Widget success and error behavior,
  malformed history, duplicate replacement, unknown fields and the ten-entry
  limit, and reloading the written temporary settings file. The Widget test
  dismisses actual Qt message boxes automatically.
- `otool -L build/test_query_service` showed Qt Core, SQL and Test without Qt
  Widgets or Gui.
- `cmake --build build -j 8` and the `format-check` target passed.
- `ctest --test-dir build --output-on-failure`: 35/36 passed in 16.90 seconds.
  The existing connection-window test failed two assertions on the known
  `/var` versus `/private/var` path spelling difference. The full run preceded
  the addition of the settings-reload assertion, which passed in the focused
  rerun above. No test was disabled or weakened.
- No manual visual review was performed.

## Execution record

At 2026-10-27 10:31 JST the goal's recorded elapsed time was 602 seconds,
including earlier work and waits. Per-command model execution and human
intervention times were not separately measured. Changes remain uncommitted
on the existing checkout; no commit or PR was created. Continue with database
connection and table operations, then the other remaining feature services and
full regression verification.

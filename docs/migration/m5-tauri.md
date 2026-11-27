# M5: move feature processing to Tauri

## Scope and result

This checkpoint is based on the Tauri foundation in PR #145, branch
`feature/issue-135-tauri-foundation`, at the selected base snapshot `47bffb1`.
The working branch contains the M5 implementation and evidence described here.

The current React contract has 31 operations. All 31 are implemented in the
Tauri host, and the frontend TypeScript contract, C++ reference registry, and
Rust Tauri registry are checked for exact operation-name parity by
`operation-contract-parity.test.mjs`. Feature processing now runs in Rust with
`serde_json`, `serde_saphyr`, `toml`, `pulldown-cmark`/`ammonia`, `fancy-regex`,
`image`, `qrcode`, `chrono`, `reqwest`, and SQLx. OS dialogs, clipboard,
settings, database history, and filesystem access stay behind the Tauri bridge.

The React source uses only the shared bridge contract. Database identifiers
are host-neutral (`sqlite`, `mysql`, `postgres`); the Qt adapter maps them to
Qt SQL driver names internally. About metadata uses a host-neutral `runtime`
field. No Qt API names or bindings remain in `frontend/src`.

The current app includes 11 tools: the nine M0 tools, Text Diff, and Timestamp
Conversion. Color Converter is **not implemented in the PR #145 base**. Its
separate PR #122 remains an open draft and is not merged into this branch. Per
the user's scope decision, Color Converter remains unimplemented and is excluded
from M5; it is not counted in the 31-operation contract.

## Acceptance ledger

| Issue #136 requirement | Evidence in this branch | Remaining check |
|---|---|---|
| Tauri runs every current feature | All 31 registered operations have Rust handlers; shared operation parity test passes; macOS UI smoke opened all 11 routes. The UI also converted a precision-sensitive JSON integer, opened/resized/reset an image, and saved verified image, QR, and Markdown files through native pickers. | Clipboard was not driven through the UI. |
| Compare against Qt on common inputs | Shared fixtures cover JSON/YAML/TOML, timestamps, diff, Markdown safety, QR contents, commands, regex, text-file reads, SQLite queries, image rotation/flips/resize/transparency/split, and legacy settings. Qt `NativeBridge` tests and the real WKWebView-to-bridge test use the pure-operation fixtures; Rust tests use the same fixtures. The Tauri SQLx integration test also passed against temporary MySQL and PostgreSQL servers. | The Qt server comparison could not run here: the QMYSQL plugin is absent, and QPSQL points to a missing Postgres.app `libpq.5.dylib`. The non-macOS Qt WebEngine host was removed by the M3 macOS-only decision. |
| Compare errors, formatting, cancellation, persistence, and cleanup | Shared fixtures check returned values and selected error codes. Rust tests cover HTTP cancellation, SQLite/MySQL/PostgreSQL transactions, limits, settings, Qt settings migration, phrase migration, and atomic text/settings writes. | Full per-operation cancellation races, DB server disconnect through the event bridge, and file overwrite/error dialogs remain unverified. |
| Keep the frontend independent of Qt APIs | TypeScript contract uses neutral database IDs and runtime metadata; grep of `frontend/src` finds no Qt bridge/API names. C++ conversion stays inside the Qt adapter. | None identified in frontend source. |
| Exercise real HTTP, DB, files, and OS integrations | Tauri UI smoke made an external HTTP request (example.com returned 200), opened and saved through native file pickers, and connected/query/listed/disconnected a real SQLite fixture. Rust tests use a real local HTTP server, SQLite file, MySQL 8.0.42 server, and PostgreSQL 17.11 server. | MySQL/PostgreSQL were exercised through SQLx, not through the Qt reference host. Clipboard and non-macOS OS integration were not exercised. |
| Record commands, environment, outcomes, and gaps | This document and the test logs below record commands and unrun items. The user accepted the current Tauri UI/usability/accessibility and the safer Markdown sanitization; Color Converter is explicitly excluded from the M5 scope. | No open product-scope decision remains for this branch. |

## Operation coverage

| Operations | Rust implementation | Contract evidence |
|---|---|---|
| `formatJson`, `convertData` | `src-tauri/src/lib.rs`, `src-tauri/src/m5.rs` | `tauri-format-json.json` and `tauri-convert-data.json`; Qt and Rust compare output, including large integer, YAML dates/tags, flow/block styles, TOML null warnings, and indentation. YAML integers outside signed/unsigned 128-bit range and TOML integers outside signed 64-bit range fail instead of being rounded through `f64`. |
| `convertTimestamp`, `compareText` | `src-tauri/src/lib.rs` | `tauri-timestamp.json` covers negative seconds/milliseconds, offsets, invalid input, and New York DST gap/overlap. `tauri-compare-text.json` compares aligned diff rows. |
| `renderMarkdown` | `src-tauri/src/m5.rs` with `ammonia` sanitization | `tauri-markdown.json` checks Unicode content, scripts, event handlers, and unsafe links. The Qt reference emits a `javascript:` href; the Tauri output strips it. This is an intentional security improvement. The shared preview iframe uses a sandbox without script permission, a restrictive CSP, and `no-referrer`. |
| `generateQr`, `saveQrImage` | `src-tauri/src/m5.rs`, native save dialog in `src-tauri/src/lib.rs` | `tauri-qr-content.json` compares all nine QR payload types against Qt. The Tauri UI generated a QR and saved `m5-qrcode-integration.png` through the native picker; the 200×200 PNG was verified and removed. |
| `commandData` | `src-tauri/src/m5.rs` | `tauri-command.json` compares Git, Docker, Compose, quoting, and invalid quoted input. Commands are generated as text and are never run. |
| `evaluateRegex`, `regexSettings` | `src-tauri/src/m5.rs`, `src-tauri/src/lib.rs` | `tauri-regex.json` covers Unicode UTF-16 offsets, captures, replacement, flags, lookbehind, and invalid syntax. Regex settings are stored separately from application settings. |
| `phraseData` | `src-tauri/src/lib.rs` | Rust tests import `content/*.txt` into app data only when needed, do not overwrite existing targets, preserve source files, and reject unsafe filenames. |
| `sendHttpRequest` | `src-tauri/src/lib.rs` with `reqwest` | Rust local-server tests exercise query encoding, status errors, and cancellation; the Tauri UI smoke received an external HTTP 200. Passwords and headers are not written to logs. Proxy/TLS/redirect behavior remains unverified. |
| `openTextFile`, `saveTextFile` | `src-tauri/src/lib.rs` with the Tauri dialog plugin | `tauri-open-text-file.json` compares UTF-8 reads; helper tests cover Unicode paths, size limits, and atomic writes. The native open picker was exercised. Markdown Save wrote a file through the native picker; its UTF-8 content was verified and the temporary file removed. Overwrite prompts and permission errors were not manually exercised. |
| `getSettings`, `saveSettings`, `saveLastUsedTool`, `getAboutInfo` | `src-tauri/src/lib.rs` | Unit tests cover versioned settings, invalid schemas, and atomic writes. Qt settings migration uses `qt-settings-migration.plist`, backs up the source, imports allowlisted values, removes passwords/bookmarks, and is idempotent. Tauri UI loaded Settings and About. Full restart/window restoration was not manually checked. |
| `copyToClipboard` | `src-tauri/src/lib.rs` with the Tauri clipboard plugin | Input validation and text/image paths are implemented. Clipboard was deliberately not touched during UI smoke; runtime clipboard success/failure is unverified. |
| `imageOpen`, `imageEdit`, `imageReset`, `imageSave`, `imageSplit`, `imageTakeDropped` | `src-tauri/src/lib.rs`, `src-tauri/src/m5.rs` | Shared pixel fixtures compare Qt and Rust rotations, flips, nearest resize, RGB/HSL/HSV transparency, and split output including remainder pixels. Image dimensions and previews are bounded; writes are atomic and split output is capped at 10,000 files. Tauri UI opened `res/dev-tools_logo.png`, resized it from 128×128 to 64×64, reset it, and saved a verified 128×128 PNG to `/private/tmp`; the temporary file was removed. Finder drop and large-image behavior remain unverified end to end. |
| `databaseHistory`, `databaseSelectFile`, `databaseConnect`, `databaseDisconnect`, `databaseQuery`, `databaseTables` | `src-tauri/src/lib.rs` with SQLx SQLite/MySQL/PostgreSQL drivers | `tauri-sqlite-query.json` and `devtools_sample.db` cover SQLite values, NULL, 64-bit values, transactions, tables, and disconnect. An env-gated integration test connected to temporary MySQL 8.0.42 and PostgreSQL 17.11 servers, queried the server version, used a temporary table, and verified rollback. `database_query` now tolerates SQLx `describe()` failing for valid transaction commands such as MySQL `BEGIN`. The Qt history fixture and Rust migration test confirm `QSQLITE`/`QMYSQL`/`QPSQL` values normalize to neutral bridge IDs without copying secrets. |

The Rust registry has the same 31 operation names as the frontend and C++
registry. This checks operation availability; it does not alone prove each
operation's behavior, so the feature-specific evidence above remains necessary.

## Persisted-data and security behavior

- Qt settings are read only from the allowlisted legacy plist path. Before
  migration, the original is copied to `migration/qsettings-source.plist`.
  Application settings, regex settings, and database history are written to
  Tauri app-config storage; repeated startup does not overwrite existing Tauri
  settings. Passwords and security-scoped bookmarks are excluded from imported
  history.
- Phrase templates are copied from the legacy working-directory `content`
  folder only when a destination template does not already exist. The original
  files remain in place.
- User-selected paths are acquired by Rust through native dialogs. Text and
  image writes use a sibling temporary file and replace the chosen destination
  only after the write succeeds. Text is UTF-8 and bounded to 1 MiB.
- Database history stores host-neutral IDs in Tauri. Previously generated
  preview history containing Qt driver identifiers is normalized on read.
  Passwords are supplied only for a connection attempt and are not stored.
- Markdown uses an allowlist sanitizer in Rust. The preview has an additional
  iframe sandbox and CSP. The old Qt reference formatter retains its historical
  `javascript:` link in the returned HTML; the Tauri formatter removes it. The
  common React preview sandbox blocks scripts and top-level navigation in both
  hosts, while Tauri-exported HTML is sanitized at the source.

## Remaining Qt code

- The Qt Widgets host, its operation handlers, and C++ tests remain in the
  repository as the existing host and parity reference. The Tauri runtime does
  not call into those handlers; `bun run tauri:build` builds the React frontend
  and Rust host without CMake or Qt.
- The frontend talks through the neutral bridge contract. The Qt adapter still
  translates database identifiers to Qt SQL driver names for the legacy host.
- This checkpoint does not delete the Qt host or its dependencies from the
  repository; any repository-wide removal is a separate migration decision.

## Verification record

Environment: macOS 26.7.2, arm64; Qt 6.12.0; Bun 1.3.14. The initial
migration checks used a temporary Rust 1.99.1 toolchain and Cargo home under
`/private/tmp`. On 2026-11-11, this PC was set up with rustup stable Rust/Cargo
1.99.1 and `rustfmt` in `~/.rustup` and `~/.cargo`; `~/.zshenv` loads Cargo's
PATH. The final Rust check, test, and format commands below used that persistent
installation and the normal `src-tauri/target` directory. The application build
is macOS-only. The
env-gated server test used disposable local MySQL 8.0.42 and PostgreSQL 17.11
clusters under `/private/tmp`; those servers were not part of the normal CTest
run. The Qt server comparison was attempted separately and could not load the
installed PostgreSQL driver dependency.

| Command | Result |
|---|---|
| `bun run lint` | Passed. |
| `bun run format:check` | Passed. |
| `bun run typecheck` | Passed. |
| `bun run test` | Passed, 24 tests in 12 files. |
| `bun run build:tauri` | Passed. Vite reported the existing ~700 kB app-chunk advisory. |
| `cargo fmt --check` | Passed after formatting. |
| `TZ=America/New_York cargo test` | Passed, 28 Rust tests, including the configured MySQL/PostgreSQL integration. |
| `cargo test --quiet` with `CARGO_TARGET_DIR=/private/tmp/issue136-target` | Passed, all 30 current Rust tests. This rerun had no configured server variables; the real MySQL/PostgreSQL integration remains evidenced by the separate configured run above. |
| `cargo check --locked` | Passed with the persistent `~/.cargo` / `~/.rustup` installation. |
| `cargo test --locked --quiet` | Passed, 30 tests with the persistent installation and normal project target directory. |
| `cargo fmt -- --check` | Passed with the installed `rustfmt` component. |
| `cargo check` | Passed. |
| `cmake --build build -j 8` | Passed. |
| `cmake --build build --target format-check -j 8` | Passed. |
| `ctest --test-dir build --output-on-failure` | Passed, 40/40 test targets. The WKWebView integration target passed the shared pure-operation fixtures through its actual script-message bridge; a dedicated New York test target covers DST. The optional Qt MySQL/PostgreSQL server test skipped in this run. |
| `make -C /private/tmp/issue136-build test_image_processing_service/fast -j2` then `/private/tmp/issue136-build/test_image_processing_service` | Passed, 8 Qt image tests including shared transform, transparency, resize, and split-pixel fixtures. |
| `cargo fmt -- --check`, `clang-format --dry-run --Werror features/image/tests/test_image_processing_service.cpp`, `git diff --check` | Passed after the final changes. |
| `cargo test connects_to_local_mysql_and_postgres_when_configured` | Passed against temporary local MySQL 8.0.42 and PostgreSQL 17.11 servers. |
| `bun run tauri:build` (from `frontend/`) | Passed with the persistent local Rust toolchain; produced `src-tauri/target/release/bundle/macos/DevTools.app` (29.52 MiB). The Vite build emitted a 699.91 kB chunk advisory. |

The UI smoke evidence was collected from the Tauri preview app on macOS. It
opened all 11 routes and exercised representative conversions, Markdown
preview and save, command generation, timestamp conversion, regex, text diff,
QR generation and native PNG save, an external HTTP request, Settings/About,
and SQLite connect/query/table browsing/disconnect. The QR output was verified
as a 200×200 PNG; the Markdown output was verified as UTF-8 text. Both temporary
files were removed. It also converted the JSON integer `9007199254740993`
without rounding and exercised image open, resize, reset, and native save using
a repository logo; the generated `/private/tmp` PNG was verified and removed.
The phrase list was empty in that preview profile. Clipboard was not touched.

The current restricted runner could not write to the repository's existing
`target/` or `build/Testing/Temporary/LastTest.log`. The current Rust and Qt
image reruns therefore used copy-on-write build directories under
`/private/tmp`; the full 40/40 CTest result above is from the earlier writable
run.

## Remaining review and limits

- The user accepted the current Tauri UI/usability/accessibility and the safer
  Markdown sanitization. Color Converter remains outside M5 by explicit scope
  decision; PR #122 is still unmerged.
- Only macOS arm64 was exercised. macOS overwrite/permission dialogs, clipboard
  runtime behavior, system settings migration from an installed
  production Qt profile, and window-state restoration across restart were not
  manually verified.
- M3 removed the non-macOS Qt WebEngine host when project support narrowed to
  macOS. M5 therefore compares the current macOS WKWebView host; it does not
  establish Qt WebEngine parity or support for other operating systems.
- Tauri SQLx integration passed against local MySQL and PostgreSQL servers. The
  Qt server comparison remains unavailable on this machine: `libqsqlmysql` is
  absent, and `libqsqlpsql.dylib` links to the missing
  `/Applications/Postgres.app/Contents/Versions/14/lib/libpq.5.dylib`. The Qt
  test reported QPSQL as available but could not load its driver. HTTP proxy,
  TLS failure, redirect, and response-limit behavior were not exercised against
  external services.
- Image split/save dialogs, Finder drop, and performance at the 100-million
  pixel limit remain unverified. Small deterministic pixel fixtures cover the
  transformations and generated split pieces.
- Signing, notarization, release packaging, and other operating systems belong
  to M6 and were not tested here.

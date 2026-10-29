# M3: React UI migration

M3 replaces the normal tool and settings screens with React while keeping
feature processing in the existing C++ services. The Qt application remains
the host for WebEngine, window lifecycle, native dialogs, clipboard, and
SQLite or image file drag-and-drop. The app's regular navigation no longer
opens a Qt Widgets tool screen.

## Screen and behavior mapping

The source inventory for this migration has 11 registered tool screens. The
Color Converter from the separate, unmerged PR #122 is not part of this set.

| Screen | React controls and retained behavior | Native operation / data source |
|---|---|---|
| Image Editor | Open or drop an image, reset, save, resize by dimensions or scale, aspect ratio, smoothing, rotation, flip, click-to-select color transparency, and split by count or cell size | `imageOpen`, `imageTakeDropped`, `imageEdit`, `imageReset`, `imageSave`, `imageSplit`; `ImageProcessingService`; native image and folder dialogs. Dropped paths stay in C++. |
| Phrase Generation | List, select, create, save, delete, copy; Ctrl/Cmd+S, Ctrl/Cmd+N, Ctrl/Cmd+Shift+C, and Delete outside text editors | `phraseData`; existing `TemplateReader` / `TemplateRepository`; clipboard bridge |
| Command Generation | Git, Docker, and Docker Compose catalog, option selection, validation, generated command, and copy | `commandData`; existing `CommandService` |
| HTTP Request | GET/POST/PUT/DELETE, 10 query rows, custom headers, Basic Auth, request body, response status/body/timing; GET and DELETE do not send the body; invalid headers are reported | `sendHttpRequest`; Qt Network; existing `HttpService::urlWithParameters` encoding |
| Data Conversion | JSON, YAML block/flow, TOML, indentation, open input and save output | `convertData`, `openTextFile`, `saveTextFile`; existing parsers and emitters; native file dialogs |
| Database | SQLite/MySQL/PostgreSQL connection form, connection history, SQLite file chooser and drop, table list, SQL query, bounded result grid, disconnect | `databaseHistory`, `databaseSelectFile`, `databaseConnect`, `databaseTables`, `databaseQuery`, `databaseDisconnect`; Qt SQL and existing database services |
| QR Code | All nine types (text, URL, email, phone, SMS, Wi-Fi, contact, calendar, location), validation, preview, copy image, save PNG | `generateQr`, `copyToClipboard`, `saveQrImage`; existing QR validation/content/image services; native save dialog |
| Markdown Preview | Edit, preview, preview theme, open/save Markdown, export HTML; preview runs in a sandboxed frame with a restrictive CSP | `renderMarkdown`, `openTextFile`, `saveTextFile`; existing `MarkdownService`; native file dialogs |
| Timestamp Conversion | Unix seconds, Unix milliseconds, ISO-8601, UTC/local results, copy each result | `convertTimestamp`; existing timestamp conversion service and clipboard bridge |
| Regex Tester | Qt regular-expression syntax, six options, presets, match offsets/captures, replacement, copy, quick reference, persisted input/settings | `evaluateRegex`, `regexSettings`, `copyToClipboard`; existing regex evaluation/settings services |
| Text Diff | Two editable inputs and native line-diff results with added/removed rows | `compareText`; existing diff service |

Shared React UI includes welcome/tool search, tabs, keyboard navigation,
settings, language and theme selection, About, pending-operation cancellation,
last-used-tool persistence, and startup sidebar/last-tool preferences. The
Qt menu still hosts About/Settings navigation, and Qt handles OS dialogs and
window behavior. The current baseline has no favorites implementation, so
M3 does not add one.

## Contract and safety boundaries

React calls only versioned `nativeBridge` operations. C++ validates operation
names and payloads, keeps file paths in the native process, and performs file,
network, database, image, and feature work. Request bodies, database results,
image previews, regex matches, and individual inputs have explicit bounds in
the native operation paths. Markdown HTML is isolated in a sandboxed iframe;
the bundled page CSP and WebEngine request policy remain in force.

The WebEngine host and existing UI modules still compile in the same CMake
build. Legacy Qt screen classes remain in the source tree because the existing
Qt GUI unit tests and feature targets still use them; they are not reachable
through the new app navigation. Removing that test/build support is a separate
cleanup item and is not evidence that a Qt WebEngine runtime path works.

## Verification record

Local checks for this M3 source snapshot (2026-10-29):

- `npm run typecheck` passed.
- `npm test` passed (6 frontend contract tests).
- `cmake --build build --target DevTools -j 4` built the application.
- `cmake --build build --target test_native_bridge -j 4` and
  `ctest --test-dir build -R '^test_native_bridge$' --output-on-failure`
  passed, including all nine QR formats through the versioned bridge.
- `ctest --test-dir build -R '^test_db_connection_window$' --output-on-failure`
  passed after canonicalizing macOS `/var` and `/private/var` paths in the test.
- `ctest --test-dir build -R '^(test_native_bridge|test_qt_webengine_frontend)$'`
  passed after adding the native file-drop, pending-image retry, database
  re-drop, and bundled-route integration checks.
- `cmake --build build --target quality-check` passed after the final source
  changes, including clang-tidy, lint, translation completeness, and
  `format-check`.
- `ctest --test-dir build --output-on-failure` passed all 58 tests.
- `cmake --build build --target run` built the app and staged
  `QtWebEngineProcess.app`, but the standalone app aborted in this automation
  session with Qt's `Argument list is empty` WebEngine startup error. No
  interactive screenshot was captured; the Qt WebEngine integration test
  loaded the bundled UI, connected the bridge, and rendered all 11 tool routes.

The project acceptance checklist also requires interactive Qt WebEngine
coverage for all tools, keyboard and accessibility checks, data/edge-case
equivalence (including large input, timezone/DST, TLS/proxy/redirect,
transactions and cancellation), and a distributable-package launch. A passing
frontend or C++ unit suite alone does not satisfy those checks. Record the
remaining checks here before advancing M3 to verified.

## HMR during development

From the repository root, run the Vite server in one terminal:

```bash
cd frontend
bun install
bun run --bun dev
```

In another terminal, launch a Debug build with the loopback URL:

```bash
DEVTOOLS_WEB_UI_DEV_SERVER_URL=http://127.0.0.1:5173 cmake --build build --target run
```

Release builds ignore this variable and load the embedded `qrc:/frontend`
assets. Stop the Vite server when development ends. See the security and
resource details in [m2-react-webengine.md](m2-react-webengine.md).

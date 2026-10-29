# M2 React and Qt WebEngine foundation

M2 adds an embedded React screen to the existing Qt Widgets application. The
screen uses a versioned Qt WebChannel contract to call real C++ services. It
does not replace the existing tools or move the application to another UI
runtime.

## Stack and build

- React and TypeScript are bundled by Vite.
- The UI uses shadcn/ui component conventions with Base UI primitives and
  Tailwind CSS. The Button component is stored in `frontend/components/ui/`.
- `frontend/bun.lock` pins the resolved JavaScript dependency tree.
- CMake runs `bun install --frozen-lockfile` and `bun run --bun build`, then
  embeds the generated HTML, JavaScript, and CSS in the
  `DevTools_web_frontend` Qt resource collection.
- Bun 1.3.14 is pinned in `frontend/package.json`.
- A production app opens `qrc:/frontend/index.html`; it does not need Bun or a
  development server at runtime.
- For HMR, run `bun install` and `bun run --bun dev` in `frontend/`, then launch
  a Debug app with
  `DEVTOOLS_WEB_UI_DEV_SERVER_URL=http://127.0.0.1:5173`. The adapter accepts
  only that exact loopback origin. Release builds ignore the environment
  variable.

The resource flow and bridge are based on [Qt WebChannel], [Qt WebEngine], and
[Vite's backend integration]. The component setup follows [shadcn/ui's Base UI
configuration] and [Base UI's React setup].

## NativeBridge contract v1

The only object registered in `QWebChannel` is `nativeBridge`. The M2
foundation initially exposed `formatJson`, `openTextFile`, and `saveTextFile`.
M3 expands the allowlisted operations to cover the migrated tools; see the
[M3 screen and behavior mapping](m3-react-ui.md#screen-and-behavior-mapping).
React does not receive a general file path, process command, Qt object, or
arbitrary native method.

Every request has this envelope:

```json
{
  "contractVersion": "1",
  "requestId": "unique-ascii-id",
  "operation": "formatJson",
  "payload": { "text": "{\"ok\":true}", "indentation": "2" }
}
```

`requestId` accepts 1–80 ASCII letters, digits, `_`, or `-`; duplicate IDs are
rejected while an operation is active. At most four operations can run at
once. `startOperation` returns immediately. An accepted operation reports
`operationStarted`, zero or more `operationProgress` events, then one terminal
event: `operationCompleted`, `operationFailed`, or `operationCancelled`.
Envelope validation failures (invalid request ID or version, unsupported
operation, duplicate ID, or the concurrency limit) report only
`operationFailed`; they do not emit `operationStarted` or `operationProgress`.
Payload validation occurs after `operationStarted` and may report
`operationFailed` without progress. Progress is an integer from 0 to 100. The
React adapter subscribes once and disconnects every signal handler when the
screen unmounts.

Errors use `{ "code": string, "message": string }`. Defined codes are
`INVALID_REQUEST`, `UNSUPPORTED_VERSION`, `DUPLICATE_REQUEST`,
`TOO_MANY_REQUESTS`, `INVALID_INPUT`, `SIZE_LIMIT_EXCEEDED`,
`INVALID_ENCODING`, `PERMISSION_DENIED`, `IO_ERROR`, and `INTERNAL_ERROR`
(reserved for future operations). Text files must contain valid UTF-8; invalid
byte sequences are rejected with `INVALID_ENCODING`. Cancellation is
cooperative: file reads and writes check the token between 64 KiB chunks; JSON
parsing checks before and after the parser and emitter calls. A cancellation
request cannot undo a file already committed by `QSaveFile`.

### Serialization and size bounds

- Request string values are UTF-8 and limited to 1 MiB each. The current C++
  text operations enforce this limit again after WebChannel decoding.
- Formatted JSON output is limited to 4 MiB. A normalized JavaScript message
  is limited to 2 MiB so Base64 binary DTOs have room for encoding overhead.
- `Date` values become ISO-8601 UTC strings. C++ `QDateTime` values must use
  the same UTC string representation.
- JavaScript `bigint` and C++ `int64` values use decimal strings. Unsafe
  JavaScript integer numbers are rejected to avoid precision loss.
- `Uint8Array` / `QByteArray` values use
  `{ "$type": "bytes", "base64": "..." }`; raw bytes are limited to 1 MiB.
- `null` remains distinct from an empty string. Empty strings are preserved.
- `undefined`, functions, symbols, non-finite numbers, and non-plain objects
  are rejected before serialization.

File operations show a native `QFileDialog`. The selected path stays in C++;
the page receives the file name and text only. Reads are bounded while
streaming. Saves use `QSaveFile` and are committed only after a successful
write.

## Security boundaries

- The page is either the bundled `qrc:/frontend/index.html` document or, in a
  Debug build, the exact `http://127.0.0.1:5173` origin.
- The request interceptor blocks other network origins and unrelated Qt
  resources. The page rejects external top-level navigation, pop-up windows,
  and subframes.
- The embedded page has a restrictive Content Security Policy. JavaScript
  cannot access local files, remote resources, the clipboard, or plugins.
- The WebEngine profile is off the record, with cache and persistent cookies
  disabled.
- User JSON is shown in a plain textarea. It is not parsed into HTML or a
  preview DOM.
- Production assets are embedded in the app; Release builds do not read the
  development-server environment variable.

## Verification record

The standalone frontend commands are:

```bash
cd frontend
bun install --frozen-lockfile
bun run --bun typecheck
bun run --bun test
bun run --bun build
```

The C++ bridge test is registered as `test_native_bridge`. Configure with unit
tests enabled, then build and run it:

```bash
cmake -S . -B <build-dir> -DENABLE_UNIT_TEST=ON
cmake --build <build-dir> --target test_native_bridge
ctest --test-dir <build-dir> -R test_native_bridge --output-on-failure
```

The application target also builds the frontend and embeds the resource
assets.

### Local verification (2026-10-28)

- `npm run typecheck`, `npm test` (6 tests), and `npm run build` passed. The
  frontend test suite includes a timed-out Qt WebChannel initialization case.
- CMake built `DevTools` and `test_native_bridge`; the focused CTest run passed.
- `format-check` passed, and the translation catalog compiled with the three
  new native-dialog strings and the React tool label.
- The production resource page loaded from `qrc:/frontend/index.html`, connected
  to contract v1, and formatted JSON through the C++ parser/emitter. Native open
  and save dialogs read and wrote a local JSON file.
- A Debug app connected to Vite on loopback. A CSS color edit appeared in the
  running page through HMR; the source was restored afterward. The Vite process
  was stopped and the temporary launchd override was removed.

### Size and runtime observations

| Metric | M2 observation | Conditions / limitation |
|---|---:|---|
| Embedded frontend assets | 285,202 bytes raw; 88,577 bytes summed gzip | JS 269,535 / 84,259 bytes, CSS 14,974 / 3,925 bytes, HTML 693 / 393 bytes; individually compressed |
| Pre-deployment `.app` | 22,920 KiB | Local CMake Debug build; M0's local Xcode bundle was 18,084 KiB, so this is directional rather than a like-for-like build comparison |
| Manually deployed `.app` | 344,696 KiB (336.6 MiB) | Unsigned local copy; macdeployqt with an explicit Homebrew library path; 19 framework directories, including QtWebEngineCore at 242,308 KiB. M0's manually deployed bundle was 107,516 KiB (105.0 MiB), about 3.2× smaller. macdeployqt emitted unresolved-rpath diagnostics while inspecting plugins; a signed release package and launch test remain unverified. |
| Launch to first accessibility tree | 2,153 ms | One CMake Debug process start through CUA `getApp`; M0's five starts had a 1,031 ms median and 1,044 ms p95. Build configuration and sample count differ. |
| Select bundled React screen to connected accessibility tree | 1,451 ms | One Debug launch with no dev-server environment set; the qrc page connected at contract v1. Not a repeatable distribution. |
| 42-byte JSON format click to completed accessibility state | 657 ms | One CUA end-to-end sample through the real C++ parser/emitter. M0's 25.2 ms synchronous `setValue` sample used a different interaction and is not directly comparable. |
| React page process footprint | DevTools 345.7 MiB (peak 484.3 MiB); QtWebEngineProcess 55.3 MiB (peak 56.8 MiB) | One Debug/HMR session, sampled with `vmmap -summary` about four minutes after launch. M0 used Activity Monitor's Memory column, so these numbers are not directly comparable; no production idle or no-page sample was collected. |

The WebEngine runtime adds a substantial macOS bundle footprint. Keep this
measurement visible in the migration decision and collect a Release build,
signed package, repeatable launch-to-accessibility distribution, and same-method
idle / peak memory samples before making a final distribution or performance
claim.

### Sources

- [Qt WebChannel](https://doc.qt.io/qt-6/qwebchannel.html)
- [Qt WebEngine Widgets](https://doc.qt.io/qt-6/qtwebenginewidgets-module.html)
- [Vite backend integration](https://vite.dev/guide/backend-integration.html)
- [shadcn/ui Base UI configuration](https://ui.shadcn.com/docs/changelog/2026-07-base-ui-default)
- [Base UI React quick start](https://base-ui.com/react/overview/quick-start)

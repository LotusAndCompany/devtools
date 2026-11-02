# PR #142 Qt WebView feasibility

This record follows the Qt WebView migration design supplied for PR #142. It
distinguishes verified repository state from SDK and macOS runtime checks that
have not been performed.

## Decision

**G0 is blocked. Keep Qt WebEngine as the current host and do not start the
Qt WebView host replacement.** The installed SDK cannot compile the C++ QWebView
API, and the inspected Qt 6.11.2 public API/backend does not provide the
application-owned pre-navigation allowlist required by the current host.
Original-file drag-and-drop also remains unverified on QWebView.

This is a gate result for the proposed Qt WebView path. It does not mark the
separate Tauri migration milestone or this task as complete.

## Baseline

| Item | Verified state |
|---|---|
| Repository branch | feature/issue-133-134-react-ui |
| Checked-out commit | 58ea590979cab3c25d3e23aff57a3a29c028260e |
| PR #142 | Open and unmerged; GitHub head matches the checked-out commit |
| Business contract | Bridge v2 |
| Frontend tools | 11 registered React tool screens |
| Contract coverage | 31 TypeScript operations and 31 C++ registry entries; names match exactly |
| Current host | QtWebEngineAdapter, QWebChannel, QWebEnginePage, request interceptor |
| Current drop path | MainWindow and WebEngine child-widget handlers pass original image/SQLite paths to C++ |

The screen-to-operation and retained-behavior inventory is in
[M3 React UI migration](m3-react-ui.md). The contract and registry are
[frontend/src/shared/bridge/types.ts](../../frontend/src/shared/bridge/types.ts)
and
[features/web_frontend/gui/bridge/operation_registry.cpp](../../features/web_frontend/gui/bridge/operation_registry.cpp).

The 31 operations are grouped as follows:

| Area | Operations |
|---|---|
| Image | imageOpen, imageTakeDropped, imageEdit, imageReset, imageSave, imageSplit |
| Phrase | phraseData |
| Command | commandData |
| HTTP | sendHttpRequest |
| Data conversion and files | formatJson, convertData, openTextFile, saveTextFile |
| Timestamp | convertTimestamp |
| Diff | compareText |
| Markdown | renderMarkdown |
| QR | generateQr, saveQrImage |
| Regex | evaluateRegex, regexSettings |
| Database | databaseHistory, databaseSelectFile, databaseConnect, databaseTables, databaseQuery, databaseDisconnect |
| Shared shell | getSettings, saveSettings, copyToClipboard, getAboutInfo, saveLastUsedTool |

SQLite file drop is a native path outside the operation registry. Neither that
path nor image drop sends a local file path to React.

## G0 evidence

| Gate check | Result | Evidence |
|---|---|---|
| Compile QWebView with Widgets | Not run: no Qt 6.11 SDK is installed | qmake selects Homebrew Qt 6.9.2; the separate Qt SDK is 6.9.3. Searches found no QWebView C++ header in either installation. Qt documents the C++ QWebView class as available since Qt 6.11. |
| JavaScript result callback | API confirmed, compilation unverified | The Qt 6.11.2 public header declares runJavaScript with a QVariant result callback. |
| Native macOS backend | Runtime unverified | Qt documents that macOS uses the system web view. The installed SDK cannot build the C++ QWebView host. |
| Pre-navigation rejection | **Fails the required public-API gate** | The public QWebView header exposes urlChanged and loadingChanged, but no navigation-policy hook. The Qt 6.11.2 Darwin delegate allows schemes handled by WKWebView and opens unhandled schemes through NSWorkspace; target=_blank is sent to that external path. Observing urlChanged and then stopping is after the navigation decision. |
| Original-file image and SQLite drop | Not verified | Qt states that WebView events are not guaranteed to propagate into Qt's event-delivery system. QWidget::createWindowContainer embeds a native child window; the existing parent-widget filters therefore cannot be assumed to receive WKWebView drops. A real Finder drop test is still required. |

The API evidence is from Qt's [QWebView class reference](https://doc.qt.io/qt-6/qwebview.html),
[Qt WebView platform overview](https://doc.qt.io/qt-6/qtwebview-index.html),
[Qt 6.11.2 public QWebView header](https://raw.githubusercontent.com/qt/qtwebview/v6.11.2/src/webview/qwebview.h),
[Qt 6.11.2 Darwin backend](https://raw.githubusercontent.com/qt/qtwebview/v6.11.2/src/plugins/darwin/qdarwinwebview.mm),
and [QWidget::createWindowContainer](https://doc.qt.io/qt-6/qwidget.html#createWindowContainer).

The navigation result is a design blocker independent of the missing SDK:
the inspected standard public API/backend cannot preserve the existing
top-level document allowlist. No Qt private API, source fork, or alternate
WKWebView host has been adopted.

## Current verification baseline

| Check | Result |
|---|---|
| Frontend unit tests | bun run test: 21 tests passed in 9 files |
| Frontend production build | bun run build: passed; Vite reports a 676.70 kB JavaScript chunk above its 500 kB warning threshold |
| Native CTest suite | 38 of 38 passed |
| Qt WebEngine frontend test | test_qt_webengine_frontend passed, including all 11 tool routes, settings/about dialogs, QR date selection, phrase discard, data conversion, database operations, and both file-drop tests |
| QWebView compile or runtime test | Not run; installed Qt is older than the C++ QWebView API |
| Finder drag-and-drop, signed package, notarization | Not run |

The integration test had selectors and layout expectations from an earlier UI
structure. They now target the current sidebar, component slots, dialogs, and
calendar, and check that controls do not overlap while allowing responsive
wrapping.

## Engine-independent preparation

The shared-client preparation is implemented:

- The typed interface lives in
  [native-bridge.ts](../../frontend/src/shared/bridge/native-bridge.ts), and
  the Effect-based runner lives in
  [operation-client.ts](../../frontend/src/shared/bridge/operation-client.ts).
- The app hook obtains its connection through
  [create-native-bridge.ts](../../frontend/src/platform/create-native-bridge.ts)
  instead of importing the Qt transport directly.
- The current QWebChannel connection and its contract checks remain under
  frontend/src/platform/qt.
- The operation-client tests now live beside the shared implementation.

After this change, frontend lint, formatting, typecheck, production build, and
all 21 Vitest tests pass. All 38 CTest tests pass, including
test_native_bridge and the Qt WebEngine frontend test.

No Qt WebView host, asset packager, or transport has been added while G0 is
unresolved.

## Next work

Formal host replacement requires a separate architecture decision because the
standard Qt WebView path does not satisfy navigation policy. Options identified
in the design are a maintained Qt patch/fork, a Qt-hosted adapter built on
public Apple WKWebView APIs, or moving the host work directly to Tauri 2. Any
choice must preserve the original-file drop behavior and be verified on macOS.
Until that decision and a Qt 6.11 SDK are available, WebView host, bundle, and
security implementation remain gated.

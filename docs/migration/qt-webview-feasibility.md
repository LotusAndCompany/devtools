# PR #142 Qt WebView feasibility

This record follows the Qt WebView migration design supplied for PR #142. It
preserves the G0 result for the standard Qt WebView API and records the selected
macOS WKWebView adapter separately. Updated 2026-11-02.

## Decision

The standard Qt `QWebView` route remains blocked at G0: its inspected public API
does not expose the pre-navigation policy needed to preserve the current host's
allowlist. The user selected the alternative **Qt host with a direct adapter
over public Apple WKWebView APIs**. On macOS, `WKWebView` is hosted inside the
Qt Widgets window using Qt's foreign-window container; Linux keeps its existing
Qt WebEngine host.

This is an experimental host implementation, not adoption of the standard Qt
WebView module. The project minimum and CI pin now match Qt 6.12.0. The adapter
uses Qt's public native-window hosting API and does not use `QWebView`. Qt 6.12.0
is installed locally; the selected-SDK build and test results are recorded in
the verification table below. This does not establish release readiness.

The WKWebView integration test verifies frontend startup, Bridge v2 requests
and responses, all 11 tool routes, Markdown `srcdoc` rendering, rejection of an
external top-level navigation, and the C++ file-path routing used by image and
SQLite drops. The user manually confirmed that Finder-originated image and
SQLite file drops both work in the Qt 6.12.0 WKWebView app.

## Baseline

| Item | Verified state |
|---|---|
| Repository branch | feature/issue-133-134-react-ui |
| Checked-out commit | `986b439effc338c386c9ab763f50b35a77bbea08` |
| PR #142 | Open and unmerged; published head is `58ea590979cab3c25d3e23aff57a3a29c028260e`; this checkout has four later local commits and uncommitted changes |
| Business contract | Bridge v2 |
| Frontend tools | 11 registered React tool screens |
| Contract coverage | 31 TypeScript operations and 31 C++ registry entries; names match exactly |
| Current macOS host | `WKWebViewAdapter`, hosted in the Qt Widgets window with `QWindow::fromWinId`; native bridge messages are validated by `NativeBridge` |
| Other platform host | `QtWebEngineAdapter`, `QWebChannel`, `QWebEnginePage`, and a request interceptor |
| macOS drop path | AppKit file-URL drag handling on the WKWebView subclass validates image/SQLite paths and passes the original paths to C++; the user confirmed both Finder drops work |
| Other platform drop path | Qt/WebEngine drop handlers route the original image/SQLite paths to C++ |

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
| Compile QWebView with Widgets | Not run; this route was not selected | Qt 6.12.0 is installed, but the direct WKWebView adapter is the selected host. The Qt 6.12.0 public C++ reference still exposes no navigation-policy hook, so compiling the class would not clear G0. |
| JavaScript result callback | Present in the QWebView API; not used by the selected host | The Qt 6.12.0 public C++ reference declares `runJavaScript` with a result callback. |
| Native backend for standard Qt `QWebView` | Not run; not selected | The active macOS host is the direct WKWebView adapter. Its runtime is covered separately in the current verification table. |
| Pre-navigation rejection | **Fails the required public-API gate** | The Qt 6.12.0 QWebView reference lists `urlChanged` and `loadingChanged`, but no navigation-policy hook. Observing `urlChanged` and then stopping is after the navigation decision. |
| Original-file image and SQLite drops through standard Qt `QWebView` | Not applicable to the selected route | The user manually verified Finder-originated image and SQLite drops in the Qt 6.12.0 app using the direct WKWebView adapter; the result is recorded below. |

The API evidence is from Qt's [Qt 6.12.0 QWebView reference](https://doc.qt.io/qt-6.12/qwebview.html),
[Qt 6.9 QML WebView reference](https://doc.qt.io/qt-6.9/qml-qtwebview-webview.html),
[Qt WebView platform overview](https://doc.qt.io/qt-6/qtwebview-index.html),
[Qt 6.11.2 public QWebView header](https://raw.githubusercontent.com/qt/qtwebview/v6.11.2/src/webview/qwebview.h),
[Qt 6.11.2 Darwin backend](https://raw.githubusercontent.com/qt/qtwebview/v6.11.2/src/plugins/darwin/qdarwinwebview.mm),
and [QWidget::createWindowContainer](https://doc.qt.io/qt-6/qwidget.html#createWindowContainer).

The navigation result is a design blocker for the standard Qt WebView API,
independent of the missing SDK. The selected direct WKWebView adapter installs
its own public `WKNavigationDelegate` and rejects non-allowlisted main-frame
navigation before it proceeds. This is a distinct architecture with Apple
framework dependencies; it does not clear the standard Qt WebView gate.

## Current verification baseline

| Check | Result |
|---|---|
| Frontend checks | `bun run lint`, `bun run format:check`, `bun run typecheck`, and `bun run test` passed; Vitest reports 24 tests in 10 files |
| WKWebView production assets | `VITE_DEVTOOLS_HOST=wkwebview bun run --bun build` passed; generated app JavaScript is 696.58 kB and triggers Vite's 500 kB advisory |
| WKWebView frontend integration | `test_wkwebview_frontend` passed under Qt 6.12.0 in actual WKWebView, including all 11 tool routes, Markdown `srcdoc` rendering, external top-level navigation rejection, Bridge v2 request/event delivery, and C++ file-path routing |
| Interactive macOS rendering | Launched the Qt 6.12.0 app executable and confirmed the React home screen, sidebar, and tool cards are visible |
| Native build and CTest | Full Qt 6.12.0 build passed; all 38 CTest tests passed |
| macOS app dependency scan | `otool -L` shows Qt 6.12.0 frameworks and Apple WebKit, with no Qt WebEngine or Qt WebChannel linkage; the app bundle contains no WebEngine helper or plugin |
| Selected Qt 6.12.0 SDK build | Passed in `/private/tmp/devtools-pr142-qt6120-build` |
| Standard Qt `QWebView` compile/runtime | Not run; the standard route is not selected |
| Finder-originated image and SQLite drops | Verified manually by the user in the Qt 6.12.0 app |
| Xcode Release build with build number 11 | Passed with Xcode 27.0 and Qt 6.12.0. The bundle reports marketing version 0.8.0 and build number 11. This is a local working-tree build, not a release-tag Archive. |
| Xcode version accepted by App Store Connect | Not confirmed. Check the currently accepted Xcode version before creating the release Archive. |
| Mac App Store signing and Archive validation | Not complete; distribution is on hold. The local `.app` build selected Apple Development and strict signature verification failed with `CSSMERR_TP_NOT_TRUSTED`. A later attempt reported no `Mac Installer Distribution` certificate/private key and an outstanding Program License Agreement. Apple distinguishes the app-signing identity (`Mac App Distribution`) from the installer-package identity (`Mac Installer Distribution`); no signed App Store `.pkg` or release Archive was produced. |
| macdeployqt database plugins | The local `.app` bundle contains `libqsqlite.dylib` and `libqsqlmimer.dylib`; Mimer cannot load because `/usr/local/lib/libmimerapi.dylib` is missing. `QMYSQL` and `QPSQL` are absent, and `QPSQL`/ODBC were skipped as not App Store compliant. The user guide lists MySQL and PostgreSQL support, so a release package would not yet contain all documented database drivers. |

The WK integration test creates the real embedded frontend and exercises the
current sidebar routes and Markdown sandbox. It also verifies that the trusted
host rejects an external main-frame navigation before loading it. File-drop
tests cover path validation and routing after AppKit supplies file paths; they
do not synthesize a Finder drag event.

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

The shared transport interface remains engine-independent. The selected WK
adapter reuses the Bridge v2 operation contract and existing C++ `NativeBridge`;
the WebEngine transport stays available to non-macOS builds. macOS frontend
assets are emitted as a self-contained classic JavaScript bundle, extracted to
a private temporary directory, and loaded with WKWebView's file read scope
limited to that directory. The adapter uses a nonpersistent website data store,
restricts top-level navigation to the packaged entry or fixed debug server,
rejects popup creation, and accepts only the three existing Bridge message
types from the trusted main frame.

The current macOS build no longer links Qt WebEngine or Qt WebChannel. The
non-macOS build still uses Qt WebEngine, as intended by the platform split.
The adapter implements AppKit file URL drag handling on its WKWebView subclass.
Its path validation and routing are covered in C++ tests, and the user confirmed
that Finder-originated image and SQLite drops work. A cross-app Finder drag was
also attempted through computer-use automation but did not deliver a drop
event; the manual confirmation is the acceptance evidence. A clean app-bundle
rebuild also confirmed that the old `QtWebEngineProcess.app` residue came from
the previous bundle and is not copied into the new WKWebView bundle.

## Next work

The host migration findings are documented; the Mac App Store distribution
work is explicitly on hold at the user's request. The current CMake/Xcode
build path deploys an `.app` bundle with `macdeployqt`; it has not produced a
release Archive or the signed `.pkg` used for Mac App Store submission. Apple's
certificate guide distinguishes the identity that signs the app
(`Mac App Distribution`) from the identity that signs its installer package
(`Mac Installer Distribution`). The latest attempt reported that the Account
Holder must accept the updated Program License Agreement and that no matching
`Mac Installer Distribution` certificate/private key is available. This
package-signing error does not by itself diagnose the app bundle's signing
identity. See Apple's [certificate purposes](https://developer.apple.com/help/account/certificates/certificates-overview/)
and [Mac packaging guidance](https://developer.apple.com/documentation/xcode/packaging-mac-software-for-distribution).
The bundled database plugins also lack the documented MySQL/PostgreSQL drivers,
so that packaging gap remains open.

Keep the standard Qt WebView G0 result separate from the implemented direct
WKWebView adapter. The published PR head and this checkout differ; do not treat
the local host implementation as merged or release-ready.

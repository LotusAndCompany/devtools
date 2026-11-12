# Architecture

DevTools is a macOS desktop application built from a Tauri 2 host, a Rust
backend, and a React/TypeScript frontend. macOS provides WebKit through Tauri.
The application has no Qt host, C++ feature layer, QWebChannel, FFI bridge,
or sidecar process.

## Components

| Area | Implementation |
|---|---|
| Window, commands, file dialogs | Tauri 2 |
| Native feature processing and persistence | Rust in `src-tauri/src/` |
| Screens, navigation, and shared UI | React/TypeScript in `frontend/src/` |
| UI localization | English and Japanese text in the React source |
| Packaging | Tauri DMG bundle for macOS arm64 |

The React application starts in `frontend/src/main.tsx`. Shell composition,
navigation, and tool registration live under `frontend/src/app/`. Feature
screens are under `frontend/src/features/`; shared UI and bridge contracts
are under `frontend/src/shared/`. The only native transport is
`frontend/src/platform/tauri/`.

Rust command registration and application setup are in
`src-tauri/src/lib.rs`; feature operations are in `src-tauri/src/m5.rs`.
The shared contract parity check compares the TypeScript operation contract
with the Rust registry.

## Data and security

Tauri commands validate typed inputs and return structured results. The
application CSP limits the webview to bundled assets, Tauri IPC, and the
specific local or data sources used by the UI. The frontend does not get an
arbitrary shell command or general filesystem bridge.

Settings and app data use Tauri's application data directory. On first run,
the app imports an allowlist from the legacy Qt preferences plist and keeps a
copy of that source plist for recovery. Passwords and SQLite security-scoped
bookmarks are not migrated. See the [installation guide](../getting-started/installation.md)
for backup and recovery steps.

## Build and distribution

Use `bun run tauri:dev` for development and `bun run tauri:build` to create a
local DMG. Pull request CI runs Rust and frontend checks and builds the macOS
bundle. Public distribution requires the release workflow, Developer ID
signing, Apple notarization, and validation. The app has no automatic updater;
users install updates from a newer signed DMG.

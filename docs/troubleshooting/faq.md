# Frequently Asked Questions

## Which platforms are supported?

The current release target is macOS 15 or later on Apple Silicon (arm64).
Windows, Linux, and Intel Macs are outside the verified support scope.

## Does the app need Qt or a separate runtime?

No. The desktop host is Tauri 2 with Rust and React/TypeScript. macOS provides
WebKit. The application does not ship Qt, CMake, vcpkg, QWebChannel, FFI, or a
sidecar runtime.

## How do I update?

Install available updates through the
[Mac App Store](https://apps.apple.com/jp/app/devtools/id6736692957?mt=12).
There is no in-app updater. Keep the app data folder when updating.

## Are database passwords migrated?

No. Passwords are not imported. Enter them again when reconnecting.

## What happens to SQLite file permissions?

Security-scoped bookmarks are not imported. Select the database file again
after migration.

## Where are settings and templates stored?

The app stores them under
`~/Library/Application Support/com.lotusandcompanyinc.dev-tools`. Back up this
folder before upgrades or recovery.

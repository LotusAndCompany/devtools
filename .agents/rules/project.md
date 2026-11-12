---
paths:
  - "**/*"
---

# Project Rules

DevTools targets macOS 15+ on Apple Silicon. The app uses a Tauri 2 Rust
backend and a React/TypeScript frontend with shadcn/ui and Base UI. The OS
provides WebKit. There is no C++/Qt host, QWebChannel transport, FFI, or
sidecar in the supported application.

## Build and checks

~~~bash
cd frontend
bun install --frozen-lockfile
bun run tauri:dev
bun run tauri:build

cd ..
cargo fmt --manifest-path src-tauri/Cargo.toml --all -- --check
cargo clippy --manifest-path src-tauri/Cargo.toml --locked --all-targets -- -D warnings
cargo test --manifest-path src-tauri/Cargo.toml --locked
~~~

A local release build creates a macOS app bundle. Public distribution uses the
shared Xcode Archive scheme and App Store Connect for TestFlight or Mac App
Store release; GitHub Releases do not carry app installers.

## Architecture

- src-tauri/src/: Tauri commands, bridge validation, native file/database
  services, settings, and legacy Qt settings migration.
- frontend/src/app/, features/, and shared/: React shell, tools, shared UI,
  typed bridge contract, and Effect helpers.
- frontend/src/platform/tauri/: the sole frontend/native transport.
- tests/fixtures/: shared contract, migration, image, and database fixtures.

Keep native operations typed in frontend/src/shared/bridge/types.ts and
validate them in Rust. Keep user data writes bounded and atomic, preserve
existing data during migrations, and never store database passwords.

## Dependencies

Rust dependencies are pinned by src-tauri/Cargo.lock; frontend dependencies
are pinned by frontend/bun.lock. Do not add Qt, CMake, vcpkg, FFI, or a
sidecar as an application dependency.

The project supports macOS only. Do not report Windows, Linux, or Intel macOS
as supported without an explicit support-scope change and verification.

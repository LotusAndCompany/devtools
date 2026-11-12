# Build Instructions

[日本語版](docs/ja/BUILD.md)

## Requirements

- macOS 15.0 or later on Apple Silicon (arm64)
- Xcode Command Line Tools
- Rust 1.99.1 with rustfmt and clippy
- Bun 1.3.14

The native host is Tauri 2 with Rust. macOS provides WebKit. The app build
does not use Qt, CMake, or the vcpkg package manager.

## Development

~~~bash
git clone https://github.com/LotusAndCompany/devtools.git
cd devtools/frontend
bun install --frozen-lockfile
bun run tauri:dev
~~~

## Build a local macOS app bundle

~~~bash
cd frontend
bun run tauri:build
~~~

This creates a local `DevTools.app` bundle. To prepare a build for TestFlight
or the Mac App Store, use the shared Xcode Archive scheme and upload it through
App Store Connect as described in [macos/README.md](macos/README.md). The local
bundle build does not upload or publish an app.

## Tests and checks

From the repository root:

~~~bash
cargo fmt --manifest-path src-tauri/Cargo.toml --all -- --check
cargo clippy --manifest-path src-tauri/Cargo.toml --locked --all-targets -- -D warnings
cargo test --manifest-path src-tauri/Cargo.toml --locked
~~~

From frontend/:

~~~bash
bun install --frozen-lockfile
bun run lint
bun run format:check
bun run --bun typecheck
bun run --bun test
bun run --bun build
~~~

The CI workflow also builds the macOS app bundle.

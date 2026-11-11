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

## Build a DMG

~~~bash
cd frontend
bun run tauri:build
~~~

Without Apple credentials this produces an unsigned local DMG. The release
workflow requires an Apple Developer signing certificate and notarization
credentials before it uploads a public installer. See the
[installation and release guide](docs/getting-started/installation.md).

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

The CI workflow also runs bun run tauri:build on macOS to produce the DMG.

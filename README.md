# DevTools

[日本語版 README](docs/ja/README.md)

A desktop collection of developer utilities. The application uses Tauri 2, a
Rust backend, and a React/TypeScript frontend. macOS supplies WebKit.

## Features

- QR code generation
- Image editing and splitting
- JSON, YAML, and TOML conversion
- HTTP requests
- Markdown preview and export
- Command generation
- SQLite, MySQL, and PostgreSQL tools
- Phrase templates, regex testing, timestamps, and text diff

## Requirements

- macOS 15.0 or later on Apple Silicon (arm64)
- Xcode Command Line Tools
- Rust 1.99.1 with rustfmt and clippy
- Bun 1.3.14

The app build no longer uses Qt, CMake, the vcpkg package manager,
QWebChannel, an FFI bridge, or sidecar processes.

## Quick Start

~~~bash
git clone https://github.com/LotusAndCompany/devtools.git
cd devtools/frontend
bun install --frozen-lockfile
bun run tauri:dev
~~~

Build a local DMG with:

~~~bash
bun run tauri:build
~~~

For installation, release signing, and update guidance, see the
[installation guide](docs/getting-started/installation.md). For contributor
build and test commands, see [BUILD.md](BUILD.md).

## Documentation

- [Documentation Index](docs/README.md)
- [Quick Start](docs/getting-started/quick-start.md)
- [Installation](docs/getting-started/installation.md)
- [User Guide](docs/user-guide/overview.md)
- [Development Architecture](docs/development/architecture.md)
- [Contributing](CONTRIBUTING.md)
- [Migration completion report](docs/migration/completion-report.md)
- [Rust API documentation](https://LotusAndCompany.github.io/devtools/)

## License

DevTools is licensed under Apache License 2.0; see [LICENSE](LICENSE). Rust and
frontend dependencies are recorded in src-tauri/Cargo.lock and frontend/bun.lock.
The release maintainer reviews those dependency licenses before publishing;
the M6 inventory and verification status are in the
[migration completion report](docs/migration/completion-report.md).

---

Made by [Lotus&Company Inc.](https://lotusandcompanyinc.com/)

# ビルド手順

[English version](../../BUILD.md)

## 必要環境

- macOS 15.0以降、Apple Silicon（arm64）
- Xcode Command Line Tools
- Rust 1.99.1（rustfmt、clippyを含む）
- Bun 1.3.14

ネイティブホストはTauri 2とRustで実装されています。WebKitはmacOSが提供します。
Qt、CMake、vcpkg、別途インストールするWebランタイムは不要です。

## 開発

~~~bash
git clone https://github.com/LotusAndCompany/devtools.git
cd devtools/frontend
bun install --frozen-lockfile
bun run tauri:dev
~~~

## ローカルmacOSアプリバンドルのビルド

~~~bash
cd frontend
bun run tauri:build
~~~

ローカルに `DevTools.app` バンドルが作られます。TestFlightまたはMac App Store向けの
ビルドは、共有Xcode Archiveスキームでアーカイブし、App Store Connectへアップロードします。
手順は[macos/README.md](../../macos/README.md)を参照してください。このローカルビルドは
アプリをアップロード・公開しません。

## テストとチェック

リポジトリのルートで実行します。

~~~bash
cargo fmt --manifest-path src-tauri/Cargo.toml --all -- --check
cargo clippy --manifest-path src-tauri/Cargo.toml --locked --all-targets -- -D warnings
cargo test --manifest-path src-tauri/Cargo.toml --locked
~~~

frontend/で実行します。

~~~bash
bun install --frozen-lockfile
bun run lint
bun run format:check
bun run --bun typecheck
bun run --bun test
bun run --bun build
~~~

CIではこれらに加え、macOSアプリバンドルをビルドします。

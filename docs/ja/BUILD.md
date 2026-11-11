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

## DMGのビルド

~~~bash
cd frontend
bun run tauri:build
~~~

Appleの認証情報を設定していない場合は、署名なしのローカルDMGが作られます。
公開インストーラーは署名証明書と公証用認証情報をRelease workflowに設定してから作成します。
詳細は[インストール・リリースガイド](../getting-started/installation.md)を参照してください。

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

CIではこれらに加え、macOS上で bun run tauri:build を実行してDMGを作ります。

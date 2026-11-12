# DevTools

[English README](../../README.md)

開発者向けのデスクトップツール集です。Tauri 2、Rust、shadcn/ui と
Base UI を使った React/TypeScript UI で構成し、WebKit は macOS が提供します。

## 機能

- QRコード生成
- 画像の編集・分割
- JSON、YAML、TOMLの変換
- HTTPリクエスト
- Markdownプレビューとエクスポート
- コマンド生成
- SQLite、MySQL、PostgreSQLの操作
- フレーズテンプレート、正規表現、タイムスタンプ、テキスト差分

## 必要環境

- macOS 15.0以降、Apple Silicon（arm64）
- Xcode Command Line Tools
- Rust 1.99.1（rustfmt、clippyを含む）
- Bun 1.3.14

Qt、CMake、vcpkg、QWebChannel、FFI、Sidecarには依存しません。

## クイックスタート

~~~bash
git clone https://github.com/LotusAndCompany/devtools.git
cd devtools/frontend
bun install --frozen-lockfile
bun run tauri:dev
~~~

ローカルのmacOSアプリバンドルを作るには次を実行します。

~~~bash
bun run tauri:build
~~~

公開版のインストールと更新は[Mac App Store](https://apps.apple.com/jp/app/devtools/id6736692957?mt=12)から行えます。
Xcode ArchiveとApp Store Connectへのアップロード手順は[macos/README.md](../../macos/README.md)、
開発者向けビルド・テスト手順は[BUILD.md](BUILD.md)を参照してください。

## ドキュメント

- [ドキュメント一覧](../README.md)
- [ユーザーガイド](../user-guide/overview.md)
- [開発アーキテクチャ](../development/architecture.md)
- [貢献ガイド](CONTRIBUTING.md)
- [移行完了レポート](../migration/completion-report.md)

## ライセンス

DevToolsのソースコードはApache License 2.0です。Rustとフロントエンドの依存関係は
それぞれのlockfileで管理します。公開前にRelease担当者が依存ライセンスを確認します。
M6の確認状況は[移行完了レポート](../migration/completion-report.md)を参照してください。

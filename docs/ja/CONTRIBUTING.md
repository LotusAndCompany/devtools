# DevToolsへの貢献

[English version](../../CONTRIBUTING.md)

## はじめに

[行動規範](../../CODE_OF_CONDUCT.md)、[ビルド手順](../../BUILD.md)、
[現在のアーキテクチャ](../development/architecture.md)を確認してください。
対応環境はApple Silicon搭載のmacOS 15以降です。Tauri 2、Rust、React、
TypeScript、Bunを使用します。

## 開発

リポジトリのルートから実行します。

~~~sh
cd frontend
bun install --frozen-lockfile
bun run tauri:dev
~~~

プルリクエストの前に[ビルド手順](../../BUILD.md)の確認を実行してください。
設定や移行を変更する場合は、新規起動、旧データの取り込み、失敗、再試行、
元データの保持を確認します。テストやfixtureに実ユーザーデータや秘密情報を
含めないでください。

## コードとドキュメント

- ネイティブ処理は `src-tauri/`、画面処理は `frontend/src/` に置きます。
- ブリッジ操作を変更した場合はRustの登録表、TypeScriptの共通契約、
  対応関係の確認を更新します。
- 画面に表示する文言は共通のローカライズ関数を使い、英語と日本語を用意します。
- 英語版に対応する日本語版がある場合は、両方を更新します。
- Qt、CMake、vcpkg、WKWebView用アダプター、sidecarを追加しないでください。
- 生成物や同梱ベンダーコードを直接編集しないでください。
- `.pen` ファイルはPencilエディターで扱い、テキストとして編集しないでください。

## プルリクエスト

目的に合うベースブランチから作業ブランチを作成し、利用者への変更、確認結果、
未確認事項を記載してください。Conventional Commitとrelease-pleaseの規約に従います。
コミット、プルリクエスト、IssueにAIツールの著者表記を加えないでください。

## リリース

公開用macOSインストーラーはリリースワークフローで作成し、組織のDeveloper IDで
署名・公証して検証してから公開します。詳細は
[インストールとリリース手順](../getting-started/installation.md)と
[リリース条件](../migration/completion-report.md#release-gate)を参照してください。

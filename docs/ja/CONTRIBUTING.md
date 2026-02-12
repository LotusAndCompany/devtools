# DevToolsへの貢献

[English version](../../CONTRIBUTING.md)

DevToolsへの貢献に興味を持っていただきありがとうございます！このドキュメントでは、貢献のためのガイドラインと手順を説明します。

## 目次

- [行動規範](#行動規範)
- [はじめに](#はじめに)
- [貢献の方法](#貢献の方法)
- [開発ガイドライン](#開発ガイドライン)
- [プルリクエストのプロセス](#プルリクエストのプロセス)
- [リリースプロセス](#リリースプロセス)

## 行動規範

このプロジェクトは[Contributor Covenant行動規範](CODE_OF_CONDUCT.md)に従います。参加することで、この規範を遵守することが期待されます。

## はじめに

### 前提条件

貢献する前に、以下を確認してください：

1. [README](README.md)を読む
2. [BUILD.md](BUILD.md)に従って開発環境をセットアップ
3. プロジェクト構造を理解する

### プロジェクト構造

```
devtools/
├── core/           # ビジネスロジックとアルゴリズム
├── gui/            # GUIコンポーネントとツールインターフェース
├── main/           # アプリケーションエントリーポイント
├── res/            # リソースと翻訳ファイル
├── tests/          # ユニットテスト
├── docs/           # ドキュメント
└── distribution/   # プラットフォーム固有のパッケージングファイル
```

## 貢献の方法

### バグの報告

1. [Issues](https://github.com/LotusAndCompany/devtools/issues)で既に報告されていないか確認
2. 報告されていない場合、バグ報告テンプレートを使用して新しいIssueを作成
3. 以下を含めてください：
   - バグの明確な説明
   - 再現手順
   - 期待される動作と実際の動作
   - 環境情報（macOSバージョン、Qtバージョンなど）
   - 該当する場合はスクリーンショット

### 機能のリクエスト

1. 既存のIssueで類似の機能リクエストがないか確認
2. 機能リクエストテンプレートを使用して新しいIssueを作成
3. 以下を記述してください：
   - 解決しようとしている問題
   - 提案する解決策
   - 検討した代替案

### コードの提出

1. リポジトリをフォーク
2. `main`から機能ブランチを作成（[ブランチ命名規則](#ブランチ命名規則)を参照）
3. 変更を加える
4. 必要に応じてテストを作成または更新
5. すべてのテストが通ることを確認
6. プルリクエストを提出

## 開発ガイドライン

### コードスタイル

- C++17の機能を適切に使用
- プロジェクト内の既存のコードパターンに従う
- 意味のある変数名と関数名を使用
- 複雑なロジックにはコメントを追加

### コードフォーマット

このプロジェクトでは、自動コードフォーマットに`clang-format`、静的解析に`clang-tidy`を使用しています。

#### セットアップ

ツールをインストール（macOS）：
```bash
brew install llvm
```

#### 使用方法

すべてのソースファイルをフォーマット：
```bash
cmake --build build --target format
```

変更なしでフォーマットをチェック：
```bash
cmake --build build --target format-check
```

静的解析を実行：
```bash
cmake --build build --target lint
```

自動修正付きで静的解析を実行：
```bash
cmake --build build --target lint-fix
```

#### Gitフック (pre-commit)

このプロジェクトでは[pre-commit](https://pre-commit.com/)を使用してコード品質チェックを自動化しています。

**インストール：**

```bash
# pre-commit をインストール
brew install pre-commit  # macOS
# または: pip install pre-commit

# フックをセットアップ
pre-commit install --install-hooks -t pre-commit -t commit-msg -t pre-push
```

**フック一覧：**

| ステージ | フック | 説明 |
|---------|--------|------|
| pre-commit | clang-format | C++コードのフォーマット |
| pre-commit | trailing-whitespace | 末尾空白の削除 |
| pre-commit | end-of-file-fixer | ファイル末尾の改行 |
| pre-commit | check-added-large-files | 大きなファイルの追加を防止 |
| commit-msg | conventional-pre-commit | コミットメッセージ形式の検証 |
| pre-push | cmake-build | ビルド成功の検証 |

**手動実行：**

```bash
# 全ファイルに対してすべてのフックを実行
pre-commit run --all-files

# 特定のフックを実行
pre-commit run clang-format --all-files

# 一時的にフックをスキップ
git commit --no-verify -m "WIP: work in progress"
git push --no-verify
```

**フックの更新：**

```bash
pre-commit autoupdate
```

#### CI (GitHub Actions)

コード品質チェックは以下のタイミングで自動実行されます：
- `main` への**プルリクエスト**時

| チェック | 実行環境 | 説明 |
|---------|---------|------|
| clang-format | macOS | 変更ファイルのフォーマットチェック |
| clang-tidy | Ubuntu | 変更ファイルの静的解析 |
| format-suggestion | macOS | PRにフォーマット修正提案をコメント |
| semantic-pr | Ubuntu | PRタイトルがConventional Commits形式に従っているか検証 |

#### IDE連携

- **Qt Creator**: 設定 > C++ > コードスタイル > `.clang-format`をインポート
- **VS Code**: 「clangd」拡張機能をインストール、`.clang-format`が自動的に使用されます
- **CLion**: 設定 > エディタ > コードスタイル > C/C++ > ClangFormatを有効化

### 命名規則

| タイプ | 規則 | 例 |
|-------|------|-----|
| クラス | PascalCase | `ImageResize` |
| 関数 | camelCase | `processImage()` |
| 変数 | snake_case | `image_width` |
| 定数 | SCREAMING_SNAKE_CASE | `MAX_IMAGE_SIZE` |
| ファイル | snake_case | `image_resize.cpp` |

### 新しいモジュールの追加

1. `CMakeLists.txt`に新しい静的ライブラリを作成：
   ```cmake
   qt_add_library(${PROJECT_NAME}_your_module STATIC
       core/your_module/your_module.h
       core/your_module/your_module.cpp
       gui/your_module/your_module_gui.h
       gui/your_module/your_module_gui.cpp
       gui/your_module/your_module_gui.ui
   )
   ```

2. 必要に応じて依存関係を追加：
   ```cmake
   add_dependencies(${PROJECT_NAME}_your_module ${PROJECT_NAME}_core)
   ```

3. `MODULE_LIST`に追加：
   ```cmake
   set(MODULE_LIST
       # ... 既存のモジュール
       ${PROJECT_NAME}_your_module
   )
   ```

### テストの追加

`tests/DevToolsTests.cmake`にテストを追加：
```cmake
DevTools_add_test(test_your_module
    SOURCES
    tests/core/your_module/test_your_module.cpp
)
```

`cmake .. -DENABLE_UNIT_TEST=ON`でテストを有効化します。

### ブランチ命名規則

以下のブランチ命名規則を使用してください：

| タイプ | パターン | 例 |
|-------|---------|-----|
| 機能追加 | `feature/<説明>` | `feature/sms-qr-code` |
| バグ修正 | `fix/<説明>` | `fix/image-rotation-angle` |
| ドキュメント | `doc/<説明>` | `doc/update-build-guide` |
| リファクタリング | `refactor/<説明>` | `refactor/data-conversion` |
| 緊急修正 | `hotfix/<説明>` | `hotfix/crash-on-startup` |

**ルール：**
- 常に`main`からブランチを作成
- 小文字とハイフンを使用（アンダースコアやスペースは使わない）
- 説明は短く、しかし分かりやすく
- 該当する場合はIssue番号を含める：`feature/123-sms-qr-code`

```bash
# 例：機能ブランチの作成
git checkout main
git pull origin main
git checkout -b feature/sms-qr-code
```

### コミットメッセージ

[Conventional Commits](https://www.conventionalcommits.org/)形式に従ってください：

```
<type>(<scope>): <subject>

<body>

<footer>
```

**タイプ一覧：**
| タイプ | 説明 |
|-------|------|
| `feat` | 新機能 |
| `fix` | バグ修正 |
| `docs` | ドキュメントの変更 |
| `style` | コードスタイルの変更（フォーマットなど、ロジック変更なし） |
| `refactor` | リファクタリング |
| `test` | テストの追加・更新 |
| `chore` | メンテナンスタスク |
| `build` | ビルドシステムの変更 |
| `ci` | CI設定の変更 |
| `perf` | パフォーマンス改善 |

**ルール：**
- 現在形を使用（「機能を追加した」ではなく「機能を追加」）
- 命令形を使用
- 件名は簡潔に（50文字以内推奨）
- 本文は72文字以内で折り返し推奨
- フッターでIssueやPRを参照

**例：**
```
feat(qr): 電話番号用QRコード生成を追加

- tel: URIスキームのサポートを実装
- 電話番号バリデーションを追加
- 電話番号入力フィールドでUIを更新

Closes #123
```

```
fix(image): 回転角度の計算を修正

特定の画像形式で回転が90度ずれていた問題を修正。
すべてのサポート形式で一貫した動作を保証。

Fixes #456
```

```
docs: macOS用ビルド手順を更新
```

## プルリクエストのプロセス

1. 必要に応じて**ドキュメントを更新**
2. 新機能の**テストを作成**
3. **CIがパス**することを確認（すべてのテストがパス、ビルドエラーなし）
4. メンテナーに**レビューをリクエスト**
5. **フィードバックに迅速に対応**
6. 要求された場合は**コミットをスカッシュ**

### PRタイトルのフォーマット

[Conventional Commits](https://www.conventionalcommits.org/)形式に従った明確で説明的なタイトルを使用してください。
タイトルはCIで自動検証されます。

**ルール:**
- subject（コロン以降）は小文字で始める（英語の場合）
- 現在形を使用

**例:**
- `feat: add SMS QR code generation`
- `fix: correct image rotation angle calculation`
- `docs: update build instructions for macOS`
- `refactor: simplify data conversion logic`

### レビュープロセス

1. メンテナーがPRをレビュー
2. 変更のリクエストや質問がある場合があります
3. 承認されると、PRがマージされます
4. あなたの貢献はChangelogで acknowledgement されます

## リリースプロセス

このプロジェクトでは[release-please](https://github.com/googleapis/release-please)を使用してリリース管理を自動化しています。

### 仕組み

1. **Conventional Commitsによるバージョン決定：**
   - `fix:` コミット → PATCHバージョン (0.3.0 → 0.3.1)
   - `feat:` コミット → MINORバージョン (0.3.0 → 0.4.0)
   - フッターの`BREAKING CHANGE:` → MAJORバージョン (0.3.0 → 1.0.0)

2. **自動化ワークフロー：**
   - `main`に変更がマージされると、release-pleaseがRelease PRを作成/更新
   - Release PRには以下が含まれます：
     - 更新された`CHANGELOG.md`
     - `CMakeLists.txt`と`vcpkg.json`のバージョン更新
   - Release PRがマージされると、GitHubリリースとタグが自動作成

3. **release-pleaseが管理するファイル：**
   - `CMakeLists.txt` - プロジェクトバージョン
   - `vcpkg.json` - 依存関係バージョン
   - `version.txt` - プレーンテキストバージョン
   - `CHANGELOG.md` - リリースノート

### メンテナー向け

- リリース準備ができたら、自動生成されたRelease PRをレビューしてマージ
- バージョン番号やCHANGELOGを手動で編集しないでください（release-pleaseが処理します）
- 正確なバージョン決定のため、コミットメッセージはConventional Commits形式に従うこと

## 質問がありますか？

質問がある場合は、お気軽に：
- ディスカッション用のIssueを開く
- メンテナーに連絡する

DevToolsへの貢献ありがとうございます！

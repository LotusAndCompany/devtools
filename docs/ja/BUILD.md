# ビルド手順

[English version](../../BUILD.md)

このドキュメントでは、DevToolsをソースからビルドする方法を説明します。

## 必要要件

### プラットフォーム
- **macOS 15.0以上**（Apple Silicon / arm64専用）

### ツール
- **CMake**: 3.21.1以上
- **C++コンパイラ**: C++17対応（Clang推奨）
- **Qt**: 6.9以上（6.9.3で検証済み。Core、Gui、Widgets、LinguistTools、Sql、
  Network、Svg、Concurrent、WebChannel、WebEngineWidgetsモジュール）
- **vcpkg**: 依存関係管理用
- **Bun**: 1.3.14（Reactフロントエンドのビルドに必要）

### 依存ライブラリ（vcpkg経由）
- toml11
- yaml-cpp

### 依存ライブラリ（CMake FetchContent経由）
- qlementine v1.4.2（Qt Widgets向けのモダンな `QStyle`、MITライセンス）

qlementineはCMake構成時に `https://github.com/oclero/qlementine.git` から取得されます。
Qt 6.8以上とCMake 3.21以上が必要です。DevToolsではQt Widgetsのサポートライブラリにリンクします。
アプリのメイン画面はReactで構成し、ViteでバンドルしてQt WebEngineに表示します。

### オプション
- **Doxygen**: 1.18以上（APIドキュメント生成用）
- **GraphViz**: クラス図・依存関係グラフの生成用（オプション）
- **Qt Creator**: 18.0.1以上（Qt統合IDE）
- **Ninja**: 1.12.1以上（高速ビルドツール）
- **Qt Installer Framework**: 4.10（アプリケーションパッケージング）

## インストール

### 1. Qt6のインストール

#### Homebrewを使用する場合（macOSで推奨）
```bash
brew install qt@6
```

#### Qt Online Installerを使用する場合
[qt.io](https://www.qt.io/download)からダウンロードし、以下のコンポーネントを含むQt 6.9以上をインストール：
- Qt 6.9以上 for macOS
- Core、Gui、Widgets、LinguistTools、Sql、Network、Svg、Concurrent、WebChannel、WebEngineWidgetsモジュール
- Qt Creator（オプション、推奨）

### 2. vcpkgのインストール

```bash
# vcpkgをクローン
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# ブートストラップ
./bootstrap-vcpkg.sh

# vcpkgのルートを設定してPATHに追加
export VCPKG_ROOT="$PWD"
export PATH="$VCPKG_ROOT:$PATH"
```

### 3. リポジトリをクローン

```bash
git clone https://github.com/LotusAndCompany/devtools.git
cd devtools
```

### 4. Bunのインストール

[Bun公式ガイド](https://bun.sh/docs/installation)に従ってBun 1.3.14以降をインストールし、
バージョンを確認します。

```bash
bun --version
```

### 5. 依存関係のインストール

```bash
vcpkg install
```

## ビルド

### CMakeを使用する場合（コマンドライン）

```bash
# ビルドディレクトリを作成
mkdir build && cd build

# 構成（vcpkgの依存関係とqlementineを取得します）
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=arm64-osx

# ビルド
cmake --build . --parallel
```

### アプリケーションの実行

ビルド後、アプリケーションを起動します：

```bash
# ビルドと実行（カラー付きコンソール出力）
cmake --build . --target run

# またはバイナリを直接実行
./DevTools.app/Contents/MacOS/DevTools

# またはApp Bundleを開く
open DevTools.app
```

`run`ターゲットは、このソースツリーから起動した古いDevToolsプロセスを置き換えます。
別途インストールしたDevToolsが起動中の場合は、先に終了してください。対象を誤って選ぶのを避けるため、
そのプロセスが残っているとターゲットはエラーで停止します。

ログ出力は重要度に応じて色分けされます：DEBUG=灰色, INFO=デフォルト, WARN=黄色, CRIT=赤。

### Qt Creatorを使用する場合

1. Qt Creatorで `CMakeLists.txt` を開く
2. Qt Creatorの設定でvcpkgプラグインを有効にする
3. 以下のCMake引数を追加：
   ```
   -DVCPKG_TARGET_TRIPLET=arm64-osx
   ```
4. プロジェクトを構成
5. ビルドボタンまたは `Cmd+B` でビルド

### ビルドオプション

| オプション | デフォルト | 説明 |
|-----------|-----------|------|
| `ENABLE_UNIT_TEST` | OFF | ユニットテストを有効にする |
| `VCPKG_TARGET_TRIPLET` | auto | Apple Siliconの場合は `arm64-osx` を設定 |

qlementineのサンプルアプリケーションはプロジェクト設定で無効化しています
（`QLEMENTINE_SANDBOX=OFF`, `QLEMENTINE_SHOWCASE=OFF`）。ビルド対象はスタイルライブラリのみです。

オプション指定の例：
```bash
cmake .. -DENABLE_UNIT_TEST=ON -DVCPKG_TARGET_TRIPLET=arm64-osx
```

## テストの実行

```bash
# まずテストを有効化
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=arm64-osx \
  -DENABLE_UNIT_TEST=ON

# ビルド
cmake --build . --parallel

# テスト実行
ctest
```

## ドキュメントの生成

APIドキュメントの生成にはDoxygen 1.18以上が必要です。

### Doxygenのインストール
```bash
brew install doxygen
```

### GraphVizのインストール（オプション）

GraphVizをインストールすると、クラス継承図・コラボレーション図・インクルード依存図が生成されます。

```bash
brew install graphviz
```

GraphVizがインストールされていない場合、図表なしでドキュメントが生成されます。

### ドキュメントの生成
```bash
cmake --build build --target DevTools_docs
```

生成されたドキュメントは `build/doxygen/html/` に出力されます。

## トラブルシューティング

### CMake構成エラー

1. **構文エラーの確認**: CMakeLists.txtを確認
2. **CMakeキャッシュの削除**:
   ```bash
   rm -rf build/CMakeCache.txt
   ```
3. **ビルドディレクトリのクリーン**:
   ```bash
   rm -rf build
   cmake -S . -B build \
     -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
     -DVCPKG_TARGET_TRIPLET=arm64-osx
   ```

### vcpkgの問題

1. **vcpkgが正しくインストールされているか確認**:
   ```bash
   vcpkg version
   ```
2. **vcpkgの更新**:
   ```bash
   cd /path/to/vcpkg
   git pull
   ./bootstrap-vcpkg.sh
   ```

### Qtが見つからない場合

1. **Qt6_DIR環境変数を設定**:
   ```bash
   export Qt6_DIR=/opt/homebrew/lib/cmake/Qt6
   ```
2. **またはCMakeに渡す**:
   ```bash
   cmake .. -DQt6_DIR=/opt/homebrew/lib/cmake/Qt6
   ```

### qlementineのFetchContentで失敗する場合

qlementineのダウンロード中にCMake構成が失敗する場合は、以下を確認してください。

1. `https://github.com/oclero/qlementine.git` へアクセスできることを確認する。
2. FetchContentのキャッシュを削除して再構成する。
   ```bash
   rm -rf build/_deps/qlementine-*
   cmake -S . -B build \
     -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
     -DVCPKG_TARGET_TRIPLET=arm64-osx
   ```
3. 使用中のQtが6.8以上で、Svgモジュールを含んでいることを確認する。

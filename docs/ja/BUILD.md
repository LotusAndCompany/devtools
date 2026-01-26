# ビルド手順

[English version](../../BUILD.md)

このドキュメントでは、DevToolsをソースからビルドする方法を説明します。

## 必要要件

### プラットフォーム
- **macOS 15.0以上**（Apple Silicon / arm64専用）

### ツール
- **CMake**: 3.21.1以上
- **C++コンパイラ**: C++17対応（Clang推奨）
- **Qt**: 6.9.3（Widgets, LinguistTools, Networkモジュール）
- **vcpkg**: 依存関係管理用

### 依存ライブラリ（vcpkg経由）
- toml11
- yaml-cpp

### オプション
- **Doxygen**: APIドキュメント生成用
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
[qt.io](https://www.qt.io/download)からダウンロードし、以下のコンポーネントを含むQt 6.xをインストール：
- Qt 6.x for macOS
- Qt Creator（オプション、推奨）

### 2. vcpkgのインストール

```bash
# vcpkgをクローン
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# ブートストラップ
./bootstrap-vcpkg.sh

# PATHに追加（永続化するにはシェルプロファイルに追加）
export PATH="$PWD:$PATH"
```

### 3. リポジトリをクローン

```bash
git clone https://github.com/LotusAndCompany/devtools.git
cd devtools
```

### 4. 依存関係のインストール

```bash
vcpkg install
```

## ビルド

### CMakeを使用する場合（コマンドライン）

```bash
# ビルドディレクトリを作成
mkdir build && cd build

# 構成
cmake .. -DVCPKG_TARGET_TRIPLET=arm64-osx

# ビルド
make

# または並列ジョブで
make -j$(sysctl -n hw.ncpu)
```

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

オプション指定の例：
```bash
cmake .. -DENABLE_UNIT_TEST=ON -DVCPKG_TARGET_TRIPLET=arm64-osx
```

## テストの実行

```bash
# まずテストを有効化
cmake .. -DENABLE_UNIT_TEST=ON

# ビルド
make

# テスト実行
ctest
```

## ドキュメントの生成

APIドキュメントの生成にはDoxygenが必要です。

### Doxygenのインストール
```bash
brew install doxygen
```

### ドキュメントの生成
```bash
cd build
make DevTools_docs
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
   rm -rf build && mkdir build && cd build && cmake ..
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

## 配布

App Store用の配布ビルドの作成については、[docs/distribution/how_to_distribute.md](../distribution/how_to_distribute.md)を参照してください。

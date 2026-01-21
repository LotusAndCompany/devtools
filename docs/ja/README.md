# DevTools

[English README](../../README.md)

開発の際に使う様々なツールを一つのアプリケーションにまとめたデスクトップアプリケーションです。今まで適当なWebサービスを使っていた簡単な作業を、一つのツールで完結できます。

<!-- TODO: スクリーンショットを追加 -->
<!-- ![DevTools スクリーンショット](docs/images/screenshot.png) -->

## 機能一覧

### QRコード生成
- **テキスト**: 任意のテキストからQRコードを生成
- **URL**: WebサイトのリンクをQRコード化
- **メール**: mailto: 形式のQRコードを生成
- **電話番号**: tel: 形式のQRコードを生成
- **SMS**: SMS送信用のQRコードを生成
- **位置情報**: 地理座標のQRコードを生成

### 画像処理
- **リサイズ**: 画像を指定サイズに拡大・縮小
- **回転**: 任意の角度で画像を回転
- **分割**: 画像を複数のパーツに分割
- **透過**: 特定の色を透明化

### データ変換
- **JSON**、**YAML**、**TOML** 形式の相互変換
- 構文検証とフォーマット機能

### APIテスト
- HTTPリクエストの送信（GET, POST, PUT, DELETE など）
- シンタックスハイライト付きのレスポンス表示

### コマンド実行
- GUIからシェルコマンドを実行
- リアルタイムで出力を確認

### フレーズ生成
- ランダムなフレーズやテキストを生成

## 必要環境

- **OS**: macOS 15.0以上（Apple Silicon / arm64 専用）
- **Qt**: 6.x
- **CMake**: 3.21.1以上
- **C++ コンパイラ**: C++17 対応

## クイックスタート

```bash
# リポジトリをクローン
git clone https://github.com/LotusAndCompany/devtools.git
cd devtools

# vcpkgで依存関係をインストール
vcpkg install

# CMakeで構成
mkdir build && cd build
cmake ..

# ビルド
make
```

詳細なビルド手順は [BUILD.md](BUILD.md) を参照してください。

## ドキュメント

- [ビルド手順](BUILD.md) / [Build Instructions](../../BUILD.md)
- [貢献ガイド](CONTRIBUTING.md) / [Contributing Guide](../../CONTRIBUTING.md)
- [APIドキュメント](https://LotusAndCompany.github.io/devtools/)（Doxygen）

## 開発

### 新しいコードを追加する

コンパイル時間の短縮のため、複数の静的ライブラリに分割してコンパイルしています。

以下のように、追加するコード群を `qt_add_library` で追加します。
名前はそれと分かれば何でも良いです。
```cmake
qt_add_library(${PROJECT_NAME}_image_resize STATIC
    core/image/resize/image_resize.h core/image/resize/image_resize.cpp
    gui/image/resize/image_resize_gui.h gui/image/resize/image_resize_gui.cpp gui/image/resize/image_resize_gui.ui
)
```

これらのモジュールに依存関係がある場合は依存関係を定義してください。
例えば画像編集系のコードは以下のように依存関係を定義しています。
```cmake
# add_dependencies(依存元 依存先)
# ${PROJECT_NAME}_image_coreは画像処理系ツールの共通部分
add_dependencies(${PROJECT_NAME}_image_resize ${PROJECT_NAME}_image_core)
add_dependencies(${PROJECT_NAME}_image_rotation ${PROJECT_NAME}_image_core)
add_dependencies(${PROJECT_NAME}_image_division ${PROJECT_NAME}_image_core)
add_dependencies(${PROJECT_NAME}_image_transparent ${PROJECT_NAME}_image_core)
```

下の方にあるモジュール一覧に追加します。
```cmake
# [モジュール一覧]
set(MODULE_LIST
    ${PROJECT_NAME}_core
    ${PROJECT_NAME}_image_core
    ${PROJECT_NAME}_image_resize
    ${PROJECT_NAME}_image_rotation
    ${PROJECT_NAME}_image_division
    ${PROJECT_NAME}_image_transparent
)
```

### テストを追加する

`tests/DevToolsTests.cmake` に以下のように追加します。
```cmake
DevTools_add_test(test_basic_image_io   # テスト名
    SOURCES
    tests/core/image/test_basic_image_io.cpp    # テストコード
)
```

デフォルトではテストはビルドしない設定になっています。
CMakeの設定で `ENABLE_UNIT_TEST` をONにするとテストができるようになります。
ただし、デバッグ実行時などにもテストコードがビルドされるのでコンパイル時間が伸びます。

### 外部ライブラリを追加する

**手間が掛かります。可能な限りQtで用意されている機能を使いましょう。**

Qt以外の外部ライブラリを利用する場合は[vcpkg](https://vcpkg.io/)を使います。
Qt Creatorの方でプラグインを有効にしてください。
macOSではなぜかarm64と判定されないのでCMakeの引数に以下を追加してください。
`-DVCPKG_TARGET_TRIPLET=arm64-osx`

また、小規模のものであれば `git submodule` で追加しても構いません。
submoduleの場合は普通にコードを追加すれば動くはずです。

vcpkgを使う場合は、
```cmake
# ここをOFFする
set(QT_CREATOR_SKIP_PACKAGE_MANAGER_SETUP OFF)

(中略)

# 使いたいパッケージをfind_packageで探す
find_package(toml11 CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
```
更に、インクルードディレクトリとライブラリへのリンクを定義してください。
```cmake
# インクルードディレクトリ
target_include_directories(${PROJECT_NAME}_data_conversion PRIVATE
    ${TOML11_INCLUDE_DIR}
    ${YAML_CPP_INCLUDE_DIR}
)
# リンク
target_link_libraries(${PROJECT_NAME}_data_conversion PRIVATE
    ${YAML_CPP_LIBRARIES}
)
```

## 貢献

貢献を歓迎します！詳細は[貢献ガイド](CONTRIBUTING.md)をご覧ください。

- バグの報告方法
- 機能リクエストの方法
- プルリクエストの提出方法
- コーディングスタイルのガイドライン

## セキュリティ

セキュリティに関する脆弱性については、[セキュリティポリシー](SECURITY.md)をご覧ください。

## ライセンス

このプロジェクトはApache License 2.0の下で公開されています。詳細は[LICENSE](../../LICENSE)ファイルをご覧ください。

### サードパーティライセンス

DevToolsは以下のサードパーティライブラリを使用しています：

| ライブラリ | ライセンス |
|-----------|-----------|
| Qt6 | LGPL v3 |
| toml11 | MIT |
| yaml-cpp | MIT |
| qrcodegen | MIT |
| Material Icons | Apache 2.0 |
| Remix Icon | Apache 2.0 |

## トラブルシューティング

### CMakeでエラーが出たら
- 構文エラーなどの確認
- `build/Qt_*/CMakeCache.txt` を消してみる
- `build/Qt_*` フォルダを消す

## 謝辞

- [Qt Project](https://www.qt.io/) - 優れたクロスプラットフォームフレームワーク
- [Project Nayuki](https://www.nayuki.io/) - QRコード生成ライブラリ
- DevToolsのすべてのコントリビューターとユーザーの皆様

---

Made with by [Lotus&Company Inc.](https://lotusandcompanyinc.com/)

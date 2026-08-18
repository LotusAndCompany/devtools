# DevTools

[English README](../../README.md)

DevToolsは、開発に使うさまざまなツールを、ひとつの使いやすいデスクトップアプリケーションにまとめたものです。複数のWebサービスを使い分けることなく、開発に必要なユーティリティをひとつのアプリケーションで利用できます。

<!-- TODO: スクリーンショットを追加 -->
<!-- ![DevToolsのスクリーンショット](docs/images/screenshot.png) -->

## 機能

### QRコード生成

- **テキスト**: 任意のテキストからQRコードを生成
- **URL**: WebサイトのリンクをQRコード化
- **メール**: mailto:形式のQRコードを生成
- **電話番号**: 電話番号用のtel:形式のQRコードを生成
- **SMS**: SMS送信用のQRコードを生成
- **位置情報**: 位置情報をQRコード化

### 画像処理

- **リサイズ**: 画像を指定したサイズに拡大・縮小
- **回転**: 画像を任意の角度で回転
- **分割**: 画像を複数のパーツに分割
- **透過**: 指定した色を透明化

### データ変換

- **JSON**、**YAML**、**TOML**形式を相互変換
- 構文検証とフォーマット

### APIテスト

- HTTPリクエスト（GET、POST、PUT、DELETEなど）を送信
- シンタックスハイライト付きでレスポンスを表示

### コマンド実行

- GUIからシェルコマンドを実行
- コマンドの出力をリアルタイムで表示

### データベース管理

- **SQLite**、**MySQL**、**PostgreSQL**に接続
- テーブルの閲覧とデータの確認
- タブ形式のインターフェースでSQLクエリを実行

### フレーズ生成

- ランダムなフレーズやテキストを生成

## 必要環境

- **OS**: macOS 15.0以降（Apple Silicon / arm64のみ）
- **Qt**: 6.9.3（qlementineにはQt 6.8以降が必要）
- **CMake**: 3.21.1以降
- **C++コンパイラ**: C++17対応

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

# ビルドして実行
cmake --build . --target run
```

詳しいビルド手順は[BUILD.md](BUILD.md)を参照してください。

## ドキュメント

### クイックリンク

- [ドキュメント一覧](../README.md)
- [クイックスタート](../getting-started/quick-start.md)
- [インストールガイド](../getting-started/installation.md)

### 詳細ドキュメント

- [ユーザーガイド](../user-guide/overview.md) - 各機能の使い方
- [開発者向けガイド](../development/architecture.md) - コントリビューター向け
- [APIドキュメント](https://LotusAndCompany.github.io/devtools/)（Doxygen）

### その他のリソース

- [ビルド手順](BUILD.md)
- [貢献ガイド](CONTRIBUTING.md)
- [トラブルシューティング](../troubleshooting/common-issues.md) / [FAQ](../troubleshooting/faq.md)

## 貢献

貢献を歓迎します。詳しくは[貢献ガイド](CONTRIBUTING.md)を参照してください。

- バグの報告方法
- 機能リクエストの方法
- プルリクエストの提出方法
- コーディングスタイルのガイドライン

## セキュリティ

セキュリティ上の脆弱性については、[セキュリティポリシー](SECURITY.md)を参照してください。

## ライセンス

このプロジェクトはApache License 2.0でライセンスされています。詳しくは[LICENSE](../../LICENSE)を参照してください。

### サードパーティライセンス

DevToolsは以下のサードパーティライブラリを使用しています。

| ライブラリ | ライセンス |
|-----------|-----------|
| Qt6 | LGPL v3 |
| toml11 | MIT |
| yaml-cpp | MIT |
| qlementine | MIT |
| qrcodegen | MIT |
| Material Icons | Apache 2.0 |
| Remix Icon | Apache 2.0 |

## 謝辞

- [Qt Project](https://www.qt.io/) - 優れたクロスプラットフォームフレームワーク
- [Qlementine](https://github.com/oclero/qlementine) - Qt Widgets向けのモダンなスタイル
- [Project Nayuki](https://www.nayuki.io/) - QRコード生成ライブラリ
- DevToolsのすべてのコントリビューターとユーザー

---

[Lotus&Company Inc.](https://lotusandcompanyinc.com/)が開発しています。

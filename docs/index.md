# DevTools documentation {#mainpage}
各クラスのドキュメンテーションです。テストコードは除いています。

ドキュメンテーションの生成のやり方
1. doxygenをインストール
```bash
brew install doxygen
```
2. cmakeでプロジェクトを構成(DebugかRelWithDebInfo)
3. ビルドフォルダに移動
```bash
cd BUILD_DIR
```
3. htmlを生成
```bash
make DevTools-docs
```

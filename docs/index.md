# DevTools documentation {#mainpage}
各クラスのドキュメンテーションを作成するつもりです。テストコードは除いています。  
`@file`が先頭に無いファイルは Files▼ の所には表示されません。  

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

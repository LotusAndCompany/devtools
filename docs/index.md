# DevTools documentation {#mainpage}
各クラスのドキュメンテーションを[Doxygen](https://www.doxygen.nl/)で作成するつもりです。テストコードは除いています。  
`@file`が先頭に無いファイルは Files▼ の所には表示されません。  

## Doxygen usage
コマンドの説明など→ https://www.doxygen.nl/manual/commands.html  
数式は[MathJax](https://www.mathjax.org/)で表示させています。
[LaTex](https://www.latex-project.org/)の記法を採用していますが、他の書き方もできると思います。

## Build
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
make DevTools_docs
```

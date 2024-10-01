# 配布できる形にする(旧)
[開発ノウハウ](https://drive.google.com/drive/folders/1LtCMztm9HaseApoA_vcSOMbkNlRQPgfB)を参考に
1. [TARGETS > DevTools > General] Minumum Deployment=13.5
2. [TARGETS > DevTools> General] App Category=Developer Tools, Bundle Identifier="com.lotusandcompany.dev-tools", Version/Build=0.0.1-test01
3. [TARGETS > DevTools > Signing & capabilities] Team, Signing Certificate設定済み
4. アイコン
    1. ローカルに作成スクリプトがある→~/Pictures/dev-tools_icon/gen.sh
    2. New File From TemplateからAsset Catalogを選択
    3. コンテキストメニュからmacOS > New macOS App Iconで作ってから各サイズの画像を追加
    4. AppIconのTarget Membershipから+を選び、対象のアプリケーションを追加する。  
       **Filterに何も入れないと何も表示されない**という信じ難い仕様のため注意。
    5. [TARGETS > DevTools> General] AppIconを設定
5. DevTools/Resources/Info.plist編集 (build/Qt_6_8_0_for_macOS-Release/CMakeFiles/DevTools.dir/Info.plist らしい)  
    App Category=Developer Tools
    Default localization=ja (配布時はenの方が良いかも)
6. [TARGETS > DevTools > Signing & capabilities] Capabilitiesの設定
    1. App Sanboxを追加し、ファイル読み各権限を追加
    2. Hardware Runtimeを追加し、Disable Library Validationを有効にする
7. Product > Arvhiveからビルドする。成功したら、
    1. Validate App実施。失敗したらそれまでの手順を見直す。
    2. Distribute AppからAppConnectを選択

# 配布できる形にする
[開発ノウハウ](https://drive.google.com/drive/folders/1LtCMztm9HaseApoA_vcSOMbkNlRQPgfB)を参考に
1. CMakeでGENERATORをXCodeに設定するこの時、以下の変数を加える
```
-DCMAKE_OSX_DEPLOYMENT_TARGET=12
-DCMAKE_OSX_ARCHITECTURES=arm64
```
2. ビルドフォルダ内にプロジェクトファイルが生成されるのでそれを開く
3. GeneralのFrameworks, Libraries and Embdded conetentsの所にQt系のフレームワークとAGLをドラッグ&ドロップし、AGLをNot Embeddedにする。他はEmbedded & Signedのままで良い。
4. ここで色々タブを切り替えるなどの操作をするとGeneral内にIdentityが出てくるので設定する (バグ?)
5. Arvhiveからビルドし、アップロード。


ドキュメントには書かれていませんが、Validate App実施前?までに[アプリ用パスワードを作成する](https://support.apple.com/ja-jp/102654)必要があるかも知れません。
共有ライブラリなどがデプロイできていない?

# コマンドラインツール再チャレンジ
```bash
cd build

# 簡略化
export B=~/Desktop/QtProjects/dev-tools/build/Qt_6_8_0_for_macOS-Debug

# デプロイ
~/Qt/6.8.0/macos/bin/macdeployqt $B/DevTools.app

# 署名
codesign \
    --verify \
    --sign "Apple Development: 有亮 横田 (LJ669DT448)" \
    --force \
    --verbose \
    --deep \
    --options runtime \
    --timestamp \
    --entitlements ../distribution/DevTools.entitlements \
    $B/DevTools.app
```

`otool -L <binary path>` で調べたところ、QWidgetsの依存関係は
QtWidgets→QtCore, QtGui
QtGui→QtCore, QtDBus
であったため、明示的にリンクするようにCMakeを修正

CMakeのジェネレーターツールをXCodeに設定し、プロジェクトを開く。これは場所を考えた方が良いかも知れない。
Assetsを↑に追加

Generalに設定:
Developer Tools
DevTools
13.0
バージョン
Qt系のライブラリをFrameworksからFrameworks, Librariesの所にドラッグ&ドロップして Embeded without signing に設定
AGLは同様にしてDo not embededにする

Infoに設定:
ja
Developper Tool
バージョン
CopyRight
Icon file削除

Signing&Capabilitiesに設定:
ファイルの読み書き権限
Disable Library Validation

BuildSettingsに追加:
BuildSettingでrpathに`@executable_path/../Frameworks`, `@loader_path/Frameworks`を追加

Product > Archive
完了したらValidation


# 動的リンクに再挑戦
普通にジェネレータをXCodeにして開く

Qt系のFrameworksをドラッグ&ドロップ
名前などを設定する。

BuildPhasesに以下を実行するフェーズを追加(`macdeployqt`の場所はバージョンに応じて変更)
~/Qt/6.8.0/macos/bin/macdeployqt $TARGET_BUILD_DIR/DevTools.app

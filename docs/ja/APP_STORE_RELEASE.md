# Mac App Store向けリリース手順

このガイドは、DevToolsをMac App Storeで配布するメンテナー向けです。ビルド環境は[ビルド手順](BUILD.md)を確認してください。初回アップロード時にアプリレコードが未作成の場合は[Appleの案内](https://developer.apple.com/help/app-store-connect/create-an-app-record/add-a-new-app/)を参照します。初回はPRからアップロードまで通読し、次回以降は作業中の段階と0.8.0の記録を参照してください。

最終確認: 2026-10-01

GitHub Releaseの作成はrelease-pleaseが行います。App Store向けのArchive、アップロード、審査提出は別の手順です。アップロード後の最終確認と審査提出はメンテナーが行います。

## PRをマージしてGitHubリリースを作る

1. 変更をPRにまとめ、Conventional Commits形式のタイトルを付けます。タイトルは小文字で始めます。例: `fix(db): restore access to external sqlite files in app sandbox`。
2. PRをレビューして`main`へマージします。release-pleaseはスカッシュコミットのタイトルからバージョンの更新幅を決め、Release PRを作成または更新します。
3. Release PRでバージョンとCHANGELOGを確認してマージします。release-pleaseがGitHub Releaseとタグを作成します。
4. App Store用ビルドは、レビュー済みソースのリリースタグから作成します。Archiveとソースの対応を追えるよう、タグと`git rev-parse HEAD`の結果を記録します。`CMakeLists.txt`、`vcpkg.json`、`version.txt`、`CHANGELOG.md`はrelease-pleaseが管理するため、手作業で変更しません。

GitHub CLIでPRを作る場合は、先に`gh auth status`で認証を確認します。Apple/XcodeへのサインインはGitHub CLIの認証には使えません。認証が無効なら`gh auth login -h github.com`を実行し、表示される認証手続きを完了してからPRを作成します。

```bash
gh pr create \
  --base main \
  --head <branch-name> \
  --title "fix: describe the change" \
  --body-file <pull-request-body.md>
```

## Archive前にXcode、バージョン、署名を確認する

複数のXcodeが入っている場合、画面で開いたXcodeとコマンドラインが使うXcodeが異なることがあります。Archive前に選択中のDeveloper Directoryとバージョンを確認します。

```bash
xcode-select -p
xcodebuild -version
```

今回の0.8.0 build 10はXcode 16.4でArchiveできました。次回はこのバージョンを前提にせず、プロジェクトとApp Store Connectが受け付けるXcodeを確認してください。

Apple DeveloperアカウントでXcodeにサインインし、対象チームとApple Distribution証明書が使えることを確認します。証明書を新しく作る場合はXcodeのアカウント設定から作成し、秘密鍵をリポジトリに置いたり、PRへ添付したりしません。

DevToolsでは、マーケティングバージョンとビルド番号を別々に設定できます。

| 項目 | 設定元 | 0.8.0 build 10 |
| --- | --- | --- |
| アプリのバージョン | `project(DevTools VERSION ...)` → `CFBundleShortVersionString` | `0.8.0` |
| ビルド番号 | CMakeキャッシュ変数`DevTools_BUILD_NUMBER` → `CFBundleVersion` | `10` |
| 署名チーム | CMakeキャッシュ変数`DEVTOOLS_APPLE_DEVELOPMENT_TEAM` | ローカルで指定 |
| 署名ID | CMakeキャッシュ変数`DEVTOOLS_APPLE_CODE_SIGN_IDENTITY` | ローカルで指定 |

ビルド番号は既定ではプロジェクトのバージョンと同じです。App Store Connectへ同じアプリバージョンのビルドを追加でアップロードするときは、ビルド番号を更新します。Appleの説明では`CFBundleVersion`は数字とピリオドで構成し、macOSアプリでは配布前に番号を増やします。[CFBundleVersion](https://developer.apple.com/documentation/BundleResources/Information-Property-List/CFBundleVersion)

Xcode用のCMakeプロジェクトを構成するときは、ビルド番号と署名設定をキャッシュ変数で渡します。次は値の形式を示す例です。`VCPKG_ROOT`はvcpkgのインストール先、`Qt6_DIR`はQtのCMake設定ファイルがあるディレクトリに合わせます。`TEAM_ID`と証明書名は実際の環境に置き換え、実値をコミットしないでください。

```bash
export VCPKG_ROOT="/path/to/vcpkg"

cmake -S . -B build-appstore -G Xcode \
  -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=arm64-osx \
  -DQt6_DIR="<Qt6_DIRのパス>" \
  -DDevTools_BUILD_NUMBER=10 \
  -DDEVTOOLS_APPLE_DEVELOPMENT_TEAM="TEAM_ID" \
  -DDEVTOOLS_APPLE_CODE_SIGN_IDENTITY="Apple Distribution: Company Name (TEAM_ID)"
```

`DEVTOOLS_APPLE_CODE_SIGN_IDENTITY`を指定すると、CMakeはXcodeの署名スタイルをManualに設定し、`macdeployqt`にも同じ署名IDを渡します。自動署名を使う場合は、Xcodeの「Signing & Capabilities」で対象チームとプロビジョニング設定を確認してください。どちらの方式でも、Archiveに使った署名IDがアプリ本体と同梱バイナリに適用されていることを検証します。

## Archiveを検証してからアップロードする

1. CMake構成時に選んだXcodeプロジェクトを開きます。例: `open build-appstore/DevTools.xcodeproj`。
2. 対象ターゲットのSigning設定、バージョン、ビルド番号、arm64アーキテクチャを確認します。
3. `Product > Archive`を実行し、OrganizerでArchiveを開きます。
4. OrganizerでValidateを実行し、エラーと警告を確認します。
5. 署名検証に成功してから`Distribute App`を選び、App Store Connectへアップロードします。

CMakeはmacOSのビルド後に`macdeployqt -appstore-compliant`を実行します。署名IDを指定した場合は、`macdeployqt`にも署名オプションが渡ります。手作業で再度`macdeployqt`や署名を実行する前に、Archiveのログと署名状態を確認してください。

Archiveのアプリ本体を直接確認できる場合は、次のように署名を検証します。

```bash
codesign --verify --deep --strict --verbose=2 \
  "<archive-path>/Products/Applications/DevTools.app"
```

Mac App StoreではApp Sandboxが必要です。DevToolsの`distribution/DevTools.entitlements`には、sandbox、ネットワーク接続、ユーザーが選んだファイルへの読み書き、アプリスコープのブックマークが設定されています。[App Sandbox](https://developer.apple.com/documentation/security/protecting-user-data-with-app-sandbox)

## App Store Connectで処理完了を確認してから審査へ進む

アップロード後、App Store Connectの処理が終わるまでビルドは一覧に表示されません。処理完了後、アプリのバージョンページでアップロードしたビルドを選び、リリースノートやストア情報を確認します。ビルドのアップロードだけでは審査提出になりません。内容を確認したメンテナーが提出します。[ビルドのアップロード](https://developer.apple.com/help/app-store-connect/manage-builds/upload-builds/)、[App Store Connectのワークフロー](https://developer.apple.com/help/app-store-connect/get-started/app-store-connect-workflow)

リリースノートには、そのバージョンで利用者に見える変更を短い箇条書きで記載します。修正内容、追加機能、既知の制限を実際にアップロードするビルドに合わせて確認し、開発中の変更を混ぜません。

## 0.8.0はアップロードまで通ったが、DBドライバーの警告が残った

- **Xcodeの選択先を先に確認する。** Xcodeへサインインしていても、別バージョンがコマンドラインで選択されている場合があります。`xcode-select -p`と`xcodebuild -version`の両方を確認します。
- **SQLiteの外部ファイルはブックマークで再接続する。** App Sandboxでは標準のファイル選択パネルで利用者に選んでもらい、アプリを終了した後も再接続する場合はセキュリティスコープ付きブックマークを保存します。解決したURLでアクセスを開始し、使用後に終了します。DevToolsでは接続履歴にブックマークを保存し、再接続時に復元します。[Sandbox外のファイルへのアクセス](https://developer.apple.com/documentation/security/accessing-files-from-the-macos-app-sandbox?language=objc)
- **同梱Qtプラグインの警告を確認する。** 今回の`macdeployqt`は、Private APIを使うODBCとPostgreSQLのプラグインをスキップし、ビルド機にないドライバー依存関係も報告しました。アップロードに成功しても対象DBドライバーがアプリに入っているとは限りません。配布前に、必要な接続方式がArchiveに含まれ、Sandbox環境で動作することを確認します。
- **pushフックの無出力はすぐに停止と判断しない。** pre-pushのCMakeビルドは出力を一時ログに記録し、終了後に結果を表示します。今回もビルド完了後に`Passed`となりました。
- **秘密情報は差分で確認する。** 今回の21ファイルを検索し、秘密鍵、一般的なAPIトークン、埋め込み認証情報、ローカルパスは見つかりませんでした。メール形式の文字列は翻訳リソースの原文と訳文に含まれる、`example.com`と`example.org`の例示アドレス4箇所だけでした。gitleaksは環境になかったため、この確認は網羅的な保証ではありません。署名チームIDと署名IDはソースへ固定せず、CMakeキャッシュで指定します。

### build 10はアップロード済みで、審査提出前

Archiveは作成でき、アプリの深い署名検証を通過してApp Store Connectへアップロード済みです。この対応時点では、メンテナーによる審査提出を行っていません。

## 用語

- **アプリバージョン**: App Store上で利用者に表示する版。DevToolsでは`CFBundleShortVersionString`に入ります。
- **ビルド番号**: 同じアプリバージョン内のビルドを区別する番号。DevToolsでは`CFBundleVersion`に入ります。
- **セキュリティスコープ付きブックマーク**: 利用者が選んだSandbox外のファイルへのアクセス権を、アプリの再起動後も復元するために保存するデータです。

手順とCMake設定が実装と異なる場合は、`CMakeLists.txt`と`distribution/DevTools.entitlements`を確認してこのガイドを更新してください。解決しない点は、該当するリリースPRかリポジトリのIssueでメンテナー間で確認します。

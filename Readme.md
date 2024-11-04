# DevTools
## 説明
開発の際に今まで適当なwebサービスとかを使っていた簡単な作業を、一つのツールに纏めたい。  
そんなアプリケーションです。

## 開発
### 新しいコードを追加する
コンパイル時間の短縮のため、複数の静的ライブラリに分割してコンパイルする事にしました。

以下のように、追加するコード群を `qt_add_library` で追加します。
名前はそれと分かれば何でも良いです。
```cmake
qt_add_library(${PROJECT_NAME}_image_resize STATIC
    core/image/resize/image_resize.h core/image/resize/image_resize.cpp
    gui/image/resize/image_resize_gui.h gui/image/resize/image_resize_gui.cpp gui/image/resize/image_resize_gui.ui
)
```

これらのモジュールに依存関係がある場合は依存関係を定義して下さい。
例えば画像編集系のコードは↓のように依存関係を定義しています。
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
tests/DevToolsTests.cmakeに以下のように追加します。
```cmake
DevTools_add_test(test_basic_image_io   # テスト名
    SOURCES
    tests/core/image/test_basic_image_io.cpp    # テストコード
)
```
デフォルトではテストはビルドしない設定になっています。
CMakeの設定で `ENABLE_UNIT_TEST` をONにするとテストができるようになります。
ただし、デバッグ実効時などにもテストコードがビルドされるのでコンパイル時間が伸びます。

### 外部ライブラリを追加する
**手間が掛かります。可能な限りQtで用意されている機能を使いましょう。**

Qt以外の外部ライブラリを利用する場合は[vcpkg](https://vcpkg.io/)を使います。
Qt Creatorの方でプラグインを有効にして下さい。
MacOSではなぜかarm64と判定されないのでCMakeの引数に以下を追加して下さい。  
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
更に、インクルードディレクトリとライブラリへのリンクを定義して下さい。
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

## トラブルシューティング
* CMakeでエラーが出たら
    * 構文エラーなどの確認
    * build/Qt_*/CMakeCache.txt を消してみる
    * build/Qt_* フォルダを消す

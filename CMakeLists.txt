cmake_minimum_required(VERSION 3.21.1)

# vcpkgを使わない時はONにする
set(QT_CREATOR_SKIP_PACKAGE_MANAGER_SETUP OFF)

project(DevTools VERSION 0.0.0 LANGUAGES CXX)

# -test, -alpha, -beta, -rcなど。無しでも良い。
set(DevTools_VERSION_SUFFIX "-test")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ENABLE_UNIT_TEST OFF CACHE BOOL "テストを行う場合はONにする")

# NOTE: ui_*.uiファイルが見つからない場合はここにパスを追加する。殆どの場合は設定不要。
#       the user interface file "*.ui" could not be found in the following directories "SRC:..."
set(CMAKE_AUTOUIC_SEARCH_PATHS
    gui/image/basic/
)

# Qtのパッケージを探す
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt6 REQUIRED COMPONENTS Widgets LinguistTools Network)
# vcpkgのパッケージを探す
find_package(toml11 CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
# Doxygenを探す
find_package(Doxygen)

qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES ja_JP en)

# TODO: 環境で分ける
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0") # NOTE: 現状arm64専用のため

    # アイコンを設定
    set(MACOSX_BUNDLE_ICON_FILE dev-tools.icns)
    set(APP_ICON_MACOS "res/${MACOSX_BUNDLE_ICON_FILE}")
    set_source_files_properties(${APP_ICON_MACOS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set(APP_ICON ${APP_ICON_MACOS})

    # gitハッシュ、アプリケーションのバージョン、コンパイル環境等を書き込む
    if(NOT EXISTS ${APP_INFO_FILE})
        execute_process(
            COMMAND "touch" ${APP_INFO_FILE}
        )
    endif()

    set(SH_FILE ${CMAKE_CURRENT_SOURCE_DIR}/generate_app_info.sh)
    file(CHMOD ${SH_FILE} PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
    add_custom_target(${PROJECT_NAME}_app_info
        COMMAND ${SH_FILE} "${CMAKE_PROJECT_VERSION}${DevTools_VERSION_SUFFIX}" "MacOS" ${CMAKE_SYSTEM_VERSION} ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}
    )
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    # Project files
    # main
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        res/application.qrc
        res/light_icons.qrc
        res/dark_icons.qrc
        ${APP_ICON}
        ${APP_INFO_FILE}
        res/dev-tools_ja_JP.ts
        res/dev-tools_en.ts

        main/main.cpp
        # ここには基本的に追加しないこと
        # qt_add_libraryで静的ライブラリにし、下の[モジュール一覧]に追加する
    )

    # modules
    qt_add_library(${PROJECT_NAME}_core STATIC
        gui/main_window.h gui/main_window.cpp gui/main_window.ui
        core/application_mixin.h core/application_mixin.cpp
        gui/gui_application.h gui/gui_application.cpp
        gui/menubar/about_devtools_dialog.h gui/menubar/about_devtools_dialog.cpp gui/menubar/about_devtools_dialog.ui
        gui/sidemenu.h gui/sidemenu.cpp gui/sidemenu.ui
        gui/contents_area.h gui/contents_area.cpp gui/contents_area.ui
        gui/sidemenu_item.h gui/sidemenu_item.cpp
        core/tool/tool.h core/tool/tool.cpp
        core/exception/common_exception.h
        core/exception/out_of_range_exception.h
        core/exception/invalid_argument_exception.h
        core/exception/invalid_state_exception.h
        core/exception/under_development_exception.h
        core/tool/tool_id_fields.h
        core/enum_cast.h
        gui/gui_tool.h gui/gui_tool.cpp
        gui/api_tool.h gui/api_tool.cpp gui/api_tool.ui
        gui/customsplitter.h
        gui/customsplitter.cpp
        gui/phrase_generation/phrase_generation.h gui/phrase_generation/phrase_generation.cpp gui/phrase_generation/phrase_generation.ui
        gui/menubar/about_devtools_dialog.cpp gui/menubar/about_devtools_dialog.h gui/menubar/about_devtools_dialog.ui gui/contents_area.cpp gui/contents_area.h gui/contents_area.ui gui/gui_application.cpp gui/gui_application.h gui/gui_tool.cpp gui/gui_tool.h gui/main_window.cpp gui/main_window.h gui/main_window.ui gui/sidemenu.cpp gui/sidemenu.h gui/sidemenu.ui gui/sidemenu_item.cpp gui/sidemenu_item.h
        gui/command/command.h gui/command/command.cpp gui/command/command.ui
        vcpkg.json
    )
    # image
    qt_add_library(${PROJECT_NAME}_image_core STATIC
        core/image/basic_image_io.h core/image/basic_image_io.cpp
        core/image/basic_image_edit_interface.h core/image/basic_image_edit_interface.cpp
        gui/image/basic/image_view.h gui/image/basic/image_view.cpp gui/image/basic/image_view.ui
        gui/image/basic/file_dialogs.h gui/image/basic/file_dialogs.cpp
        gui/image/basic/control.h gui/image/basic/control.cpp gui/image/basic/control.ui
    )
    qt_add_library(${PROJECT_NAME}_image_resize STATIC
        core/image/resize/image_resize.h core/image/resize/image_resize.cpp
        gui/image/resize/image_resize_gui.h gui/image/resize/image_resize_gui.cpp gui/image/resize/image_resize_gui.ui
    )
    qt_add_library(${PROJECT_NAME}_image_rotation STATIC
        core/image/rotation/image_rotation.h core/image/rotation/image_rotation.cpp
        gui/image/rotation/image_rotation_gui.h gui/image/rotation/image_rotation_gui.cpp gui/image/rotation/image_rotation_gui.ui
    )
    qt_add_library(${PROJECT_NAME}_image_division STATIC
        core/image/division/image_division.h core/image/division/image_division.cpp
        gui/image/division/image_division_gui.h gui/image/division/image_division_gui.cpp gui/image/division/image_division_gui.ui
        gui/image/division/image_view_for_image_division.h gui/image/division/image_view_for_image_division.cpp
    )
    qt_add_library(${PROJECT_NAME}_image_transparent STATIC
        core/image/transparent/image_transparent.h core/image/transparent/image_transparent.cpp
        gui/image/transparent/image_transparent_gui.h gui/image/transparent/image_transparent_gui.cpp gui/image/transparent/image_transparent_gui.ui
        gui/image/transparent/color_sample.h gui/image/transparent/color_sample.cpp
        gui/image/transparent/image_view_for_image_transparent.h gui/image/transparent/image_view_for_image_transparent.cpp
    )
    # phrase_generation
    qt_add_library(${PROJECT_NAME}_phrase_generation STATIC
        gui/phrase_generation/phrase_generation.h gui/phrase_generation/phrase_generation.cpp
        gui/phrase_generation/phrase_generation.ui
    )
    # 画像関係の依存関係
    add_dependencies(${PROJECT_NAME}_image_resize ${PROJECT_NAME}_image_core)
    add_dependencies(${PROJECT_NAME}_image_rotation ${PROJECT_NAME}_image_core)
    add_dependencies(${PROJECT_NAME}_image_division ${PROJECT_NAME}_image_core)
    add_dependencies(${PROJECT_NAME}_image_transparent ${PROJECT_NAME}_image_core)

    # data_conversion
    qt_add_library(${PROJECT_NAME}_data_conversion STATIC
        # core
        core/data_conversion/data_conversion.h core/data_conversion/data_conversion.cpp
        core/data_conversion/parser/basic_parser.h
        core/data_conversion/emitter/basic_emitter.h core/data_conversion/emitter/basic_emitter.cpp
        gui/data_conversion/data_conversion_gui.h gui/data_conversion/data_conversion_gui.cpp gui/data_conversion/data_conversion_gui.ui
        # json
        core/data_conversion/parser/json_parser.h core/data_conversion/parser/json_parser.cpp
        core/data_conversion/emitter/json_emitter.h core/data_conversion/emitter/json_emitter.cpp
        # yaml
        core/data_conversion/parser/yaml_parser.h core/data_conversion/parser/yaml_parser.cpp
        # toml
        core/data_conversion/parser/toml_parser.h core/data_conversion/parser/toml_parser.cpp
        core/data_conversion/emitter/toml_emitter.h core/data_conversion/emitter/toml_emitter.cpp
    )
    # toml11, yaml-cppのインクルードディレクトリ
    target_include_directories(${PROJECT_NAME}_data_conversion PUBLIC
        ${TOML11_INCLUDE_DIR}   # 何故か大文字
        ${YAML_CPP_INCLUDE_DIR} # 何故か大文字
    )
    # toml11はヘッダーのみなのでリンクしなくて良い、yaml-cppはリンクが必要
    target_link_libraries(${PROJECT_NAME}_data_conversion PUBLIC
        ${YAML_CPP_LIBRARIES}
    )

    #set_target_properties(${PROJECT_NAME}_core PROPERTIES
    #    AUTOMOC ON
    #    AUTOUIC ON
    #)

    qt_add_translations(
        TARGETS ${PROJECT_NAME}
        TS_FILE_DIR res/
        TS_FILE_BASE "dev-tools"
    )
else()
    # Qt5以下は使わない予定
    message(FATAL_ERROR "Qt5 configuration is not supported")
endif()

# BUILD_TYPE_* というプリプロセッサ定義を追加する(ビルドタイプ判別用)
target_compile_definitions(${PROJECT_NAME} PRIVATE BUILD_TYPE_${CMAKE_BUILD_TYPE})

# 有効にするのが望ましいが、mocまで対象になるのでやめておく
# 未定義動作の静的チェックを有効にする
#target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=undefined)
# メモリアドレス関連の静的チェックを有効にする
#target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address)

# app_info.autogen.cppを自動生成
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_app_info)

# 翻訳ファイルの更新と生成を自動化
# NOTE: 今の所動いているが、必ずしもupdate_translations→release_translationsの順に処理されるとは限らない可能性はある
add_dependencies(${PROJECT_NAME} update_translations)
add_dependencies(${PROJECT_NAME} release_translations)

# vcpkgのパッケージを探す
find_package(toml11 CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

# インクルードディレクトリとしてプロジェクトフォルダと依存ライブラリのインクルードディレクトリを追加
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# [モジュール一覧]
set(MODULE_LIST
    ${PROJECT_NAME}_core
    ${PROJECT_NAME}_image_core
    ${PROJECT_NAME}_image_resize
    ${PROJECT_NAME}_image_rotation
    ${PROJECT_NAME}_image_division
    ${PROJECT_NAME}_image_transparent
    ${PROJECT_NAME}_phrase_generation
    ${PROJECT_NAME}_data_conversion
)

foreach (MODULE ${MODULE_LIST})
    add_dependencies(${PROJECT_NAME} ${MODULE})
    if(NOT ${MODULE} STREQUAL ${PROJECT_NAME}_core)
        add_dependencies(${MODULE} ${PROJECT_NAME}_core)
    endif()

    # インクルードディレクトリとしてプロジェクトフォルダと依存ライブラリのインクルードディレクトリを追加
    target_include_directories(${MODULE} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${Qt6Core_INCLUDE_DIRS}
        ${Qt6Bus_INCLUDE_DIRS}
        ${Qt6Gui_INCLUDE_DIRS}
        ${Qt6Widgets_INCLUDE_DIRS}
    )

    # ライブラリをリンクする
    target_link_libraries(${MODULE} PUBLIC
        Qt6::Core
        Qt6::DBus
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
    )
endforeach()

# Tests
if(${ENABLE_UNIT_TEST})
    enable_testing()

    # TODO: submoduleに書きたいが... コードの一部を共有しているから無理か?
    include(tests/DevToolsTests.cmake)
endif()

# Doxygen
if(DOXYGEN_FOUND)
    include(docs/DevToolsDocs.cmake)
else()
    message(WARNING "Doxygen not found")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_dependencies(${PROJECT_NAME}
    ${MODULE_LIST}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${MODULE_LIST}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()

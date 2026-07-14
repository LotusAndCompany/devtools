cmake_minimum_required(VERSION 3.21.1)

# vcpkgを使わない時はONにする
set(QT_CREATOR_SKIP_PACKAGE_MANAGER_SETUP OFF)

project(DevTools VERSION 0.7.0 LANGUAGES CXX) # x-release-please-version

# -test, -alpha, -beta, -rcなど。無しでも良い。
set(DevTools_VERSION_SUFFIX "")

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
find_package(Qt6 REQUIRED COMPONENTS Core Widgets LinguistTools Sql Network Svg)
# vcpkgのパッケージを探す
find_package(toml11 CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

# qlementineはシステムQt6に対してビルドするためFetchContentを使用
include(FetchContent)
FetchContent_Declare(qlementine
    GIT_REPOSITORY "https://github.com/oclero/qlementine.git"
    GIT_TAG        v1.4.2
    GIT_SHALLOW    TRUE
)
set(QLEMENTINE_SANDBOX OFF CACHE BOOL "" FORCE)
set(QLEMENTINE_SHOWCASE OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(qlementine)
# Doxygenを探す（1.16以上が必要）
find_package(Doxygen 1.16)

qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES ja_JP en)

set(APP_INFO_FILE app_info.autogen.cpp)

# プラットフォーム固有の設定
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "15.0") # NOTE: 現状arm64専用のため
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.lotusandcompanyinc.dev-tools)

    # アイコンを設定
    set(MACOSX_BUNDLE_ICON_FILE dev-tools.icns)
    set(APP_ICON_MACOS "res/${MACOSX_BUNDLE_ICON_FILE}")
    set_source_files_properties(${APP_ICON_MACOS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set(APP_ICON ${APP_ICON_MACOS})

    set(OS_NAME "MacOS")
elseif(UNIX)
    set(OS_NAME "Linux")
endif()

# gitハッシュ、アプリケーションのバージョン、コンパイル環境等を書き込む
if(NOT EXISTS ${APP_INFO_FILE})
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E touch ${APP_INFO_FILE}
    )
endif()

set(SH_FILE ${CMAKE_CURRENT_SOURCE_DIR}/generate_app_info.sh)
file(CHMOD ${SH_FILE} PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
add_custom_target(${PROJECT_NAME}_app_info
    COMMAND ${SH_FILE} "${CMAKE_PROJECT_VERSION}${DevTools_VERSION_SUFFIX}" "${OS_NAME}" ${CMAKE_SYSTEM_VERSION} ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}
)

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
        main/main.cpp
        # ContentsArea instantiates feature modules, so keep it out of DevTools_core.
        gui/contents_area.h gui/contents_area.cpp
        vcpkg.json
        # ここには基本的に追加しないこと
        # qt_add_libraryで静的ライブラリにし、下の[モジュール一覧]に追加する
    )

    # modules
    # core
    qt_add_library(${PROJECT_NAME}_core STATIC
        core/application_mixin.h core/application_mixin.cpp
        core/tool/tool.h core/tool/tool.cpp
        core/exception/common_exception.h
        core/exception/out_of_range_exception.h
        core/exception/invalid_argument_exception.h
        core/exception/invalid_state_exception.h
        core/exception/under_development_exception.h
        core/tool/tool_id_fields.h
        core/enum_cast.h
        core/qr_tool/qrcodegen.hpp core/qr_tool/qrcodegen.cpp
        gui/main_window.h gui/main_window.cpp
        gui/gui_application.h gui/gui_application.cpp
        gui/menubar/about_devtools_dialog.h gui/menubar/about_devtools_dialog.cpp
        gui/menubar/settings_dialog.h gui/menubar/settings_dialog.cpp
        gui/sidemenu.h gui/sidemenu.cpp
        gui/sidemenu_item.h gui/sidemenu_item.cpp
        gui/welcome_page.h gui/welcome_page.cpp
        gui/gui_tool.h gui/gui_tool.cpp
    )

    # http_request
    qt_add_library(${PROJECT_NAME}_http_request STATIC
        gui/api_tool.h gui/api_tool.cpp
    )

    # command
    qt_add_library(${PROJECT_NAME}_command STATIC
        gui/command/command.h gui/command/command.cpp gui/command/command_option.h
        gui/command/command_function.h
    )

    # phrase_generation
    qt_add_library(${PROJECT_NAME}_phrase_generation STATIC
        gui/phrase_generation/phrase_generation.h gui/phrase_generation/phrase_generation.cpp
    )

    # image
    qt_add_library(${PROJECT_NAME}_image_core STATIC
        core/image/basic_image_io.h core/image/basic_image_io.cpp
        core/image/basic_image_edit_interface.h core/image/basic_image_edit_interface.cpp
        gui/image/basic/image_view.h gui/image/basic/image_view.cpp
        gui/image/basic/file_dialogs.h gui/image/basic/file_dialogs.cpp
        gui/image/basic/control.h gui/image/basic/control.cpp
    )
    qt_add_library(${PROJECT_NAME}_image_tools_unified STATIC
        gui/image/basic/image_tools_unified_gui.h gui/image/basic/image_tools_unified_gui.cpp
        gui/image/transparent/color_sample.h gui/image/transparent/color_sample.cpp
        gui/image/transparent/image_view_for_image_transparent.h gui/image/transparent/image_view_for_image_transparent.cpp
    )

    # db_tool
    qt_add_library(${PROJECT_NAME}_db_tool STATIC
        gui/db_tool/query_page/query_page.h gui/db_tool/query_page/query_page.cpp
        gui/db_tool/main/db_main.h gui/db_tool/main/db_main.cpp
        gui/db_tool/connection_window/connection_window.h gui/db_tool/connection_window/connection_window.cpp
        gui/db_tool/connection_selector/connection_selector.h gui/db_tool/connection_selector/connection_selector.cpp
    )

    # 画像関係の依存関係
    add_dependencies(${PROJECT_NAME}_image_tools_unified ${PROJECT_NAME}_image_core)

    # data_conversion
    qt_add_library(${PROJECT_NAME}_data_conversion STATIC
        # core
        core/data_conversion/data_conversion.h core/data_conversion/data_conversion.cpp
        core/data_conversion/parser/basic_parser.h
        core/data_conversion/emitter/basic_emitter.h core/data_conversion/emitter/basic_emitter.cpp
        gui/data_conversion/data_conversion_gui.h gui/data_conversion/data_conversion_gui.cpp
        # json
        core/data_conversion/parser/json_parser.h core/data_conversion/parser/json_parser.cpp
        core/data_conversion/emitter/json_emitter.h core/data_conversion/emitter/json_emitter.cpp
        # yaml
        core/data_conversion/parser/yaml_parser.h core/data_conversion/parser/yaml_parser.cpp
        core/data_conversion/emitter/yaml_emitter.h core/data_conversion/emitter/yaml_emitter.cpp
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
        yaml-cpp::yaml-cpp
    )

    # qr_code_generation
    qt_add_library(${PROJECT_NAME}_qr_code_generation STATIC
        core/qr_tool/content_generator.h
        core/qr_tool/content_generator.cpp
        gui/qr_code/qr_code_generation_gui.h
        gui/qr_code/qr_code_generation_gui.cpp
    )

    #set_target_properties(${PROJECT_NAME}_core PROPERTIES
    #    AUTOMOC ON
    #    AUTOUIC ON
    #)

    set(TRANSLATION_TS_FILES
        res/dev-tools_ja_JP.ts
    )

    qt_collect_translation_source_targets(I18N_SOURCE_TARGETS)

    qt_add_lupdate(
        SOURCE_TARGETS ${I18N_SOURCE_TARGETS}
        TS_FILES ${TRANSLATION_TS_FILES}
        LUPDATE_TARGET update_devtools_translations
        OPTIONS -locations none -no-obsolete
        NO_GLOBAL_TARGET
    )

    qt_add_lrelease(
        TS_FILES ${TRANSLATION_TS_FILES}
        LRELEASE_TARGET release_devtools_translations
        QM_FILES_OUTPUT_VARIABLE TRANSLATION_QM_FILES
        NO_GLOBAL_TARGET
    )

    qt_add_resources(${PROJECT_NAME} "${PROJECT_NAME}_translations"
        PREFIX "/i18n"
        BASE "${CMAKE_CURRENT_BINARY_DIR}"
        FILES ${TRANSLATION_QM_FILES}
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

# 画像関係の依存関係
# target_link_libraries を使用してリンク依存関係も設定する（GNU ld 対応）
target_link_libraries(${PROJECT_NAME}_image_tools_unified PUBLIC ${PROJECT_NAME}_image_core)

# qlementineはgui_application.cppで使用するためDevTools_coreにリンク
target_link_libraries(${PROJECT_NAME}_core PUBLIC qlementine)

# app_info.autogen.cppを自動生成
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_app_info)

# vcpkgのパッケージを探す
find_package(toml11 CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()


# [モジュール一覧]
set(MODULE_LIST
    ${PROJECT_NAME}_core
    ${PROJECT_NAME}_image_core
    ${PROJECT_NAME}_image_tools_unified
    ${PROJECT_NAME}_phrase_generation
    ${PROJECT_NAME}_http_request
    ${PROJECT_NAME}_command
    ${PROJECT_NAME}_data_conversion
    ${PROJECT_NAME}_qr_code_generation
    ${PROJECT_NAME}_db_tool
)

set(APPLICATION_MODULE_LIST
    ${PROJECT_NAME}_image_tools_unified
    ${PROJECT_NAME}_phrase_generation
    ${PROJECT_NAME}_http_request
    ${PROJECT_NAME}_command
    ${PROJECT_NAME}_data_conversion
    ${PROJECT_NAME}_qr_code_generation
    ${PROJECT_NAME}_db_tool
)
set(APPLICATION_LINK_MODULES ${APPLICATION_MODULE_LIST})

foreach (MODULE ${MODULE_LIST})
    add_dependencies(${PROJECT_NAME} ${MODULE})
    if(NOT ${MODULE} STREQUAL ${PROJECT_NAME}_core
        AND NOT ${MODULE} STREQUAL ${PROJECT_NAME}_image_tools_unified)
        # target_link_libraries を使用してリンク依存関係も設定する（GNU ld 対応）
        target_link_libraries(${MODULE} PUBLIC ${PROJECT_NAME}_core)
    endif()

    # インクルードディレクトリとしてプロジェクトフォルダと依存ライブラリのインクルードディレクトリを追加
    target_include_directories(${MODULE} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${Qt6Core_INCLUDE_DIRS}
        ${Qt6Bus_INCLUDE_DIRS}
        ${Qt6Gui_INCLUDE_DIRS}
        ${Qt6Widgets_INCLUDE_DIRS}
        ${Qt6Network_INCLUDE_DIRS}
        ${Qt6Sql_INCLUDE_DIRS}
    )

    # ライブラリをリンクする
    target_link_libraries(${MODULE} PUBLIC
        Qt6::Core
        Qt6::DBus
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::Sql
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
    include(doxygen/DevToolsDocs.cmake)
else()
    message(STATUS "Doxygen not found")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_dependencies(${PROJECT_NAME}
    ${MODULE_LIST}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${APPLICATION_LINK_MODULES}
)

string(TOLOWER ${CMAKE_GENERATOR} GENERATOR_LOWER)
if(${GENERATOR_LOWER} STREQUAL "xcode")
    configure_file(${CMAKE_SOURCE_DIR}/distribution/Info.plist.in ${CMAKE_BINARY_DIR}/distribution/Info.plist @ONLY)

    set(XCODE_ASSETS distribution/Assets.xcassets)
    target_sources(${PROJECT_NAME} PRIVATE ${XCODE_ASSETS})
    set_source_files_properties(${XCODE_ASSETS} PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )

    #file(RELATIVE_PATH REL_BUILD_DIR "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}")

    # NOTE: XCODE_ATTRIBUTE_*は↓で確認
    # https://developer.apple.com/documentation/xcode/build-settings-reference
    set_target_properties(${PROJECT_NAME} PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION}${DevTools_VERSION_SUFFIX}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE    TRUE
        WIN32_EXECUTABLE TRUE
        # 機能しないので削除
        #INSTALL_RPATH "@executable_path/../Frameworks;@executable_path/../Libs"
        # NOTE: x86_64もサポートするのが望ましいが、静的リンクライブラリがビルドできなかった。
        #       また、パッケージマネージャと競合する可能性がある。
        XCODE_ATTRIBUTE_ARCHS   "arm64"
        XCODE_ATTRIBUTE_SDKROOT "macosx"
        XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH  "YES"
        XCODE_ATTRIBUTE_PRODUCT_NAME    ${PROJECT_NAME}
        MACOSX_BUNDLE_INFO_PLIST    ${CMAKE_BINARY_DIR}/distribution/Info.plist
        #XCODE_ATTRIBUTE_INFOPLIST_FILE  ${REL_BUILD_DIR}/distribution/Info.plist
        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME  "AppIcon"
        XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS  "distribution/${PROJECT_NAME}.entitlements"
        XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/../Frameworks"
    )

    # 上手く動かない...
    # add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    #     COMMAND ${MACDEPLOYQT_EXECUTABLE} ${CMAKE_BUILD_TYPE}/${PROJECT_NAME}.app
    #     VERBATIM
    # )
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION}${DevTools_VERSION_SUFFIX}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE    TRUE
        WIN32_EXECUTABLE TRUE
    )
endif()

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# =============================================================================
# Code Quality Tools (clang-format and clang-tidy)
# =============================================================================

if(APPLE)
    set(CLANG_FORMAT_HINTS /opt/homebrew/opt/llvm/bin /usr/local/opt/llvm/bin)
else()
    set(CLANG_FORMAT_HINTS)
endif()
find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format clang-format-17 clang-format-16 clang-format-15
    HINTS ${CLANG_FORMAT_HINTS})
if(APPLE)
    set(CLANG_TIDY_HINTS /opt/homebrew/opt/llvm/bin /usr/local/opt/llvm/bin)
else()
    set(CLANG_TIDY_HINTS)
endif()
find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy clang-tidy-17 clang-tidy-16 clang-tidy-15
    HINTS ${CLANG_TIDY_HINTS})

file(GLOB_RECURSE ALL_SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/core/*.cpp
    ${CMAKE_SOURCE_DIR}/core/*.h
    ${CMAKE_SOURCE_DIR}/gui/*.cpp
    ${CMAKE_SOURCE_DIR}/gui/*.h
    ${CMAKE_SOURCE_DIR}/main/*.cpp
    ${CMAKE_SOURCE_DIR}/main/*.h
    ${CMAKE_SOURCE_DIR}/tests/*.cpp
    ${CMAKE_SOURCE_DIR}/tests/*.h
)

list(FILTER ALL_SOURCE_FILES EXCLUDE REGEX ".*/build.*/.*")
list(FILTER ALL_SOURCE_FILES EXCLUDE REGEX ".*/CMakeFiles/.*")
list(FILTER ALL_SOURCE_FILES EXCLUDE REGEX ".*/\\.cache/.*")
# Exclude third-party library files from linting
list(FILTER ALL_SOURCE_FILES EXCLUDE REGEX ".*/qrcodegen\\.cpp$")
list(FILTER ALL_SOURCE_FILES EXCLUDE REGEX ".*/qrcodegen\\.hpp$")

# Create LINT_SOURCE_FILES excluding tests (format still applies to tests)
set(LINT_SOURCE_FILES ${ALL_SOURCE_FILES})
list(FILTER LINT_SOURCE_FILES EXCLUDE REGEX ".*/tests/.*")

if(CLANG_FORMAT_EXECUTABLE)
    message(STATUS "Found clang-format: ${CLANG_FORMAT_EXECUTABLE}")

    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${ALL_SOURCE_FILES}
        COMMENT "Formatting source files with clang-format"
        VERBATIM
    )

    add_custom_target(format-check
        COMMAND ${CLANG_FORMAT_EXECUTABLE} --dry-run --Werror ${ALL_SOURCE_FILES}
        COMMENT "Checking source file formatting with clang-format"
        VERBATIM
    )
else()
    message(WARNING "clang-format not found. Format targets will not be available.")

    add_custom_target(format
        COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found. Please install it."
        COMMENT "clang-format not available"
    )

    add_custom_target(format-check
        COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found. Please install it."
        COMMENT "clang-format not available"
    )
endif()

if(CLANG_TIDY_EXECUTABLE)
    message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXECUTABLE}")

    set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Generate compile_commands.json" FORCE)

    # Get macOS SDK path for proper system header resolution (macOS only)
    set(CLANG_TIDY_EXTRA_ARGS "")
    if(APPLE)
        find_program(XCRUN_EXECUTABLE xcrun)
        if(XCRUN_EXECUTABLE)
            execute_process(
                COMMAND ${XCRUN_EXECUTABLE} --show-sdk-path
                OUTPUT_VARIABLE MACOS_SDK_PATH
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            if(MACOS_SDK_PATH)
                list(APPEND CLANG_TIDY_EXTRA_ARGS "--extra-arg=-isysroot${MACOS_SDK_PATH}")
            endif()
        endif()
    endif()

    add_custom_target(lint
        COMMAND ${CLANG_TIDY_EXECUTABLE} -p ${CMAKE_BINARY_DIR}
            ${CLANG_TIDY_EXTRA_ARGS}
            ${LINT_SOURCE_FILES}
        COMMENT "Running clang-tidy static analysis"
        VERBATIM
    )

    add_custom_target(lint-fix
        COMMAND ${CLANG_TIDY_EXECUTABLE} -p ${CMAKE_BINARY_DIR} --fix --fix-errors
            ${CLANG_TIDY_EXTRA_ARGS}
            ${LINT_SOURCE_FILES}
        COMMENT "Running clang-tidy with automatic fixes"
        VERBATIM
    )
else()
    message(WARNING "clang-tidy not found. Lint targets will not be available.")

    add_custom_target(lint
        COMMAND ${CMAKE_COMMAND} -E echo "clang-tidy not found. Please install it."
        COMMENT "clang-tidy not available"
    )

    add_custom_target(lint-fix
        COMMAND ${CMAKE_COMMAND} -E echo "clang-tidy not found. Please install it."
        COMMENT "clang-tidy not available"
    )
endif()

add_custom_target(run
    COMMAND ${CMAKE_BINARY_DIR}/DevTools.app/Contents/MacOS/DevTools
    DEPENDS ${PROJECT_NAME}
    COMMENT "Running DevTools with console output"
    VERBATIM
)

add_custom_target(quality-check
    DEPENDS format-check lint
    COMMENT "Running all code quality checks"
)

option(ENABLE_CLANG_TIDY "Enable clang-tidy during compilation" OFF)
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXECUTABLE)
    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXECUTABLE})
    message(STATUS "clang-tidy enabled for compilation")
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()

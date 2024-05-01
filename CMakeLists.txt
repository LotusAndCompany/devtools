cmake_minimum_required(VERSION 3.5)

project(DevTools VERSION 0.0.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# パッケージを探す
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Test)
find_package(Doxygen)

qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES ja_JP)

# アイコンを設定
# TODO: 環境で分ける
if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE dev-tools.icns)
    set(APP_ICON_MACOS "res/${MACOSX_BUNDLE_ICON_FILE}")
    set_source_files_properties(${APP_ICON_MACOS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set(APP_ICON ${APP_ICON_MACOS})
endif()

set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.lotusandcompanyinc.dev-tools)

# gitハッシュ、アプリケーションのバージョン、コンパイル環境等を書き込む
set(APP_INFO_FILE app_info.autogen.cpp)
# TODO: 環境毎に分ける
# configure_file()を使うようにしたかったが、ビルド時にCMAKE変数を参照させるのは無理そう?
if(APPLE)
    if(NOT EXISTS ${APP_INFO_FILE})
        execute_process(
            COMMAND "touch" ${APP_INFO_FILE}
        )
    endif()

    set(SH_FILE ${CMAKE_CURRENT_SOURCE_DIR}/generate_app_info.sh)
    file(CHMOD ${SH_FILE} PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
    add_custom_target(${PROJECT_NAME}_app_info
        COMMAND ${SH_FILE} ${CMAKE_PROJECT_VERSION} "MacOS" ${CMAKE_SYSTEM_VERSION} ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}
    )
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    # Project files
    set(TRANSLATION_FILES res/dev-tools_ja_JP.ts res/dev-tools_en.ts)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        res/application.qrc
        res/light_icons.qrc
        res/dark_icons.qrc
        ${APP_ICON}
        ${APP_INFO_FILE}
        ${TRANSLATION_FILES}

        main/main.cpp
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
        core/image/basic_image_io.h core/image/basic_image_io.cpp
        core/image/basic_image_edit_interface.h core/image/basic_image_edit_interface.cpp
        core/image/resize/image_resize.h core/image/resize/image_resize.cpp
        gui/image/basic/image_view.h gui/image/basic/image_view.cpp gui/image/basic/image_view.ui
        gui/image/basic/file_dialogs.h gui/image/basic/file_dialogs.cpp
        gui/image/basic/control.h gui/image/basic/control.cpp gui/image/basic/control.ui
        gui/image/resize/image_resize_gui.h gui/image/resize/image_resize_gui.cpp gui/image/resize/image_resize_gui.ui
        core/image/rotation/image_rotation.h core/image/rotation/image_rotation.cpp
        gui/image/rotation/image_rotation_gui.h gui/image/rotation/image_rotation_gui.cpp gui/image/rotation/image_rotation_gui.ui
    )
    qt_add_translations(
        TARGETS ${PROJECT_NAME}
        TS_FILE_DIR res/
        TS_FILE_BASE "dev-tools"
    )
else()
    # Qt5は使わない予定
    message(FATAL_ERROR "Qt5 configuration is not supported")

    if(ANDROID)
        add_library(DevTools SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(DevTools
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()


if(${CMAKE_BUILD_TYPE} STREQUAL "Debug" OR ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    set(ENABLE_DEBUG_BUILD_FEATURES ON)
else()
    set(ENABLE_DEBUG_BUILD_FEATURES OFF)
endif()

# TODO: tests/CMakeLists.txtに書きたい
#add_subdirectory(tests)
# Tests
if(${ENABLE_DEBUG_BUILD_FEATURES})
    set(TEST_CONFIG_IN ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_config.cpp.in)
    set(TEST_CONFIG_OUT ${CMAKE_CURRENT_BINARY_DIR}/tests/test_config.cpp)
    configure_file(${TEST_CONFIG_IN} ${TEST_CONFIG_OUT} @ONLY)

    add_library(${PROJECT_NAME}_test_util SHARED
        tests/test_util.h tests/test_util.cpp
        tests/random_data.h tests/random_data.cpp
        tests/mock_helper.h
    )
    target_include_directories(${PROJECT_NAME}_test_util PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/tests)
    target_link_libraries(${PROJECT_NAME}_test_util PUBLIC Qt6::Core)

    add_library(${PROJECT_NAME}_tool SHARED
        core/tool/tool.h core/tool/tool.cpp)
    target_link_libraries(${PROJECT_NAME}_tool PUBLIC Qt6::Core)

    function(DevTools_add_test TEST_NAME)
        cmake_parse_arguments(
            DEVTOOLS_TEST
            "NO_UTIL;NO_TOOL;GUI;WIDGETS"    # options (NO_UTIL: test_util.hを使わない, NO_TOOL: tool.hに依存しない, GUI: guiに依存する, WIDGETS: QWidgetに依存する)
            ""  # one value keywords
            "SOURCES"   # multi value keywords
            ${ARGN}
        )

        #message(NOTICE "TEST_NAME=${TEST_NAME}")
        #message(NOTICE ARGN=${ARGN})
        #message(NOTICE DEVTOOLS_TEST_SOURCES=${DEVTOOLS_TEST_SOURCES})
        #message(NOTICE "DEVTOOLS_TEST_NO_UTIL=${DEVTOOLS_TEST_NO_UTIL}")
        #message(NOTICE "DEVTOOLS_TEST_NO_TOOL=${DEVTOOLS_TEST_NO_TOOL}")
        #message(NOTICE "DEVTOOLS_TEST_GUI=${DEVTOOLS_TEST_GUI}")
        #message(NOTICE "DEVTOOLS_TEST_WIDGETS=${DEVTOOLS_TEST_WIDGETS}")
        #message(NOTICE "DEVTOOLS_TEST_UNPARSED_ARGUMENTS=${DEVTOOLS_TEST_UNPARSED_ARGUMENTS}")
        #message(NOTICE "DEVTOOLS_TEST_KEYWORDS_MISSING_VALUES=${DEVTOOLS_TEST_KEYWORDS_MISSING_VALUES}")

        add_executable(${TEST_NAME} ${DEVTOOLS_TEST_SOURCES})
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

        target_link_libraries(${TEST_NAME} PRIVATE Qt6::Test)

        if(NOT ${DEVTOOLS_TEST_NO_UTIL})
            target_link_libraries(${TEST_NAME} PRIVATE ${PROJECT_NAME}_test_util)
        endif()

        if(NOT ${DEVTOOLS_TEST_NO_TOOL})
            target_link_libraries(${TEST_NAME} PRIVATE ${PROJECT_NAME}_tool)
        endif()

        if(${DEVTOOLS_TEST_GUI} OR ${DEVTOOLS_TEST_WIDGETS})
            target_link_libraries(${TEST_NAME} PRIVATE Qt6::Gui)
        endif()

        if(${DEVTOOLS_TEST_WIDGETS})
            target_link_libraries(${TEST_NAME} PRIVATE Qt6::Widgets)
        endif()
    endfunction()
    
    # core
    DevTools_add_test(test_enum_cast NO_UTIL NO_TOOL
        SOURCES
        core/enum_cast.h
        tests/core/test_enum_cast.cpp
    )

    # core/exception
    DevTools_add_test(test_common_exception NO_TOOL
        SOURCES
        core/exception/common_exception.h
        tests/core/exception/test_common_exception.cpp
    )
    DevTools_add_test(test_under_development_exception NO_TOOL
        SOURCES
        core/exception/under_development_exception.h
        tests/core/exception/test_under_development_exception.cpp
    )
    DevTools_add_test(test_invalid_argument_exception NO_TOOL
        SOURCES
        core/exception/invalid_argument_exception.h
        tests/core/exception/test_invalid_argument_exception.cpp
    )
    DevTools_add_test(test_invalid_state_exception NO_TOOL
        SOURCES
        core/exception/invalid_state_exception.h
        tests/core/exception/test_invalid_state_exception.cpp
    )
    DevTools_add_test(test_out_of_range_exception NO_TOOL
        SOURCES
        core/exception/out_of_range_exception.h
        tests/core/exception/test_out_of_range_exception.cpp
    )

    # core/tool
    DevTools_add_test(test_tool NO_UTIL
        SOURCES
        core/tool/tool.h
        tests/core/tool/test_tool.cpp
    )

    # core/iamge
    DevTools_add_test(test_basic_image_io NO_TOOL GUI
        SOURCES
        core/image/basic_image_io.h
        core/image/basic_image_io.cpp
        tests/core/image/test_basic_image_io.cpp
    )
    DevTools_add_test(test_basic_image_edit_interface NO_TOOL NO_UTIL GUI
        SOURCES
        core/image/basic_image_edit_interface.h
        core/image/basic_image_edit_interface.cpp
        tests/core/image/test_basic_image_edit_interface.cpp
    )

    set(IMAGE_TOOL_SRC
        core/image/basic_image_io.cpp
        core/image/basic_image_edit_interface.cpp
    )

    # core/image/reseize
    DevTools_add_test(test_image_reisze GUI
        SOURCES
        core/image/resize/image_resize.h
        core/image/resize/image_resize.cpp
        ${IMAGE_TOOL_SRC}
        tests/core/image/resize/test_image_resize.cpp
    )

    # core/image/rotation
    DevTools_add_test(test_image_rotation GUI
        SOURCES
        core/image/rotation/image_rotation.h
        core/image/rotation/image_rotation.cpp
        ${IMAGE_TOOL_SRC}
        tests/core/image/rotation/test_image_rotation.cpp
    )

    # gui/image/basic
    DevTools_add_test(test_basic_image_view_control WIDGETS
        SOURCES
        gui/image/basic/control.h
        gui/image/basic/file_dialogs.h
        gui/image/basic/control.cpp
        gui/image/basic/file_dialogs.cpp
        tests/gui/image/basic/test_control.cpp
    )
    DevTools_add_test(test_basic_image_view WIDGETS
        SOURCES
        gui/image/basic/image_view.h
        gui/image/basic/image_view.cpp
        tests/gui/image/basic/test_basic_image_view.cpp
    )

    set(IMAGE_TOOL_GUI_SRC
        gui/image/basic/control.cpp
        gui/image/basic/file_dialogs.cpp
        gui/image/basic/image_view.cpp
        ${IMAGE_TOOL_SRC}
    )

    # gui/image/resize
    DevTools_add_test(test_image_resize_gui WIDGETS
        SOURCES
        gui/image/resize/image_resize_gui.h
        gui/image/resize/image_resize_gui.cpp
        core/image/resize/image_resize.h
        core/image/resize/image_resize.cpp
        ${IMAGE_TOOL_GUI_SRC}
        tests/gui/image/resize/test_image_resize_gui.cpp
    )

    # gui/image/rotation
    DevTools_add_test(test_image_rotation_gui WIDGETS
        SOURCES
        gui/image/rotation/image_rotation_gui.h
        gui/image/rotation/image_rotation_gui.cpp
        core/image/rotation/image_rotation.h
        core/image/rotation/image_rotation.cpp
        ${IMAGE_TOOL_GUI_SRC}
        tests/gui/image/rotation/test_image_rotation_gui.cpp
    )
endif()

# Documents
if(${ENABLE_DEBUG_BUILD_FEATURES})
    if(DOXYGEN_FOUND)
        # set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/docs/Doxyfile)
        SET(DOXYGEN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

        # copy icon
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/docs/dev-tools_icon.doxygen.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/docs/)

        # request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started")

        # note the option ALL which allows to build the docs together with the application
        add_custom_target(${PROJECT_NAME}_docs ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    else()
      message(WARNING "Doxygen not found")
    endif()
endif()

# PRIVATE BUILD_TYPE_* というプリプロセッサ定義を追加する(ビルドタイプ判別用)
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

# インクルードディレクトリとしてプロジェクトフォルダを追加
target_include_directories(${PROJECT_NAME}
    PRIVATE .
)

# ライブラリをリンクする
target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
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

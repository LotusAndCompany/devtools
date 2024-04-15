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
    macro(guiless_test test_name)
        add_executable(${test_name} ${ARGN} tests/test_util.cpp)
        add_test(NAME ${test_name} COMMAND ${test_name})

        target_link_libraries(${test_name} PRIVATE Qt${QT_VERSION_MAJOR}::Test)
    endmacro(guiless_test)

    #guiless_test(test_example tests/test_example.cpp tests/test_example_external.cpp)

    # core
    guiless_test(test_enum_cast tests/core/test_enum_cast.cpp)
    guiless_test(test_common_exception tests/core/exception/test_common_exception.cpp)
    guiless_test(test_under_development_exception tests/core/exception/test_under_development_exception.cpp)
    guiless_test(test_invalid_argument_exception tests/core/exception/test_invalid_argument_exception.cpp)
    guiless_test(test_invalid_state_exception tests/core/exception/test_invalid_state_exception.cpp)
    guiless_test(test_out_of_range_exception tests/core/exception/test_out_of_range_exception.cpp)
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

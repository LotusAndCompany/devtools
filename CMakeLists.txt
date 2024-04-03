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
if(APPLE)
    if(NOT EXISTS ${APP_INFO_FILE})
        execute_process(
            COMMAND "touch" ${APP_INFO_FILE}
        )
    endif()

    set(SH_FILE ${CMAKE_CURRENT_SOURCE_DIR}/generate_app_info.sh)
    file(CHMOD ${SH_FILE} PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
    add_custom_target(generate_app_info
        COMMAND ${SH_FILE} ${CMAKE_PROJECT_VERSION} "MacOS" ${CMAKE_SYSTEM_VERSION} ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}
    )
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    # プロジェクトファイル
    # TODO: Support unit test -> https://doc.qt.io/qt-6/qttest-index.html
    #       Debug, RelWithDebInfoの時にテストコードを追加すれば良い?
    set(TRANSLATION_FILES res/dev-tools_ja_JP.ts)
    qt_add_executable(DevTools
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
    )
    qt_add_translations(DevTools
        RESOURCE_PREFIX "i18n"
        TS_FILES ${TRANSLATION_FILES})
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

# TODO: tests/CMakeLists.txtに書きたい
#add_subdirectory(tests)
# Tests
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug" OR ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    macro(guiless_test test_name)
        add_executable(${test_name} ${ARGN})
        add_test(NAME ${test_name} COMMAND ${test_name})

        target_link_libraries(${test_name} PRIVATE Qt${QT_VERSION_MAJOR}::Test)
    endmacro(guiless_test)

    #guiless_test(test_example tests/test_example.cpp tests/test_example_external.cpp)
    guiless_test(test_enum_cast tests/core/test_enum_cast.cpp)
    guiless_test(test_common_exception tests/core/exception/test_common_exception.cpp)
    guiless_test(test_under_development_exception tests/core/exception/test_under_development_exception.cpp)
endif()

target_compile_definitions(DevTools PRIVATE BUILD_TYPE_${CMAKE_BUILD_TYPE})

add_dependencies(DevTools generate_app_info)
add_dependencies(DevTools_lrelease DevTools_lupdate)
add_dependencies(DevTools DevTools_lrelease)

target_include_directories(DevTools
    PRIVATE .
)

# ライブラリをリンクする
# TODO: 動的リンクになっているか確認する
target_link_libraries(DevTools
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
)

set_target_properties(DevTools PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS DevTools
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(DevTools)
endif()

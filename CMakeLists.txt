cmake_minimum_required(VERSION 3.5)

project(dev-tools VERSION 0.0.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES res/dev-tools_ja_JP.ts)

set(QRC_FILES res/default.qrc)

# TODO: Support unit test -> https://doc.qt.io/qt-6/qttest-index.html
#       Debug, RelWithDebInfoの時にテストコードを追加すれば良い?
set(PROJECT_SOURCES
        src/main.cpp
        src/gui/mainwindow.cpp
        include/gui/mainwindow.h
        src/gui/mainwindow.ui
        include/application/application_mixin.h
        src/application/application_mixin.cpp
        include/gui/gui_application.h
        src/gui/gui_application.cpp
        ${TS_FILES}
        ${QRC_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(dev-tools
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
    qt_add_translations(dev-tools
        TS_FILES
        ${TS_FILES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET dev-tools APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    #qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    message(WARNING "Qt5 configuration may not work correctly")

    if(ANDROID)
        add_library(dev-tools SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(dev-tools
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_include_directories(dev-tools
    PRIVATE .
    PRIVATE ./include
)

target_link_libraries(dev-tools
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
)

set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.lotusandcompanyinc.dev-tools)

set_target_properties(dev-tools PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS dev-tools
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(dev-tools)
endif()

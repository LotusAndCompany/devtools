# DevTools Tests

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

set(TEST_CONFIG_IN ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_config.cpp.in)
set(TEST_CONFIG_OUT ${CMAKE_CURRENT_BINARY_DIR}/tests/test_config.cpp)
configure_file(${TEST_CONFIG_IN} ${TEST_CONFIG_OUT} @ONLY)

qt6_add_library(${PROJECT_NAME}_test_lib SHARED
    tests/test_util.h tests/test_util.cpp
    tests/random_data.h tests/random_data.cpp
    tests/mock_helper.h
)
target_link_libraries(${PROJECT_NAME}_test_lib PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
)
target_include_directories(${PROJECT_NAME}_test_lib PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/tests
)

function(DevTools_add_test TEST_NAME)
    cmake_parse_arguments(
        DEVTOOLS_TEST
        "" # options
        ""  # one value keywords
        "SOURCES"   # multi value keywords
        ${ARGN}
    )

    add_executable(${TEST_NAME}
        ${DEVTOOLS_TEST_SOURCES}
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    add_dependencies(${TEST_NAME}
        ${PROJECT_NAME}_lib
        ${PROJECT_NAME}_test_lib
    )

    target_link_libraries(${TEST_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt6::Test
        ${PROJECT_NAME}_lib
        ${PROJECT_NAME}_test_lib
    )
    target_include_directories(${TEST_NAME} PRIVATE
        ${Qt6Test_INCLUDE_DIRS}
        ${CMAKE_AUTOUIC_SEARCH_PATHS}
    )
endfunction()

# core
DevTools_add_test(test_enum_cast
    SOURCES
    core/enum_cast.h
    tests/core/test_enum_cast.cpp
)

# core/exception
DevTools_add_test(test_common_exception
    SOURCES
    core/exception/common_exception.h
    tests/core/exception/test_common_exception.cpp
)
DevTools_add_test(test_under_development_exception
    SOURCES
    core/exception/under_development_exception.h
    tests/core/exception/test_under_development_exception.cpp
)
DevTools_add_test(test_invalid_argument_exception
    SOURCES
    core/exception/invalid_argument_exception.h
    tests/core/exception/test_invalid_argument_exception.cpp
)
DevTools_add_test(test_invalid_state_exception
    SOURCES
    core/exception/invalid_state_exception.h
    tests/core/exception/test_invalid_state_exception.cpp
)
DevTools_add_test(test_out_of_range_exception
    SOURCES
    core/exception/out_of_range_exception.h
    tests/core/exception/test_out_of_range_exception.cpp
)

# core/tool
DevTools_add_test(test_tool
    SOURCES
    core/tool/tool.h
    tests/core/tool/test_tool.cpp
)

# core/iamge
DevTools_add_test(test_basic_image_io
    SOURCES
    core/image/basic_image_io.h
    core/image/basic_image_io.cpp
    tests/core/image/test_basic_image_io.cpp
)
DevTools_add_test(test_basic_image_edit_interface
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
DevTools_add_test(test_image_reisze
    SOURCES
    core/image/resize/image_resize.h
    core/image/resize/image_resize.cpp
    ${IMAGE_TOOL_SRC}
    tests/core/image/resize/test_image_resize.cpp
)

# core/image/rotation
DevTools_add_test(test_image_rotation
    SOURCES
    core/image/rotation/image_rotation.h
    core/image/rotation/image_rotation.cpp
    ${IMAGE_TOOL_SRC}
    tests/core/image/rotation/test_image_rotation.cpp
)

# core/image/division
DevTools_add_test(test_image_division
    SOURCES
    core/image/division/image_division.h
    core/image/division/image_division.cpp
    ${IMAGE_TOOL_SRC}
    tests/core/image/division/test_image_division.cpp
)

# core/image/transparent
DevTools_add_test(test_image_transparent
    SOURCES
    core/image/transparent/image_transparent.h
    core/image/transparent/image_transparent.cpp
    ${IMAGE_TOOL_SRC}
    tests/core/image/transparent/test_image_transparent.cpp
)

# gui/image/basic
DevTools_add_test(test_basic_image_view_control
    SOURCES
    gui/image/basic/control.h
    gui/image/basic/file_dialogs.h
    gui/image/basic/control.cpp
    gui/image/basic/file_dialogs.cpp
    tests/gui/image/basic/test_control.cpp
)
DevTools_add_test(test_basic_image_view
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
DevTools_add_test(test_image_resize_gui
    SOURCES
    gui/image/resize/image_resize_gui.h
    gui/image/resize/image_resize_gui.cpp
    core/image/resize/image_resize.h
    core/image/resize/image_resize.cpp
    ${IMAGE_TOOL_GUI_SRC}
    tests/gui/image/resize/test_image_resize_gui.cpp
)

# gui/image/rotation
DevTools_add_test(test_image_rotation_gui
    SOURCES
    gui/image/rotation/image_rotation_gui.h
    gui/image/rotation/image_rotation_gui.cpp
    core/image/rotation/image_rotation.h
    core/image/rotation/image_rotation.cpp
    ${IMAGE_TOOL_GUI_SRC}
    tests/gui/image/rotation/test_image_rotation_gui.cpp
)

# gui/image/division
DevTools_add_test(test_image_view_for_image_division
    SOURCES
    gui/image/basic/image_view.h
    gui/image/basic/image_view.cpp
    gui/image/division/image_view_for_image_division.h
    gui/image/division/image_view_for_image_division.cpp
    tests/gui/image/division/test_image_view_for_image_division.cpp
)
DevTools_add_test(test_image_division_gui
    SOURCES
    gui/image/division/image_division_gui.h
    gui/image/division/image_division_gui.cpp
    core/image/division/image_division.h
    core/image/division/image_division.cpp
    gui/image/division/image_view_for_image_division.h
    gui/image/division/image_view_for_image_division.cpp
    ${IMAGE_TOOL_GUI_SRC}
    tests/gui/image/division/test_image_division_gui.cpp
)

# gui/image/transparent
DevTools_add_test(test_color_sample
    SOURCES
    gui/image/transparent/color_sample.h
    gui/image/transparent/color_sample.cpp
    tests/gui/image/transparent/test_color_sample.cpp
)
DevTools_add_test(test_image_view_for_image_transparent
    SOURCES
    gui/image/basic/image_view.h
    gui/image/basic/image_view.cpp
    gui/image/transparent/image_view_for_image_transparent.h
    gui/image/transparent/image_view_for_image_transparent.cpp
    tests/gui/image/transparent/test_image_view_for_image_transparent.cpp
)
DevTools_add_test(test_image_transparent_gui
    SOURCES
    core/image/transparent/image_transparent.h
    core/image/transparent/image_transparent.cpp
    gui/image/transparent/image_transparent_gui.h
    gui/image/transparent/image_transparent_gui.cpp
    gui/image/transparent/image_view_for_image_transparent.h
    gui/image/transparent/image_view_for_image_transparent.cpp
    gui/image/transparent/color_sample.h
    gui/image/transparent/color_sample.cpp
    ${IMAGE_TOOL_GUI_SRC}
    tests/gui/image/transparent/test_image_transparent_gui.cpp
)

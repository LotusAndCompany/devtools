# DevTools Tests

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

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
        # NO_UTIL: test_util.hを使わない
        # NO_TOOL: tool.hに依存しない
        # GUI: guiに依存する
        # WIDGETS: QWidgetに依存する
        # VCPKG_YAML: yaml-cppに依存する
        # VCPKG_TOML: toml11に依存する
        "NO_UTIL;NO_TOOL;GUI;WIDGETS;VCPKG_YAML;VCPKG_TOML" # options
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

    if(${DEVTOOLS_TEST_VCPKG_YAML})
        target_include_directories(${TEST_NAME}
            PRIVATE
            ${YAML_CPP_INCLUDE_DIR}
        )

        target_link_libraries(${TEST_NAME}
            PRIVATE ${YAML_CPP_LIBRARIES}
        )
    endif()
    if(${DEVTOOLS_TEST_VCPKG_TOML})
        target_include_directories(${TEST_NAME}
            PRIVATE
            ${TOML11_INCLUDE_DIR}
        )
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

# core/image/division
DevTools_add_test(test_image_division GUI
    SOURCES
    core/image/division/image_division.h
    core/image/division/image_division.cpp
    ${IMAGE_TOOL_SRC}
    tests/core/image/division/test_image_division.cpp
)

# core/image/transparent
DevTools_add_test(test_image_transparent GUI
    SOURCES
    core/image/transparent/image_transparent.h
    core/image/transparent/image_transparent.cpp
    ${IMAGE_TOOL_SRC}
    tests/core/image/transparent/test_image_transparent.cpp
)

# core/data_conversion/parser
DevTools_add_test(test_basic_parser
    SOURCES
    core/data_conversion/parser/basic_parser.h
    core/data_conversion/parser/basic_parser.cpp
    tests/core/data_conversion/parser/test_basic_parser.cpp
)
DevTools_add_test(test_json_parser
    SOURCES
    core/data_conversion/parser/basic_parser.h
    core/data_conversion/parser/basic_parser.cpp
    core/data_conversion/parser/json_parser.h
    core/data_conversion/parser/json_parser.cpp
    tests/core/data_conversion/parser/test_json_parser.cpp
)
DevTools_add_test(test_yaml_parser VCPKG_YAML
    SOURCES
    core/data_conversion/parser/basic_parser.h
    core/data_conversion/parser/basic_parser.cpp
    core/data_conversion/parser/yaml_parser.h
    core/data_conversion/parser/yaml_parser.cpp
    tests/core/data_conversion/parser/test_yaml_parser.cpp
)
DevTools_add_test(test_toml_parser VCPKG_YAML
    SOURCES
    core/data_conversion/parser/basic_parser.h
    core/data_conversion/parser/basic_parser.cpp
    core/data_conversion/parser/toml_parser.h
    core/data_conversion/parser/toml_parser.cpp
    tests/core/data_conversion/parser/test_toml_parser.cpp
)
# core/data_conversion
DevTools_add_test(test_data_conversion VCPKG_YAML VCPKG_TOML
    SOURCES
    core/data_conversion/data_conversion.h
    core/data_conversion/data_conversion.cpp
    core/data_conversion/parser/basic_parser.h
    core/data_conversion/parser/basic_parser.cpp
    core/data_conversion/parser/json_parser.h
    core/data_conversion/parser/json_parser.cpp
    core/data_conversion/parser/yaml_parser.h
    core/data_conversion/parser/yaml_parser.cpp
    tests/core/data_conversion/test_data_conversion.cpp
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

# gui/image/division
DevTools_add_test(test_image_view_for_image_division WIDGETS
    SOURCES
    gui/image/basic/image_view.h
    gui/image/basic/image_view.cpp
    gui/image/division/image_view_for_image_division.h
    gui/image/division/image_view_for_image_division.cpp
    tests/gui/image/division/test_image_view_for_image_division.cpp
)
DevTools_add_test(test_image_division_gui WIDGETS
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
DevTools_add_test(test_color_sample WIDGETS
    SOURCES
    gui/image/transparent/color_sample.h
    gui/image/transparent/color_sample.cpp
    tests/gui/image/transparent/test_color_sample.cpp
)
DevTools_add_test(test_image_view_for_image_transparent WIDGETS
    SOURCES
    gui/image/basic/image_view.h
    gui/image/basic/image_view.cpp
    gui/image/transparent/image_view_for_image_transparent.h
    gui/image/transparent/image_view_for_image_transparent.cpp
    tests/gui/image/transparent/test_image_view_for_image_transparent.cpp
)
DevTools_add_test(test_image_transparent_gui WIDGETS
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
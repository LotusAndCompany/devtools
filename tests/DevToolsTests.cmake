# DevTools Tests

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Test)
find_package(Qt6 REQUIRED COMPONENTS Test)

set(TEST_CONFIG_IN ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_config.cpp.in)
set(TEST_CONFIG_OUT ${CMAKE_CURRENT_BINARY_DIR}/tests/test_config.cpp)
configure_file(${TEST_CONFIG_IN} ${TEST_CONFIG_OUT} @ONLY)

qt_add_library(${PROJECT_NAME}_test_lib SHARED
    tests/test_util.h tests/test_util.cpp
    tests/random_data.h tests/random_data.cpp
    tests/mock_helper.h
    features/framework/gui/contents_area.h features/framework/gui/contents_area.cpp
)
target_link_libraries(${PROJECT_NAME}_test_lib PUBLIC
    Qt6::Test
    ${APPLICATION_LINK_MODULES}
)
target_include_directories(${PROJECT_NAME}_test_lib PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/tests
    ${CMAKE_AUTOUIC_SEARCH_PATHS}
    ${Qt6Test_INCLUDE_DIRS}
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
        ${PROJECT_NAME}_test_lib
    )
    target_link_libraries(${TEST_NAME} PRIVATE
        ${PROJECT_NAME}_test_lib
    )
endfunction()

# core
DevTools_add_test(test_enum_cast
    SOURCES
    features/framework/tests/test_enum_cast.cpp
)

# core/logging
DevTools_add_test(test_logging
    SOURCES
    features/framework/tests/test_logging.cpp
)

# core/exception
DevTools_add_test(test_common_exception
    SOURCES
    features/framework/tests/test_common_exception.cpp
)
DevTools_add_test(test_under_development_exception
    SOURCES
    features/framework/tests/test_under_development_exception.cpp
)
DevTools_add_test(test_invalid_argument_exception
    SOURCES
    features/framework/tests/test_invalid_argument_exception.cpp
)
DevTools_add_test(test_invalid_state_exception
    SOURCES
    features/framework/tests/test_invalid_state_exception.cpp
)
DevTools_add_test(test_out_of_range_exception
    SOURCES
    features/framework/tests/test_out_of_range_exception.cpp
)

# core/tool
DevTools_add_test(test_tool
    SOURCES
    features/framework/tests/test_tool.cpp
)

# core/iamge
DevTools_add_test(test_basic_image_io
    SOURCES
    features/image/tests/test_basic_image_io.cpp
)
DevTools_add_test(test_basic_image_edit_interface
    SOURCES
    features/image/tests/test_basic_image_edit_interface.cpp
)

# core/data_conversion/parser
DevTools_add_test(test_basic_parser
    SOURCES
    features/data_conversion/tests/parser/test_basic_parser.cpp
)
DevTools_add_test(test_json_parser
    SOURCES
    features/data_conversion/tests/parser/test_json_parser.cpp
)
DevTools_add_test(test_yaml_parser VCPKG_YAML
    SOURCES
    features/data_conversion/tests/parser/test_yaml_parser.cpp
)
DevTools_add_test(test_toml_parser VCPKG_YAML
    SOURCES
    features/data_conversion/tests/parser/test_toml_parser.cpp
)
# core/data_conversion/emitter
DevTools_add_test(test_basic_emitter
    SOURCES
    features/data_conversion/tests/emitter/test_basic_emitter.cpp
)
DevTools_add_test(test_json_emitter
    SOURCES
    features/data_conversion/tests/emitter/test_json_emitter.cpp
)
DevTools_add_test(test_toml_emitter
    SOURCES
    features/data_conversion/tests/emitter/test_toml_emitter.cpp
)
DevTools_add_test(test_yaml_emitter
    SOURCES
    features/data_conversion/tests/emitter/test_yaml_emitter.cpp
)
# core/data_conversion
DevTools_add_test(test_data_conversion
    SOURCES
    features/data_conversion/tests/test_data_conversion.cpp
)

# gui/image/basic
DevTools_add_test(test_basic_image_view_control
    SOURCES
    features/image/tests/basic/test_control.cpp
)
DevTools_add_test(test_basic_image_view
    SOURCES
    features/image/tests/basic/test_basic_image_view.cpp
)

# gui/image/transparent
DevTools_add_test(test_color_sample
    SOURCES
    features/image/tests/transparent/test_color_sample.cpp
)
DevTools_add_test(test_image_view_for_image_transparent
    SOURCES
    features/image/tests/transparent/test_image_view_for_image_transparent.cpp
)
# features/regex_tool/core
DevTools_add_test(test_regex_tool
    SOURCES
    features/regex_tool/tests/test_regex_tool.cpp
)

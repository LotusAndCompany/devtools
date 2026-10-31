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
)
target_link_libraries(${PROJECT_NAME}_test_lib PUBLIC
    Qt6::Core
    Qt6::Test
)
target_include_directories(${PROJECT_NAME}_test_lib PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/tests
    ${Qt6Test_INCLUDE_DIRS}
)

function(DevTools_add_test TEST_NAME)
    cmake_parse_arguments(
        DEVTOOLS_TEST
        "CORE_ONLY" # options
        ""  # one value keywords
        "SOURCES"   # multi value keywords
        ${ARGN}
    )

    add_executable(${TEST_NAME}
        ${DEVTOOLS_TEST_SOURCES}
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    if(DEVTOOLS_TEST_CORE_ONLY)
        target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
        target_link_libraries(${TEST_NAME} PRIVATE Qt6::Core Qt6::Test)
    else()
        add_dependencies(${TEST_NAME}
            ${PROJECT_NAME}_test_lib
        )
        target_link_libraries(${TEST_NAME} PRIVATE
            ${PROJECT_NAME}_test_lib
        )
    endif()
endfunction()

# core
DevTools_add_test(test_enum_cast CORE_ONLY
    SOURCES
    features/framework/tests/test_enum_cast.cpp
)

# core/logging
DevTools_add_test(test_logging CORE_ONLY
    SOURCES
    features/framework/tests/test_logging.cpp
)
DevTools_add_test(test_application_settings_service CORE_ONLY
    SOURCES
    features/framework/tests/test_application_settings_service.cpp
    features/framework/core/application_settings_service.cpp
)
DevTools_add_test(test_local_file_selection_service CORE_ONLY
    SOURCES
    features/framework/tests/test_local_file_selection_service.cpp
    features/framework/core/local_file_selection_service.cpp
)
DevTools_add_test(test_command_service CORE_ONLY
    SOURCES
    features/command/tests/test_command_service.cpp
    features/command/core/command_service.cpp
)

# core/exception
DevTools_add_test(test_common_exception
    SOURCES
    features/framework/tests/test_common_exception.cpp
)
DevTools_add_test(test_under_development_exception CORE_ONLY
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
DevTools_add_test(test_tool CORE_ONLY
    SOURCES
    features/framework/tests/test_tool.cpp
)
target_link_libraries(test_tool PRIVATE DevTools_core)

# core/markdown_preview
DevTools_add_test(test_markdown_preview CORE_ONLY
    SOURCES
    features/markdown_preview/tests/test_markdown_preview.cpp
    features/markdown_preview/core/markdown_service.cpp
)
target_link_libraries(test_markdown_preview PRIVATE Qt6::Gui)
# core/diff_tool
DevTools_add_test(test_diff_tool CORE_ONLY
    SOURCES
    features/diff_tool/tests/test_diff_tool.cpp
    features/diff_tool/core/diff_tool.cpp
)
target_link_libraries(test_diff_tool PRIVATE DevTools_core)

# core/image
DevTools_add_test(test_image_processing_service CORE_ONLY
    SOURCES
    features/image/tests/test_image_processing_service.cpp
    features/image/core/image_processing_service.cpp
)
target_link_libraries(test_image_processing_service PRIVATE Qt6::Gui)
# core/data_conversion/parser
DevTools_add_test(test_basic_parser CORE_ONLY
    SOURCES
    features/data_conversion/tests/parser/test_basic_parser.cpp
)
DevTools_add_test(test_json_parser
    SOURCES
    features/data_conversion/tests/parser/test_json_parser.cpp
)
target_link_libraries(test_json_parser PRIVATE DevTools_data_conversion)
DevTools_add_test(test_yaml_parser VCPKG_YAML
    SOURCES
    features/data_conversion/tests/parser/test_yaml_parser.cpp
)
target_link_libraries(test_yaml_parser PRIVATE DevTools_data_conversion)
DevTools_add_test(test_toml_parser VCPKG_YAML
    SOURCES
    features/data_conversion/tests/parser/test_toml_parser.cpp
)
target_link_libraries(test_toml_parser PRIVATE DevTools_data_conversion)
# core/data_conversion/emitter
DevTools_add_test(test_basic_emitter
    SOURCES
    features/data_conversion/tests/emitter/test_basic_emitter.cpp
)
target_link_libraries(test_basic_emitter PRIVATE DevTools_data_conversion)
DevTools_add_test(test_json_emitter
    SOURCES
    features/data_conversion/tests/emitter/test_json_emitter.cpp
)
target_link_libraries(test_json_emitter PRIVATE DevTools_data_conversion)
DevTools_add_test(test_toml_emitter
    SOURCES
    features/data_conversion/tests/emitter/test_toml_emitter.cpp
)
target_link_libraries(test_toml_emitter PRIVATE DevTools_data_conversion)
DevTools_add_test(test_yaml_emitter
    SOURCES
    features/data_conversion/tests/emitter/test_yaml_emitter.cpp
)
target_link_libraries(test_yaml_emitter PRIVATE DevTools_data_conversion)

# features/web_frontend
DevTools_add_test(test_native_bridge CORE_ONLY
    SOURCES
    features/web_frontend/tests/test_native_bridge.cpp
    res/application.qrc
)
target_link_libraries(test_native_bridge PRIVATE DevTools_web_frontend)
DevTools_add_test(test_qt_webengine_frontend CORE_ONLY
    SOURCES
    features/web_frontend/tests/test_qt_webengine_frontend.cpp
    res/application.qrc
)
target_link_libraries(test_qt_webengine_frontend PRIVATE DevTools_web_frontend)
# core/data_conversion
DevTools_add_test(test_data_conversion CORE_ONLY
    SOURCES
    features/data_conversion/tests/test_data_conversion.cpp
    tests/test_util.cpp
    tests/random_data.cpp
    features/data_conversion/core/data_conversion.cpp
    features/data_conversion/core/parser/json_parser.cpp
    features/data_conversion/core/parser/toml_parser.cpp
    features/data_conversion/core/parser/yaml_parser.cpp
    features/data_conversion/core/emitter/basic_emitter.cpp
    features/data_conversion/core/emitter/json_emitter.cpp
    features/data_conversion/core/emitter/toml_emitter.cpp
    features/data_conversion/core/emitter/yaml_emitter.cpp
)
target_include_directories(test_data_conversion PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/tests
    ${TOML11_INCLUDE_DIR}
    ${YAML_CPP_INCLUDE_DIR}
)
target_link_libraries(test_data_conversion PRIVATE DevTools_core yaml-cpp::yaml-cpp)
# core/timestamp_conversion
DevTools_add_test(test_timestamp_conversion CORE_ONLY
    SOURCES
    features/timestamp_conversion/tests/test_timestamp_conversion.cpp
    features/timestamp_conversion/core/timestamp_conversion.cpp
)

# features/regex_tool/core
DevTools_add_test(test_regex_tool CORE_ONLY
    SOURCES
    features/regex_tool/tests/test_regex_tool.cpp
    features/regex_tool/core/regex_tool.cpp
)

# Exercise the regex worker without linking or constructing Widgets.
DevTools_add_test(test_regex_worker CORE_ONLY
    SOURCES
    features/regex_tool/tests/test_regex_worker.cpp
    features/regex_tool/core/regex_tool.cpp
    features/regex_tool/core/regex_worker.cpp
    features/regex_tool/core/regex_worker.h
)
DevTools_add_test(test_regex_settings_repository CORE_ONLY
    SOURCES
    features/regex_tool/tests/test_regex_settings_repository.cpp
    features/regex_tool/core/regex_settings_repository.cpp
)
DevTools_add_test(test_regex_evaluation_service CORE_ONLY
    SOURCES
    features/regex_tool/tests/test_regex_evaluation_service.cpp
    features/regex_tool/core/regex_evaluation_service.cpp
    features/regex_tool/core/regex_worker.cpp
    features/regex_tool/core/regex_tool.cpp
)

DevTools_add_test(test_template_reader CORE_ONLY
    SOURCES
    features/phrase_generation/tests/test_template_reader.cpp
    features/phrase_generation/core/template_reader.cpp
)
DevTools_add_test(test_template_repository CORE_ONLY
    SOURCES
    features/phrase_generation/tests/test_template_repository.cpp
    features/phrase_generation/core/template_repository.cpp
    features/phrase_generation/core/template_reader.cpp
)
DevTools_add_test(test_http_service CORE_ONLY
    SOURCES
    features/http_request/tests/test_http_service.cpp
    features/http_request/core/http_service.cpp
    features/http_request/core/http_service.h
)
target_link_libraries(test_http_service PRIVATE Qt6::Network)

DevTools_add_test(test_query_service CORE_ONLY
    SOURCES
    features/db_tool/tests/test_query_service.cpp
    features/db_tool/core/query_service.cpp
)
target_link_libraries(test_query_service PRIVATE Qt6::Sql)
DevTools_add_test(test_database_browse_service CORE_ONLY
    SOURCES
    features/db_tool/tests/test_database_browse_service.cpp
    features/db_tool/core/database_browse_service.cpp
)
target_link_libraries(test_database_browse_service PRIVATE Qt6::Sql)

DevTools_add_test(test_connection_history CORE_ONLY
    SOURCES
    features/db_tool/tests/test_connection_history.cpp
    features/db_tool/core/connection_history.cpp
)

DevTools_add_test(test_database_connection CORE_ONLY
    SOURCES
    features/db_tool/tests/test_database_connection.cpp
    features/db_tool/core/database_connection.cpp
    features/db_tool/core/sqlite_paths.cpp
    features/db_tool/core/sqlite_access_restore.cpp
    features/framework/core/local_file_selection_service.cpp
)
target_link_libraries(test_database_connection PRIVATE Qt6::Sql)
if(APPLE)
    target_sources(test_database_connection PRIVATE features/db_tool/core/sqlite_file_access.mm)
    target_link_libraries(test_database_connection PRIVATE ${FOUNDATION_FRAMEWORK})
else()
    target_sources(test_database_connection PRIVATE features/db_tool/core/sqlite_file_access.cpp)
endif()

DevTools_add_test(test_qr_service CORE_ONLY
    SOURCES
    features/qr_code/tests/test_qr_service.cpp
    features/qr_code/core/content_generator.cpp
    features/qr_code/core/qr_validation.cpp
    features/qr_code/core/qr_image.cpp
    features/qr_code/core/qrcodegen.cpp
)
target_link_libraries(test_qr_service PRIVATE Qt6::Gui)

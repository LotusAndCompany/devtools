find_program(BUN_EXECUTABLE bun REQUIRED)

set(FRONTEND_DIR "${CMAKE_SOURCE_DIR}/frontend")
set(FRONTEND_DIST_DIR "${FRONTEND_DIR}/dist")
set(FRONTEND_INPUTS)
foreach(FRONTEND_PATTERN IN ITEMS
        "${FRONTEND_DIR}/*.html"
        "${FRONTEND_DIR}/*.json"
        "${FRONTEND_DIR}/*.ts"
        "${FRONTEND_DIR}/*.tsx"
        "${FRONTEND_DIR}/*.css"
        "${FRONTEND_DIR}/src/*.ts"
        "${FRONTEND_DIR}/src/*.tsx"
        "${FRONTEND_DIR}/src/*.css")
    file(GLOB FRONTEND_FILES CONFIGURE_DEPENDS "${FRONTEND_PATTERN}")
    list(APPEND FRONTEND_INPUTS ${FRONTEND_FILES})
endforeach()
file(GLOB_RECURSE FRONTEND_SOURCE_FILES CONFIGURE_DEPENDS
    "${FRONTEND_DIR}/src/*.ts"
    "${FRONTEND_DIR}/src/*.tsx"
    "${FRONTEND_DIR}/src/*.css"
)
list(APPEND FRONTEND_INPUTS ${FRONTEND_SOURCE_FILES})

add_custom_command(
    OUTPUT "${FRONTEND_DIST_DIR}/index.html"
           "${FRONTEND_DIST_DIR}/assets/app.js"
           "${FRONTEND_DIST_DIR}/assets/style.css"
    COMMAND "${BUN_EXECUTABLE}" install --frozen-lockfile
    COMMAND "${BUN_EXECUTABLE}" run --bun build
    WORKING_DIRECTORY "${FRONTEND_DIR}"
    DEPENDS ${FRONTEND_INPUTS} "${FRONTEND_DIR}/bun.lock"
    COMMENT "Building embedded React frontend"
    VERBATIM
)

add_custom_target(DevTools_frontend_assets
    DEPENDS "${FRONTEND_DIST_DIR}/index.html"
            "${FRONTEND_DIST_DIR}/assets/app.js"
            "${FRONTEND_DIST_DIR}/assets/style.css"
)

target_sources(DevTools_web_frontend PRIVATE
    gui/main_window.cpp
    gui/native_bridge.h gui/native_bridge.cpp
    gui/bridge/operation_registry.h gui/bridge/operation_registry.cpp
    gui/bridge/bridge_io.h
    gui/bridge/operation_runtime.cpp
    gui/bridge/request_validation.cpp
    gui/bridge/tool_operation_handler.cpp
    gui/handlers/database_state_handler.h gui/handlers/database_state_handler.cpp
    gui/handlers/database_operation_handler.cpp
    gui/handlers/image_state_handler.h gui/handlers/image_state_handler.cpp
    gui/handlers/image_operation_handler.cpp
    gui/handlers/native_settings_handler.cpp
    gui/handlers/text_file_handler.cpp
    gui/qt_webengine_adapter.h gui/qt_webengine_adapter.cpp
    core/operations/command_operations.h core/operations/command_operations.cpp
    core/operations/data_operations.h core/operations/data_operations.cpp
    core/operations/database_operations.h core/operations/database_operations.cpp
    core/operations/diff_operations.h core/operations/diff_operations.cpp
    core/operations/frontend_operation_service.h core/operations/frontend_operation_service.cpp
    core/operations/http_operations.h core/operations/http_operations.cpp
    core/operations/markdown_operations.h core/operations/markdown_operations.cpp
    core/operations/operation_support.h
    core/operations/phrase_operations.h core/operations/phrase_operations.cpp
    core/operations/qr_operations.h core/operations/qr_operations.cpp
    core/operations/regex_operations.h core/operations/regex_operations.cpp
    resources/frontend.qrc
)
target_include_directories(DevTools_web_frontend PRIVATE ${CMAKE_BINARY_DIR})
add_dependencies(DevTools_web_frontend DevTools_app_info)
add_dependencies(DevTools_web_frontend DevTools_frontend_assets)
target_link_libraries(DevTools_web_frontend PUBLIC
    DevTools_command
    DevTools_core
    DevTools_data_conversion
    DevTools_diff_tool
    DevTools_framework_gui
    DevTools_http_request
    DevTools_image_core
    DevTools_markdown_preview
    DevTools_phrase_generation
    DevTools_qr_code_generation
    DevTools_regex_tester
    DevTools_timestamp_conversion
    DevTools_db_tool
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
    Qt6::Concurrent
    Qt6::WebChannel
    Qt6::WebEngineWidgets
)

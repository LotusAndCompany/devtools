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

set(FRONTEND_HOST_ENV)
if(APPLE)
    list(APPEND FRONTEND_HOST_ENV "VITE_DEVTOOLS_HOST=wkwebview")
endif()

add_custom_command(
    OUTPUT "${FRONTEND_DIST_DIR}/index.html"
           "${FRONTEND_DIST_DIR}/assets/app.js"
           "${FRONTEND_DIST_DIR}/assets/style.css"
    COMMAND "${BUN_EXECUTABLE}" install --frozen-lockfile
    COMMAND "${CMAKE_COMMAND}" -E env
            ${FRONTEND_HOST_ENV}
            "${BUN_EXECUTABLE}" run --bun build
    WORKING_DIRECTORY "${FRONTEND_DIR}"
    DEPENDS ${FRONTEND_INPUTS} "${FRONTEND_DIR}/bun.lock"
            "${CMAKE_SOURCE_DIR}/res/dev-tools_logo.png"
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
    gui/bridge/bridge_limits.h
    gui/bridge/bridge_io.h
    gui/bridge/operation_runtime.cpp
    gui/bridge/request_validation.cpp
    gui/bridge/tool_operation_handler.cpp
    gui/handlers/database/database_state_handler.h gui/handlers/database/database_state_handler.cpp
    gui/handlers/database/database_operation_handler.cpp
    gui/handlers/image/image_state_handler.h gui/handlers/image/image_state_handler.cpp
    gui/handlers/image/image_operation_handler.cpp
    gui/handlers/settings/native_settings_handler.cpp
    gui/handlers/files/text_file_handler.cpp
    gui/bridge/operations/command/command_operations.h gui/bridge/operations/command/command_operations.cpp
    gui/bridge/operations/data/data_operations.h gui/bridge/operations/data/data_operations.cpp
    gui/bridge/operations/database/database_operations.h gui/bridge/operations/database/database_operations.cpp
    gui/bridge/operations/diff/diff_operations.h gui/bridge/operations/diff/diff_operations.cpp
    gui/bridge/operations/frontend_operation_service.h gui/bridge/operations/frontend_operation_service.cpp
    gui/bridge/operations/http/http_operations.h gui/bridge/operations/http/http_operations.cpp
    gui/bridge/operations/markdown/markdown_operations.h gui/bridge/operations/markdown/markdown_operations.cpp
    gui/bridge/operations/operation_support.h
    gui/bridge/operations/phrase/phrase_operations.h gui/bridge/operations/phrase/phrase_operations.cpp
    gui/bridge/operations/qr/qr_operations.h gui/bridge/operations/qr/qr_operations.cpp
    gui/bridge/operations/regex/regex_operations.h gui/bridge/operations/regex/regex_operations.cpp
    resources/frontend.qrc
)

if(APPLE)
    target_sources(DevTools_web_frontend PRIVATE
        gui/wk_webview_adapter.h gui/wk_webview_adapter.mm
    )
    set_source_files_properties(gui/wk_webview_adapter.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
    set_target_properties(DevTools_web_frontend PROPERTIES
        OBJCXX_STANDARD 17
        OBJCXX_STANDARD_REQUIRED YES
    )
    find_library(DEVTOOLS_APPKIT_FRAMEWORK AppKit REQUIRED)
    find_library(DEVTOOLS_WEBKIT_FRAMEWORK WebKit REQUIRED)
    target_link_libraries(DevTools_web_frontend PUBLIC
        "${DEVTOOLS_APPKIT_FRAMEWORK}"
        "${DEVTOOLS_WEBKIT_FRAMEWORK}"
    )
else()
    target_sources(DevTools_web_frontend PRIVATE
        gui/qt_webengine_adapter.h gui/qt_webengine_adapter.cpp
    )
    target_link_libraries(DevTools_web_frontend PUBLIC Qt6::WebChannel Qt6::WebEngineWidgets)
endif()
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
)

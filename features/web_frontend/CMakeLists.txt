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
        "${FRONTEND_DIR}/components/*.ts"
        "${FRONTEND_DIR}/components/*.tsx"
        "${FRONTEND_DIR}/components/ui/*.ts"
        "${FRONTEND_DIR}/components/ui/*.tsx"
        "${FRONTEND_DIR}/lib/*.ts"
        "${FRONTEND_DIR}/lib/*.tsx"
        "${FRONTEND_DIR}/tests/*.ts"
        "${FRONTEND_DIR}/tests/*.tsx")
    file(GLOB FRONTEND_FILES CONFIGURE_DEPENDS "${FRONTEND_PATTERN}")
    list(APPEND FRONTEND_INPUTS ${FRONTEND_FILES})
endforeach()

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
    gui/frontend_operation_service.h gui/frontend_operation_service.cpp
    gui/native_bridge.h gui/native_bridge.cpp
    gui/qt_webengine_adapter.h gui/qt_webengine_adapter.cpp
    gui/react_bridge_demo_widget.h gui/react_bridge_demo_widget.cpp
    resources/frontend.qrc
)
add_dependencies(DevTools_web_frontend DevTools_frontend_assets)
target_link_libraries(DevTools_web_frontend PUBLIC
    DevTools_command
    DevTools_core
    DevTools_data_conversion
    DevTools_diff_tool
    DevTools_http_request
    DevTools_image_core
    DevTools_image_tools_unified
    DevTools_markdown_preview
    DevTools_phrase_generation
    DevTools_qr_code_generation
    DevTools_regex_tester
    DevTools_timestamp_conversion
    DevTools_db_tool
    Qt6::Concurrent
    Qt6::WebChannel
    Qt6::WebEngineWidgets
)

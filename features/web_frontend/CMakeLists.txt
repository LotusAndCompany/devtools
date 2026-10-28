find_program(NPM_EXECUTABLE npm REQUIRED)

set(FRONTEND_DIR "${CMAKE_SOURCE_DIR}/frontend")
set(FRONTEND_DIST_DIR "${FRONTEND_DIR}/dist")
file(GLOB_RECURSE FRONTEND_INPUTS CONFIGURE_DEPENDS
    "${FRONTEND_DIR}/*"
)
list(FILTER FRONTEND_INPUTS EXCLUDE REGEX "/(node_modules|dist)/")

add_custom_command(
    OUTPUT "${FRONTEND_DIST_DIR}/index.html"
           "${FRONTEND_DIST_DIR}/assets/app.js"
           "${FRONTEND_DIST_DIR}/assets/style.css"
    COMMAND "${NPM_EXECUTABLE}" ci --no-audit --no-fund
    COMMAND "${NPM_EXECUTABLE}" run build
    WORKING_DIRECTORY "${FRONTEND_DIR}"
    DEPENDS ${FRONTEND_INPUTS}
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

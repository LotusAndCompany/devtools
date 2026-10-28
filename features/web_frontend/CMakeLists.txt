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
    gui/native_bridge.h gui/native_bridge.cpp
    gui/qt_webengine_adapter.h gui/qt_webengine_adapter.cpp
    gui/react_bridge_demo_widget.h gui/react_bridge_demo_widget.cpp
    resources/frontend.qrc
)
add_dependencies(DevTools_web_frontend DevTools_frontend_assets)
target_link_libraries(DevTools_web_frontend PUBLIC
    DevTools_data_conversion
    Qt6::Concurrent
    Qt6::WebChannel
    Qt6::WebEngineWidgets
)

# Database tool module (DevTools_db_tool)
# SQLite / MySQL / PostgreSQL database management

target_sources(DevTools_db_tool PRIVATE
    gui/connection_selector/connection_selector.h gui/connection_selector/connection_selector.cpp
    gui/connection_window/connection_window.h gui/connection_window/connection_window.cpp
    gui/db_main/db_main.h gui/db_main/db_main.cpp
    gui/query_page/query_page.h gui/query_page/query_page.cpp
)

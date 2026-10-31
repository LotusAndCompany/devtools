# Database tool module (DevTools_db_tool)
# SQLite / MySQL / PostgreSQL database management

target_sources(DevTools_db_tool PRIVATE
    core/sqlite_file_access.h core/sqlite_access_restore.cpp
    core/query_service.h core/query_service.cpp
    core/database_browse_service.h core/database_browse_service.cpp
    core/database_connection.h core/database_connection.cpp
    core/sqlite_paths.h core/sqlite_paths.cpp
    core/connection_history.h core/connection_history.cpp
)

if(APPLE)
    target_sources(DevTools_db_tool PRIVATE core/sqlite_file_access.mm)
    find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
    find_library(APPKIT_FRAMEWORK AppKit REQUIRED)
    find_library(UNIFORM_TYPE_IDENTIFIERS_FRAMEWORK UniformTypeIdentifiers REQUIRED)
    target_link_libraries(DevTools_db_tool PRIVATE ${FOUNDATION_FRAMEWORK} ${APPKIT_FRAMEWORK}
                                                   ${UNIFORM_TYPE_IDENTIFIERS_FRAMEWORK})
else()
    target_sources(DevTools_db_tool PRIVATE core/sqlite_file_access.cpp)
endif()

# Feature modules
# Target definitions (qt_add_library / qt_add_executable) live in root CMakeLists.txt.
# Each feature directory lists its sources via target_sources().

add_subdirectory(framework)
add_subdirectory(data_conversion)
add_subdirectory(qr_code)
add_subdirectory(markdown_preview)
add_subdirectory(image)
add_subdirectory(db_tool)
add_subdirectory(http_request)
add_subdirectory(command)
add_subdirectory(regex_tool)
add_subdirectory(phrase_generation)

# Regex tester module (DevTools_regex_tester)

target_sources(DevTools_regex_tester PRIVATE
    # Core
    core/regex_tool.h core/regex_tool.cpp
    core/regex_worker.h core/regex_worker.cpp
    core/regex_settings_repository.h core/regex_settings_repository.cpp
    # GUI
    gui/regex_tester_gui.h gui/regex_tester_gui.cpp
)

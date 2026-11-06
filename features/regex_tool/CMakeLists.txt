# Regex tester module (DevTools_regex_tester)

target_sources(DevTools_regex_tester PRIVATE
    # Core
    core/regex_tool.h core/regex_tool.cpp
    core/regex_worker.h core/regex_worker.cpp
    core/regex_evaluation_service.h core/regex_evaluation_service.cpp
    core/regex_settings_repository.h core/regex_settings_repository.cpp
)

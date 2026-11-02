# Framework module (DevTools_core)
# Application infrastructure shared by all features

target_sources(DevTools_core PRIVATE
    # Core
    core/application_settings_service.h core/application_settings_service.cpp
    core/local_file_selection_service.h core/local_file_selection_service.cpp
    core/application_mixin.h core/application_mixin.cpp
    core/enum_cast.h
    core/tool/tool.h core/tool/tool.cpp
    core/tool/tool_id_fields.h
    core/exception/common_exception.h
    core/exception/invalid_argument_exception.h
    core/exception/invalid_state_exception.h
    core/exception/out_of_range_exception.h
    core/exception/under_development_exception.h
)

target_sources(DevTools_framework_gui PRIVATE
    # Qt Widgets application host
    gui/main_window.h
    gui/gui_application.h gui/gui_application.cpp
)

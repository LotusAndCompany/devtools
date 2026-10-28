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
    # GUI
    gui/icon_utils.h gui/icon_utils.cpp
    gui/main_window.h
    gui/gui_application.h gui/gui_application.cpp
    gui/design_system.h gui/design_system.cpp
    gui/gui_tool.h gui/gui_tool.cpp
    gui/sidemenu.h gui/sidemenu.cpp
    gui/sidemenu_item.h gui/sidemenu_item.cpp
    gui/welcome_page.h gui/welcome_page.cpp
    gui/menubar/about_devtools_dialog.h gui/menubar/about_devtools_dialog.cpp
    gui/menubar/settings_dialog.h gui/menubar/settings_dialog.cpp
)

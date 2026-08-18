# Image processing modules
# DevTools_image_core: core image logic + basic GUI widgets
# DevTools_image_tools_unified: unified image tools including transparency

target_sources(DevTools_image_core PRIVATE
    core/basic_image_io.h core/basic_image_io.cpp
    core/basic_image_edit_interface.h core/basic_image_edit_interface.cpp
    gui/basic/image_view.h gui/basic/image_view.cpp
    gui/basic/file_dialogs.h gui/basic/file_dialogs.cpp
    gui/basic/control.h gui/basic/control.cpp
)

target_sources(DevTools_image_tools_unified PRIVATE
    gui/basic/image_tools_unified_gui.h gui/basic/image_tools_unified_gui.cpp
    gui/transparent/color_sample.h gui/transparent/color_sample.cpp
    gui/transparent/image_view_for_image_transparent.h gui/transparent/image_view_for_image_transparent.cpp
)

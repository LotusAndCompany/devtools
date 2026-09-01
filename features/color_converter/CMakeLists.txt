# color_converter feature module
# Target (qt_add_library(${PROJECT_NAME}_color_converter STATIC)) is defined in the
# root CMakeLists.txt, alongside the other feature modules. This file only lists
# the sources that belong to it, following the same pattern as the other
# features/*/CMakeLists.txt files.

target_sources(${PROJECT_NAME}_color_converter
    PRIVATE
        core/color_converter.h
        core/color_converter.cpp
        gui/color_converter.h
        gui/color_converter.cpp
)

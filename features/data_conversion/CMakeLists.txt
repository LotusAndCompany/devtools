# Data conversion module (DevTools_data_conversion)
# JSON / YAML / TOML conversion

target_sources(DevTools_data_conversion PRIVATE
    # Core
    core/data_conversion.h core/data_conversion.cpp
    core/parser/basic_parser.h
    core/parser/json_parser.h core/parser/json_parser.cpp
    core/parser/toml_parser.h core/parser/toml_parser.cpp
    core/parser/yaml_parser.h core/parser/yaml_parser.cpp
    core/emitter/basic_emitter.h core/emitter/basic_emitter.cpp
    core/emitter/json_emitter.h core/emitter/json_emitter.cpp
    core/emitter/toml_emitter.h core/emitter/toml_emitter.cpp
    core/emitter/yaml_emitter.h core/emitter/yaml_emitter.cpp
    # GUI
    gui/data_conversion_gui.h gui/data_conversion_gui.cpp
)

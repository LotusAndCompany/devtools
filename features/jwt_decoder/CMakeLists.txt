# JWT decoder module (DevTools_jwt_decoder)

target_sources(DevTools_jwt_decoder PRIVATE
    core/jwt_decoder.h core/jwt_decoder.cpp
    gui/jwt_decoder_gui.h gui/jwt_decoder_gui.cpp
    gui/jwt_highlighters.h gui/jwt_highlighters.cpp
    gui/jwt_palette.h gui/jwt_palette.cpp
)

# QR code generation module (DevTools_qr_code_generation)

target_sources(DevTools_qr_code_generation PRIVATE
    core/content_generator.h core/content_generator.cpp
    core/qrcodegen.hpp core/qrcodegen.cpp
    gui/qr_code_generation_gui.h gui/qr_code_generation_gui.cpp
)

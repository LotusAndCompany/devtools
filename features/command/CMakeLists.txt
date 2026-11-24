# Shell command execution module (DevTools_command)

target_sources(DevTools_command PRIVATE
    gui/command.h gui/command.cpp
    core/command_service.cpp core/command_service.h
    core/command_function.h
    core/command_option.h
)

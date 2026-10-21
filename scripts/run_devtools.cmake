if(NOT DEFINED DEVTOOLS_BINARY)
    message(FATAL_ERROR "DEVTOOLS_BINARY must point to the development executable")
endif()

find_program(PGREP_EXECUTABLE pgrep REQUIRED)
find_program(LSOF_EXECUTABLE lsof REQUIRED)
find_program(KILL_EXECUTABLE kill REQUIRED)

file(REAL_PATH "${DEVTOOLS_BINARY}" DEVTOOLS_BINARY)
file(REAL_PATH "${CMAKE_CURRENT_LIST_DIR}/.." DEVTOOLS_PROJECT_ROOT)

execute_process(
    COMMAND "${PGREP_EXECUTABLE}" -x DevTools
    OUTPUT_VARIABLE DEVTOOLS_PIDS
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE PGREP_RESULT
)

set(STOPPED_PIDS)
if(PGREP_RESULT EQUAL 0)
    string(REPLACE "\n" ";" DEVTOOLS_PID_LIST "${DEVTOOLS_PIDS}")
    foreach(DEVTOOLS_PID IN LISTS DEVTOOLS_PID_LIST)
        execute_process(
            COMMAND "${LSOF_EXECUTABLE}" -a -p "${DEVTOOLS_PID}" -d txt -Fn
            OUTPUT_VARIABLE OPEN_EXECUTABLES
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )

        string(REPLACE "\n" ";" OPEN_EXECUTABLE_LIST "${OPEN_EXECUTABLES}")
        set(PROCESS_EXECUTABLE)
        foreach(OPEN_EXECUTABLE IN LISTS OPEN_EXECUTABLE_LIST)
            if(OPEN_EXECUTABLE MATCHES "^n")
                string(REGEX REPLACE "^n" "" PROCESS_EXECUTABLE "${OPEN_EXECUTABLE}")
                break()
            endif()
        endforeach()

        if(PROCESS_EXECUTABLE STREQUAL "${DEVTOOLS_BINARY}")
            list(APPEND STOPPED_PIDS "${DEVTOOLS_PID}")
            message(STATUS "Replacing existing canonical DevTools process ${DEVTOOLS_PID}")
        else()
            string(FIND "${PROCESS_EXECUTABLE}" "${DEVTOOLS_PROJECT_ROOT}/" PROJECT_ROOT_INDEX)
            if(PROJECT_ROOT_INDEX EQUAL 0)
                list(APPEND STOPPED_PIDS "${DEVTOOLS_PID}")
                message(STATUS "Replacing existing project DevTools process ${DEVTOOLS_PID}: ${PROCESS_EXECUTABLE}")
            else()
                message(FATAL_ERROR
                        "Another DevTools process is running outside the source tree: "
                        "${DEVTOOLS_PID} (${PROCESS_EXECUTABLE}). Close it before running the "
                        "development build.")
            endif()
        endif()
    endforeach()
endif()

foreach(DEVTOOLS_PID IN LISTS STOPPED_PIDS)
    execute_process(
        COMMAND "${KILL_EXECUTABLE}" -TERM "${DEVTOOLS_PID}"
        ERROR_QUIET
        RESULT_VARIABLE STOP_RESULT
    )
    if(NOT STOP_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to stop DevTools process ${DEVTOOLS_PID}")
    endif()

    set(PROCESS_RUNNING TRUE)
    foreach(ATTEMPT RANGE 1 100)
        execute_process(
            COMMAND "${KILL_EXECUTABLE}" -0 "${DEVTOOLS_PID}"
            ERROR_QUIET
            RESULT_VARIABLE RUNNING_RESULT
        )
        if(NOT RUNNING_RESULT EQUAL 0)
            set(PROCESS_RUNNING FALSE)
            break()
        endif()
        execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 0.1)
    endforeach()

    if(PROCESS_RUNNING)
        message(FATAL_ERROR "Timed out while stopping DevTools process ${DEVTOOLS_PID}")
    endif()
endforeach()

message(STATUS "Starting canonical DevTools binary: ${DEVTOOLS_BINARY}")
execute_process(
    COMMAND "${DEVTOOLS_BINARY}"
    RESULT_VARIABLE DEVTOOLS_RESULT
)

if(NOT DEVTOOLS_RESULT EQUAL 0)
    message(FATAL_ERROR "DevTools exited with status ${DEVTOOLS_RESULT}")
endif()

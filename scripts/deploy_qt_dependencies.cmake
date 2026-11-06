if(DEFINED ENV{TARGET_BUILD_DIR} AND DEFINED ENV{FULL_PRODUCT_NAME})
    set(APP_BUNDLE "$ENV{TARGET_BUILD_DIR}/$ENV{FULL_PRODUCT_NAME}")
else()
    set(APP_BUNDLE "${APP_BUNDLE_PATH}")
endif()

if(NOT EXISTS "${APP_BUNDLE}")
    message(FATAL_ERROR "Could not find app bundle \"${APP_BUNDLE}\"")
endif()

execute_process(
    COMMAND "${MACDEPLOYQT_EXECUTABLE}" "${APP_BUNDLE}" ${MACDEPLOYQT_OPTIONS}
    RESULT_VARIABLE MACDEPLOYQT_RESULT
)

if(NOT MACDEPLOYQT_RESULT EQUAL 0)
    message(FATAL_ERROR "macdeployqt failed with exit code ${MACDEPLOYQT_RESULT}")
endif()

if(QT_DSYM_ARCHIVING_WORKAROUND_ENABLED)
    set(DWARF_DSYM_FOLDER "$ENV{DWARF_DSYM_FOLDER_PATH}")
    if(DWARF_DSYM_FOLDER STREQUAL "")
        message(FATAL_ERROR "Xcode did not provide DWARF_DSYM_FOLDER_PATH during archive")
    endif()

    find_program(DWARFDUMP_EXECUTABLE NAMES dwarfdump REQUIRED)
    file(GLOB_RECURSE QT_DSYM_BUNDLES LIST_DIRECTORIES true
        "${QT_INSTALL_PREFIX}/*.dSYM")
    file(GLOB_RECURSE APP_FILES LIST_DIRECTORIES false "${APP_BUNDLE}/*")
    file(MAKE_DIRECTORY "${DWARF_DSYM_FOLDER}")

    set(QT_DSYM_DWARF_FILES)
    foreach(QT_DSYM_BUNDLE IN LISTS QT_DSYM_BUNDLES)
        file(GLOB QT_DSYM_CONTENTS LIST_DIRECTORIES false
            "${QT_DSYM_BUNDLE}/Contents/Resources/DWARF/*")
        list(APPEND QT_DSYM_DWARF_FILES ${QT_DSYM_CONTENTS})
    endforeach()

    set(COPIED_QT_DSYM_BUNDLES)
    foreach(APP_FILE IN LISTS APP_FILES)
        get_filename_component(APP_FILE_NAME "${APP_FILE}" NAME)
        set(MATCHING_QT_DSYM_FOUND false)
        set(QT_DSYM_UUID_MATCHED false)

        foreach(QT_DWARF_FILE IN LISTS QT_DSYM_DWARF_FILES)
            get_filename_component(QT_DWARF_NAME "${QT_DWARF_FILE}" NAME)
            get_filename_component(QT_DWARF_DIR "${QT_DWARF_FILE}" DIRECTORY)
            get_filename_component(QT_DSYM_BUNDLE "${QT_DWARF_DIR}/../../.." ABSOLUTE)
            get_filename_component(QT_DSYM_NAME "${QT_DSYM_BUNDLE}" NAME)
            string(REGEX REPLACE "\\.dSYM$" "" QT_DSYM_NAME "${QT_DSYM_NAME}")

            if(NOT APP_FILE_NAME STREQUAL QT_DWARF_NAME AND
               NOT APP_FILE_NAME STREQUAL QT_DSYM_NAME)
                continue()
            endif()
            set(MATCHING_QT_DSYM_FOUND true)

            execute_process(
                COMMAND "${DWARFDUMP_EXECUTABLE}" --uuid "${APP_FILE}"
                RESULT_VARIABLE APP_UUID_RESULT
                OUTPUT_VARIABLE APP_UUID_OUTPUT
                ERROR_QUIET
            )
            if(NOT APP_UUID_RESULT STREQUAL "0")
                continue()
            endif()
            execute_process(
                COMMAND "${DWARFDUMP_EXECUTABLE}" --uuid "${QT_DWARF_FILE}"
                RESULT_VARIABLE QT_UUID_RESULT
                OUTPUT_VARIABLE QT_UUID_OUTPUT
                ERROR_QUIET
            )
            if(NOT QT_UUID_RESULT STREQUAL "0")
                continue()
            endif()

            string(REGEX MATCHALL "UUID: [0-9A-Fa-f-]+ \\([^)]+\\)"
                APP_UUIDS "${APP_UUID_OUTPUT}")
            string(REGEX MATCHALL "UUID: [0-9A-Fa-f-]+ \\([^)]+\\)"
                QT_UUIDS "${QT_UUID_OUTPUT}")
            list(SORT APP_UUIDS)
            list(SORT QT_UUIDS)
            if(NOT APP_UUIDS STREQUAL QT_UUIDS)
                continue()
            endif()

            list(FIND COPIED_QT_DSYM_BUNDLES "${QT_DSYM_BUNDLE}" DSYM_INDEX)
            if(DSYM_INDEX EQUAL -1)
                file(REMOVE_RECURSE "${DWARF_DSYM_FOLDER}/${QT_DSYM_NAME}")
                file(COPY "${QT_DSYM_BUNDLE}" DESTINATION "${DWARF_DSYM_FOLDER}")
                list(APPEND COPIED_QT_DSYM_BUNDLES "${QT_DSYM_BUNDLE}")
            endif()
            set(QT_DSYM_UUID_MATCHED true)
            break()
        endforeach()

        if(MATCHING_QT_DSYM_FOUND AND NOT QT_DSYM_UUID_MATCHED)
            message(FATAL_ERROR "Could not find a UUID-matching Qt dSYM for ${APP_FILE}")
        endif()
    endforeach()

    list(LENGTH COPIED_QT_DSYM_BUNDLES QT_DSYM_COUNT)
    if(QT_DSYM_COUNT EQUAL 0)
        message(FATAL_ERROR "No Qt dSYMs matched the archived app binaries")
    endif()
    message(STATUS "Copied ${QT_DSYM_COUNT} UUID-matching Qt dSYM bundles for the archive")
endif()

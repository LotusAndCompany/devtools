if(NOT DEFINED TRANSLATION_TS_FILE)
    message(FATAL_ERROR "TRANSLATION_TS_FILE is required")
endif()

if(NOT EXISTS "${TRANSLATION_TS_FILE}")
    message(FATAL_ERROR "Translation file does not exist: ${TRANSLATION_TS_FILE}")
endif()

file(READ "${TRANSLATION_TS_FILE}" TRANSLATION_CONTENT)

if(TRANSLATION_CONTENT MATCHES "<translation[^>]*type=\"unfinished\"")
    message(FATAL_ERROR "Incomplete translations found in ${TRANSLATION_TS_FILE}")
endif()

if(TRANSLATION_CONTENT MATCHES "<translation>[ \t\r\n]*</translation>" OR
   TRANSLATION_CONTENT MATCHES "<translation[ \t]*/>")
    message(FATAL_ERROR "Empty translations found in ${TRANSLATION_TS_FILE}")
endif()

message(STATUS "No incomplete translations found in ${TRANSLATION_TS_FILE}")

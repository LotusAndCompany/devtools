# DevTools Docmentation

# set input and output files
set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/Doxyfile.in)
set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/doxygen/Doxyfile)
set(DOXYGEN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# copy icon
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/dev-tools_icon.doxygen.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/doxygen/)

# request to configure the file
configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
message("Doxygen build started")

# note the option ALL which allows to build the docs together with the application
add_custom_target(${PROJECT_NAME}_docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
)

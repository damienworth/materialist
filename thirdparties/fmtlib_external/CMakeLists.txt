# download and unpack fmtlib at configure time
configure_file(${PROJECT_SOURCE_DIR}/cmake/fmtlib.txt.in ${CMAKE_BINARY_DIR}/fmtlib-download/CMakeLists.txt)

execute_process(
    COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/fmtlib-download)

if(result)
    message(FATAL_ERROR "CMake step for fmtlib failed: ${result}")
endif()

execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/fmtlib-download)

if(result)
    message(FATAL_ERROR "Build step for fmtlib failed: ${result}")
endif()

# add fmtlib directly to our build. This defines fmt::fmt target.
add_subdirectory(
    ${CMAKE_BINARY_DIR}/fmtlib-src
    ${CMAKE_BINARY_DIR}/fmtlib-build
    EXCLUDE_FROM_ALL)

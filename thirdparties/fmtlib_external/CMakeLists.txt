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

# prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# add fmtlib directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(
    ${CMAKE_BINARY_DIR}/fmtlib-src
    ${CMAKE_BINARY_DIR}/fmtlib-build
    EXCLUDE_FROM_ALL)

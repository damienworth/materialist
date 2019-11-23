# download and unpack spdlog at configure time
configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/spdlog.txt.in ${CMAKE_BINARY_DIR}/spdlog-download/CMakeLists.txt)

execute_process(
    COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/spdlog-download)

if(result)
    message(FATAL_ERROR "CMake step for spdlog failed: ${result}")
endif(result)

execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/spdlog-download)

if(result)
    message(FATAL_ERROR "Build step for spdlog failed: ${result}")
endif(result)

add_subdirectory(
    ${CMAKE_BINARY_DIR}/spdlog-src
    ${CMAKE_BINARY_DIR}/spdlog-build
    EXCLUDE_FROM_ALL)

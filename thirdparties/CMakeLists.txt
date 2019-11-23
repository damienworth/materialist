# download and unpack spdlog at configure time
configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/spdlog.txt.in ${CMAKE_CURRENT_BINARY_DIR}/spdlog-download/CMakeLists.txt)

execute_process(
    COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/spdlog-download)

if(result)
    message(FATAL_ERROR "CMake step for spdlog failed: ${result}")
endif()

execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/spdlog-download)

if(result)
    message(FATAL_ERROR "Build step for spdlog failed: ${result}")
endif()

add_subdirectory(
    ${CMAKE_CURRENT_BINARY_DIR}/spdlog-src
    ${CMAKE_CURRENT_BINARY_DIR}/spdlog-build
    EXCLUDE_FROM_ALL)

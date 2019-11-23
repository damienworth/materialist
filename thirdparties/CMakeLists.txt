add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/googletest_external
    ${CMAKE_CURRENT_BINARY_DIR}/googletest_external-build
    EXCLUDE_FROM_ALL)

add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/spdlog_external
    ${CMAKE_CURRENT_BINARY_DIR}/spdlog_external-build
    EXCLUDE_FROM_ALL)

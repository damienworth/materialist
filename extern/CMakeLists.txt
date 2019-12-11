add_subdirectory(fmt ${CMAKE_CURRENT_BINARY_DIR}/fmt EXCLUDE_FROM_ALL)
set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "Don't build bundled fmtlib")
add_subdirectory(spdlog ${CMAKE_CURRENT_BINARY_DIR}/spdlog EXCLUDE_FROM_ALL)
add_subdirectory(glfw ${CMAKE_CURRENT_BINARY_DIR}/glfw EXCLUDE_FROM_ALL)
add_subdirectory(glm ${CMAKE_CURRENT_BINARY_DIR}/glm EXCLUDE_FROM_ALL)
add_subdirectory(gsl ${CMAKE_CURRENT_BINARY_DIR}/gsl EXCLUDE_FROM_ALL)


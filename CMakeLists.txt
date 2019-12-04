cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

project(materialist VERSION 0.1.0 LANGUAGES CXX)

find_package(glfw3 REQUIRED)
find_package(glm 0.9.9 REQUIRED)
find_package(Vulkan REQUIRED)

find_program(GLSLC NAMES glslc)
if (NOT GLSLC)
    message(FATAL_ERROR "glslc not found")
endif (NOT GLSLC)

if (UNIX AND NOT APPLE)
    find_program(BASH NAMES bash)
    if (NOT BASH)
        message(FATAL_ERROR "bash not found")
    endif (NOT BASH)
endif (UNIX AND NOT APPLE)

add_executable(materialist
    src/application_hello_triangle.cpp
    src/application_hello_triangle.hpp
    src/fmtlib_all.hpp
    src/main.cpp
    src/spdlog_all.hpp)

set(SHADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADERS_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

if (UNIX AND NOT APPLE)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/compile_shaders.sh.in ${CMAKE_CURRENT_BINARY_DIR}/compile_shaders.sh @ONLY)

add_custom_command(
TARGET materialist
PRE_BUILD
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
VERBATIM USES_TERMINAL
COMMENT "compiling shaders"
BYPRODUCTS vert.spv frag.spv
COMMAND ${BASH} ./compile_shaders.sh)
endif (UNIX AND NOT APPLE)

target_include_directories(materialist
PRIVATE
    src)

target_compile_features(materialist
PRIVATE
    cxx_std_17)

target_compile_definitions(materialist
PRIVATE
    GLFW_INCLUDE_VULKAN
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_FORCE_RADIANS)

if (UNIX AND NOT APPLE)
    target_compile_options(materialist
    PRIVATE
        -Wall
        -Werror
        -Wextra
        -pedantic
        -Wconversion
        -Winit-self
        -Woverloaded-virtual
        -Wunreachable-code
        -Wold-style-cast
        -Wsign-promo
        -Wshadow)
endif (UNIX AND NOT APPLE)

target_link_libraries(materialist
PRIVATE
    fmt::fmt
    spdlog::spdlog
    glm
    glfw
    Vulkan::Vulkan)

add_subdirectory(thirdparties EXCLUDE_FROM_ALL)

if (NOT CMAKE_CROSSCOMPILING)

    enable_testing()
    add_subdirectory(tests)

endif (NOT CMAKE_CROSSCOMPILING)

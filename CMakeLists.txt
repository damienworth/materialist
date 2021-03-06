cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

project(materialist VERSION 0.1.0 LANGUAGES CXX)

include(ProcessorCount)
ProcessorCount(CORES_NUM)

if(CORES_NUM EQUAL 0)
    set(CORES_NUM 1)
endif(CORES_NUM EQUAL 0)

find_package(Vulkan REQUIRED)

find_program(GLSLC NAMES glslc)
if (NOT GLSLC)
    message(FATAL_ERROR "glslc not found")
endif (NOT GLSLC)

find_program(MAKE NAMES make)
if (NOT MAKE)
    message(FATAL_ERROR "make not found")
endif (NOT MAKE)

add_subdirectory(extern)

add_executable(materialist
    src/application.hpp
    src/error_handling.cpp
    src/error_handling.hpp
    src/fmtlib_all.hpp
    src/main.cpp
    src/materialist.hpp
    src/spdlog_all.hpp
    src/vulkan_context.hpp)

set(SHADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADERS_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/compile_shaders.in ${CMAKE_CURRENT_BINARY_DIR}/compile_shaders.makefile @ONLY)

add_custom_target(compile_shaders ALL
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
VERBATIM USES_TERMINAL
COMMENT "compiling shaders"
COMMAND ${MAKE} -j${CORES_NUM} -f ${CMAKE_CURRENT_BINARY_DIR}/compile_shaders.makefile)

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
    GLM_FORCE_RADIANS
    VK_NO_PROTOTYPES
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    VULKAN_HPP_NO_EXCEPTIONS)

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
    # asan
    # ubsan
    fmt::fmt
    spdlog::spdlog_header_only
    glm_static
    glfw
    Vulkan::Vulkan)

if (NOT CMAKE_CROSSCOMPILING)

    enable_testing()
    add_subdirectory(tests)

endif (NOT CMAKE_CROSSCOMPILING)

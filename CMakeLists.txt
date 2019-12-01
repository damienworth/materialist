cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

project(materialist VERSION 0.1.0 LANGUAGES CXX)

find_package(glfw3 REQUIRED)
find_package(glm 0.9.9 REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(materialist
    src/application_hello_triangle.cpp
    src/application_hello_triangle.hpp
    src/fmtlib_all.hpp
    src/main.cpp
    src/spdlog_all.hpp)

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

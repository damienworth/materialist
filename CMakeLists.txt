cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

project(materialist VERSION 0.1.0 LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

find_package(GLEW REQUIRED)
find_package(glm 0.9.9 REQUIRED)
find_package(GLUT REQUIRED)
find_package(OpenGL REQUIRED)

if (NOT OPENGL_FOUND)
    message(FATAL_ERROR "couldn't find opengl")
endif (NOT OPENGL_FOUND)

if (NOT OPENGL_GLU_FOUND)
    message(FATAL_ERROR "couldn't find GLU")
endif (NOT OPENGL_GLU_FOUND)

add_executable(materialist
    src/spdlog_all.hpp
    src/error_callback.cpp
    src/error_callback.hpp
    src/main.cpp)

target_include_directories(materialist
PRIVATE
    src
    ${GLFW_INCLUDE_DIRS})

target_compile_features(materialist
PRIVATE
    cxx_std_17)

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
    spdlog::spdlog
    glm
    GLUT::GLUT
    OpenGL::GL
    OpenGL::GLU
    GLEW
    ${GLFW_STATIC_LIBRARIES})

add_subdirectory(thirdparties EXCLUDE_FROM_ALL)

if (NOT CMAKE_CROSSCOMPILING)

    enable_testing()
    add_subdirectory(tests)

endif (NOT CMAKE_CROSSCOMPILING)

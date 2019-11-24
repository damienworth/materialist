#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <string_view>

#include "opengl_all.hpp"

namespace main_window {

// window size
extern int _width;
extern int _height;

// window handle
extern GLFWwindow* _window;

bool create(std::string_view, int, int) noexcept;

void resize_callback(GLFWwindow*, int, int) noexcept;

void loop(GLuint, GLuint) noexcept;

} // namespace main_window

#endif // MAIN_WINDOW_HPP

#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <string_view>

#include "opengl_all.hpp"

namespace mw {

// window size
extern int _width;
extern int _height;

// window handle
extern GLFWwindow* _window;

bool create_window(std::string_view, int, int) noexcept;

void window_size_callback(GLFWwindow*, int, int) noexcept;

void main_loop(GLuint, GLuint) noexcept;

} // namespace mw

#endif // MAIN_WINDOW_HPP

#ifndef FPS_COUNTER_HPP
#define FPS_COUNTER_HPP

#include <string_view>

#include "opengl_all.hpp"

namespace fps_counter {

void update(GLFWwindow*, std::string_view) noexcept;

}

#endif // FPS_COUNTER_HPP

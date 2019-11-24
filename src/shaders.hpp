#ifndef SHADERS_HPP
#define SHADERS_HPP

#include <string_view>

#include "opengl_all.hpp"

namespace shaders {

GLuint load_vertex_shader(std::string_view) noexcept;
GLuint load_fragment_shader(std::string_view) noexcept;
GLuint create_programme(std::string_view, std::string_view) noexcept;

} // namespace shaders

#endif // SHADERS_HPP

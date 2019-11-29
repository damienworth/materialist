#ifndef SHADERS_HPP
#define SHADERS_HPP

#include <optional>
#include <string_view>

#include <glm/glm.hpp>

#include "opengl_all.hpp"

namespace shaders {

std::optional<GLuint> load_vertex_shader(std::string_view) noexcept;
std::optional<GLuint> load_fragment_shader(std::string_view) noexcept;

std::optional<GLuint>
    create_programme(std::string_view, std::string_view) noexcept;

} // namespace shaders

#endif // SHADERS_HPP

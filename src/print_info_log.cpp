#include "print_info_log.hpp"

#include "fmtlib_all.hpp"
#include "spdlog_all.hpp"

namespace shaders {

using spdlog::debug;

namespace {

constexpr auto max_length = 2048;

const char*
GL_type_to_string(GLenum type)
{
    switch (type) {
    case GL_BOOL: return "bool";
    case GL_FLOAT: return "float";
    case GL_FLOAT_MAT2: return "mat2";
    case GL_FLOAT_MAT3: return "mat3";
    case GL_FLOAT_MAT4: return "mat4";
    case GL_FLOAT_VEC2: return "vec2";
    case GL_FLOAT_VEC3: return "vec3";
    case GL_FLOAT_VEC4: return "vec4";
    case GL_INT: return "int";
    case GL_SAMPLER_2D: return "sampler-2D";
    case GL_SAMPLER_2D_SHADOW: return "sampler-2D-shadow";
    case GL_SAMPLER_3D: return "sampler-3D";
    case GL_SAMPLER_CUBE: return "sampler-cube";
    default: break;
    }
    return "other";
}

[[maybe_unused]] bool
is_valid(GLuint programme)
{
#ifndef NDEBUG
    glValidateProgram(programme);
    int params = -1;
    glGetProgramiv(programme, GL_VALIDATE_STATUS, &params);
    debug("program {} validate status = {}", programme, params);
    if (GL_TRUE != params) {
        print_programme_info_log(programme);
        return false;
    }
#else  // NDEBUG
    (void)programme;
#endif // NDEBUG
    return true;
}

} // namespace

void
print_all(GLuint programme)
{
    debug("shader programme {} info", programme);
    int params = -1;
    glGetProgramiv(programme, GL_LINK_STATUS, &params);
    debug("link status = {}", params);
    glGetProgramiv(programme, GL_ATTACHED_SHADERS, &params);
    debug("attached shaders = {}", params);
    glGetProgramiv(programme, GL_ACTIVE_ATTRIBUTES, &params);
    debug("active attributes = {}", params);

    constexpr auto max_length_   = 64;
    auto           name          = std::array<char, max_length_>{};
    int            actual_length = 0;
    int            size          = 0;
    GLenum         type;

    std::string long_name;
    int         location = 0;

    for (GLuint i = 0; i < static_cast<GLuint>(params); ++i) {
        glGetActiveAttrib(
            programme,
            i,
            max_length_,
            &actual_length,
            &size,
            &type,
            name.data());
        if (size > 1) {
            for (int j = 0; j < size; ++j) {
                long_name = fmt::format(
                    "{}[{}]", reinterpret_cast<const char*>(name.data()), j);
                location = glGetAttribLocation(programme, long_name.c_str());
                debug(
                    "  {}) type: {}, name: {}, location: {}",
                    i,
                    GL_type_to_string(type),
                    long_name.c_str(),
                    location);
            }
        } else {
            location = glGetAttribLocation(programme, name.data());
            debug(
                "  {}) type: {}, name {}, location: {}",
                i,
                GL_type_to_string(type),
                name.data(),
                location);
        }
    }

    glGetProgramiv(programme, GL_ACTIVE_UNIFORMS, &params);
    debug("GL_ACTIVE_UNIFORMS = {}", params);
    for (GLuint i = 0; i < static_cast<GLuint>(params); ++i) {
        glGetActiveUniform(
            programme,
            i,
            max_length_,
            &actual_length,
            &size,
            &type,
            name.data());
        if (size > 1) {
            for (int j = 0; j < size; ++j) {
                long_name = fmt::format(
                    "{}[{}]", reinterpret_cast<const char*>(name.data()), j);
                location = glGetUniformLocation(programme, long_name.c_str());
                debug(
                    "  {}) type: {}, name: {}, location: {}",
                    i,
                    GL_type_to_string(type),
                    long_name.c_str(),
                    location);
            }
        } else {
            location = glGetUniformLocation(programme, name.data());
            debug(
                "  {}) type: {}, name: {}, location: {}",
                i,
                GL_type_to_string(type),
                name.data(),
                location);
        }
    }

    print_programme_info_log(programme);
}

void
print_programme_info_log(GLuint programme)
{
    auto actual_length = 0;
    auto log           = std::array<char, max_length>{};
    glGetProgramInfoLog(programme, max_length, &actual_length, log.data());
    debug("program {}\n{}", programme, log.data());
}

void
print_shader_info_log(GLuint shader)
{
    auto actual_length = 0;
    auto log           = std::array<char, max_length>{};
    glGetShaderInfoLog(shader, max_length, &actual_length, log.data());
    debug("shader {}\n{}", shader, log.data());
}

} // namespace shaders

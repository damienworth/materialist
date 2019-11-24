#include "log_gl_params.hpp"

#include <array>

#include "opengl_all.hpp"
#include "spdlog_all.hpp"

using std::array;

namespace gl_params {

void
log()
{
#ifndef NDEBUG
    const auto params = array{GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
                              GL_MAX_CUBE_MAP_TEXTURE_SIZE,
                              GL_MAX_DRAW_BUFFERS,
                              GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
                              GL_MAX_TEXTURE_IMAGE_UNITS,
                              GL_MAX_TEXTURE_SIZE,
                              GL_MAX_VARYING_FLOATS,
                              GL_MAX_VERTEX_ATTRIBS,
                              GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
                              GL_MAX_VERTEX_UNIFORM_COMPONENTS,
                              GL_MAX_VIEWPORT_DIMS,
                              GL_STEREO};

    const auto names = array{"GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS",
                             "GL_MAX_CUBE_MAP_TEXTURE_SIZE",
                             "GL_MAX_DRAW_BUFFERS",
                             "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
                             "GL_MAX_TEXTURE_IMAGE_UNITS",
                             "GL_MAX_TEXTURE_SIZE",
                             "GL_MAX_VARYING_FLOATS",
                             "GL_MAX_VERTEX_ATTRIBS",
                             "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS",
                             "GL_MAX_VERTEX_UNIFORM_COMPONENTS",
                             "GL_MAX_VIEWPORT_DIMS",
                             "GL_STEREO"};

    std::string GL_context_params = "GL context params: \n";

    // integers - only works if the order is 0-10 integer return types
    for (int i = 0; i < 10; i++) {
        int v = 0;
        glGetIntegerv(params[i], &v);
        GL_context_params += names[i];
        GL_context_params += " ";
        GL_context_params += std::to_string(v);
        GL_context_params += "\n";
    }

    // others
    int v[2];
    v[0] = v[1] = 0;
    glGetIntegerv(params[10], v);
    GL_context_params += names[10];
    GL_context_params += " ";
    GL_context_params += v[0];
    GL_context_params += " ";
    GL_context_params += v[0];
    GL_context_params += "\n";

    unsigned char s = 0;
    glGetBooleanv(params[11], &s);
    GL_context_params += names[11];
    GL_context_params += " ";
    GL_context_params += static_cast<unsigned int>(s);
    spdlog::debug(GL_context_params);
#endif // NDEBUG
}

} // namespace gl_params
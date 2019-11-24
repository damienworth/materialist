#include "log_gl_params.hpp"

#include <array>

#include "fmtlib_all.hpp"
#include "opengl_all.hpp"
#include "spdlog_all.hpp"

using std::array;

namespace gl_params {

#ifndef NDEBUG
void
log()
{
    using spdlog::debug;

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

    size_t max_length = 0;
    std::for_each(
        std::begin(names), std::end(names), [&max_length](const auto& text) {
            max_length = std::max(max_length, std::strlen(text));
        });

    debug("");
    debug("{:>{}}", "GL context params", max_length);
    debug("");

    // integers - only works if the order is 0-10 integer return types
    for (int i = 0; i < 10; i++) {
        int v = 0;
        glGetIntegerv(params[i], &v);
        debug(fmt::format("{:>{}}: {}", names[i], max_length, v));
    }

    // others
    int v[2];
    v[0] = v[1] = 0;
    glGetIntegerv(params[10], v);
    debug(fmt::format("{:>{}}: {} {}", names[10], max_length, v[0], v[1]));

    unsigned char s = 0;
    glGetBooleanv(params[11], &s);
    debug(fmt::format(
        "{:>{}}: {}", names[11], max_length, static_cast<unsigned int>(s)));
    debug("");
}
#else
void
log() noexcept
{
}
#endif // NDEBUG

} // namespace gl_params
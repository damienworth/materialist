#ifndef LOG_GL_PARAMS_HPP
#define LOG_GL_PARAMS_HPP

namespace gl_params {

#ifndef NDEBUG
void log();
#else
void log() noexcept;
#endif // NDEBUG

} // namespace gl_params

#endif // LOG_GL_PARAMS_HPP

#include "error_callback.hpp"

#include "spdlog_all.hpp"

namespace glfw {

void
error_callback(int error, const char* description) noexcept
{
    spdlog::error("glfw: code {}, message: \"{}\"", error, description);
}

} // namespace glfw

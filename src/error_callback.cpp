#include "error_callback.hpp"

#include "spdlog_all.hpp"

void glfw_error_callback(int error, const char* description) {
    spdlog::error("glfw: code {}, message: \"{}\"", error, description);
}

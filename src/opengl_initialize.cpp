#include "opengl_initialize.hpp"

#include "error_callback.hpp"
#include "opengl_all.hpp"
#include "spdlog_all.hpp"

namespace opengl {

bool initialize() noexcept {
    spdlog::debug("starting GLFW3 {}", glfwGetVersionString());
    glfwSetErrorCallback(glfw::error_callback);

    if (!glfwInit()) {
        spdlog::error("could not start GLFW3");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    return true;
}

}

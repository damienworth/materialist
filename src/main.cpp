#include <cstdlib> // EXIT_SUCCESS / EXIT_FAILURE

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include "spdlog_all.hpp"

using namespace glm;
using spdlog::debug;

int
main(int, char**)
{
#ifndef NDEBUG
    spdlog::set_level(spdlog::level::debug);
#endif // NDEBUG

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window =
        glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    debug("{} extensions supported", extensionCount);

    glm::mat4 matrix;
    glm::vec4 vec;
    auto      test = matrix * vec;
    (void)test;

    while (!glfwWindowShouldClose(window)) { glfwPollEvents(); }

    glfwDestroyWindow(window);

    glfwTerminate();

    return EXIT_SUCCESS;
}

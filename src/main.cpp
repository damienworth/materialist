#include <cstdlib> // EXIT_SUCCESS / EXIT_FAILURE
#include <vector>

#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include "spdlog_all.hpp"

using namespace glm;
using spdlog::debug;

namespace {

class hello_triangle_application {
    GLFWwindow* _window = nullptr;
    const int   _WIDTH  = 800;
    const int   _HEIGHT = 600;
    VkInstance  _instance;

public:
    void
    run() noexcept
    {
        init_window();
        init_vulkan();
        main_loop();
        cleanup();
    }

private:
    void
    create_instance() noexcept
    {
        VkApplicationInfo app_info  = {};
        app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName   = "Hello Triangle";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName        = "No Engine";
        app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion         = VK_API_VERSION_1_0;

        VkInstanceCreateInfo create_info = {};
        create_info.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;

        uint32_t     glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        create_info.enabledExtensionCount   = glfwExtensionCount;
        create_info.ppEnabledExtensionNames = glfwExtensions;
        create_info.enabledLayerCount       = 0;

        if (vkCreateInstance(&create_info, nullptr, &_instance) != VK_SUCCESS) {
            spdlog::error("failed to create instance!");
            std::terminate();
        }

        uint32_t extension_count = 0;
        vkEnumerateInstanceExtensionProperties(
            nullptr, &extension_count, nullptr);
        std::vector<VkExtensionProperties> extensions(extension_count);
        vkEnumerateInstanceExtensionProperties(
            nullptr, &extension_count, extensions.data());

#ifndef NDEBUG
        debug("available extensions:");

        for (const auto& extension : extensions) {
            debug("\t{}", extension.extensionName);
        }
#endif // NDEBUG
    }

    void
    init_window() noexcept
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        _window = glfwCreateWindow(_WIDTH, _HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void
    init_vulkan() noexcept
    {
        create_instance();
    }

    void
    main_loop() noexcept
    {
        while (!glfwWindowShouldClose(_window)) { glfwPollEvents(); }
    }

    void
    cleanup() noexcept
    {
        vkDestroyInstance(_instance, nullptr);
        glfwDestroyWindow(_window);
        glfwTerminate();
    }
};

} // namespace

int
main(int, char**)
{
#ifndef NDEBUG
    spdlog::set_level(spdlog::level::debug);
#endif // NDEBUG

    hello_triangle_application app;

    app.run();

    return EXIT_SUCCESS;
}
#include <cstdlib> // EXIT_SUCCESS / EXIT_FAILURE

#include "spdlog_all.hpp"

#include <vulkan/vulkan.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#include "application_hello_triangle.hpp"
#include "error_handling.hpp"

int
main(int, char**)
{
#ifndef NDEBUG
    spdlog::set_level(spdlog::level::debug);
#endif // NDEBUG

    vk::DynamicLoader dl;
    if (!dl.success()) { ERROR("failed to create dynamic loader"); }
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
        dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    application::hello_triangle app;
    app.run();

    return EXIT_SUCCESS;
}

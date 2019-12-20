#ifndef MATERIALIST_ERROR_HANDLING_HPP
#define MATERIALIST_ERROR_HANDLING_HPP

#include <exception>

#ifndef NDEBUG
#include <vulkan/vulkan.hpp>
#endif // NDEBUG

#include "spdlog_all.hpp"

#define ERROR(...)              \
    spdlog::error(__VA_ARGS__); \
    std::terminate();

#ifndef NDEBUG
extern "C" {
VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT*,
    void*);
}
#endif // NDEBUG

#endif // MATERIALIST_ERROR_HANDLING_HPP


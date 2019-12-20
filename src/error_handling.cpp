#include "error_handling.hpp"

extern "C" {

#ifndef NDEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL
                    debug_callback(
                        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                        VkDebugUtilsMessageTypeFlagsEXT,
                        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                        void*)
{
    switch (severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        spdlog::debug(callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        spdlog::info(callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        spdlog::warn(callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        spdlog::error(callback_data->pMessage);
        break;
    default: spdlog::critical(callback_data->pMessage); break;
    }

    return VK_FALSE;
}
#endif // NDEBUG
}


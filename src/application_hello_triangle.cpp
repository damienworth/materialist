#include "application_hello_triangle.hpp"

#include <iterator>
#include <string_view>

#include "spdlog_all.hpp"

using spdlog::critical;
using spdlog::debug;
using spdlog::error;
using spdlog::info;
using spdlog::log;
using spdlog::warn;

namespace application {

namespace {

#ifndef NDEBUG
VkResult create_debug_utils_messenger_EXT(
    VkInstance,
    const VkDebugUtilsMessengerCreateInfoEXT*,
    const VkAllocationCallbacks*,
    VkDebugUtilsMessengerEXT*);

void destroy_debug_utils_messenger_EXT(
    VkInstance&, VkDebugUtilsMessengerEXT&, const VkAllocationCallbacks*);

bool check_validation_layer_support(const std::vector<const char*>&) noexcept;
#endif // NDEBUG

VkInstance create_instance(
#ifndef NDEBUG
    const std::vector<const char*>&
#endif // NDEBUG
    ) noexcept;

GLFWwindow* init_window(std::string_view, int, int) noexcept;

#ifndef NDEBUG
void setup_debug_messenger(VkInstance&, VkDebugUtilsMessengerEXT&) noexcept;
#endif // NDEBUG

VkInstance init_vulkan(
#ifndef NDEBUG
    VkDebugUtilsMessengerEXT&, const std::vector<const char*>&
#endif // NDEBUG
    ) noexcept;

void main_loop(GLFWwindow*) noexcept;

void cleanup(
    VkInstance&&,
    GLFWwindow*
#ifndef NDEBUG
    ,
    VkDebugUtilsMessengerEXT
#endif // NDEBUG
    ) noexcept;

std::vector<const char*> get_required_extensions();

#ifndef NDEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT*,
    void*);
#endif // NDEBUG

} // namespace

void
hello_triangle::run() noexcept
{
    _window   = init_window("Vulkan", _WIDTH, _HEIGHT);
    _instance = std::move(init_vulkan(
#ifndef NDEBUG
        _debug_messenger, _validation_layers
#endif // NDEBUG
        ));
    main_loop(_window);
    cleanup(
        std::move(_instance),
        _window
#ifndef NDEBUG
        ,
        _debug_messenger
#endif // NDEBUG
    );
}

namespace {

#ifndef NDEBUG
VkResult
create_debug_utils_messenger_EXT(
    VkInstance                                instance,
    const VkDebugUtilsMessengerCreateInfoEXT* create_info,
    const VkAllocationCallbacks*              allocator,
    VkDebugUtilsMessengerEXT*                 debug_messenger)
{
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func != nullptr) {
        return func(instance, create_info, allocator, debug_messenger);
    }

    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void
destroy_debug_utils_messenger_EXT(
    VkInstance&                  instance,
    VkDebugUtilsMessengerEXT&    debug_messenger,
    const VkAllocationCallbacks* allocator)
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr) { func(instance, debug_messenger, allocator); }
}

bool
check_validation_layer_support(
    const std::vector<const char*>& validation_layers) noexcept
{
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    debug("available validation layers:");
    for (const auto& layer_properties : available_layers) {
        debug("\t{}", layer_properties.layerName);
    }
    debug("");

    for (const char* layer_name : validation_layers) {
        bool layer_found = false;

        for (const auto& layer_properties : available_layers) {
            if (strcmp(layer_name, layer_properties.layerName) == 0) {
                layer_found = true;
                break;
            }
        }

        if (!layer_found) { return false; }
    }

    return true;
}
#endif // NDEBUG

VkInstance
create_instance(
#ifndef NDEBUG
    const std::vector<const char*>& validation_layers
#endif // NDEBUG
    ) noexcept
{
#ifndef NDEBUG
    if (!check_validation_layer_support(validation_layers)) {
        error("validation layers requested, but not available!");
        std::terminate();
    }
#endif // NDEBUG

    VkInstance instance;

    VkApplicationInfo app_info  = {};
    app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName   = "Hello Triangle";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName        = "No Engine";
    app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion         = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {};
    create_info.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo     = &app_info;

    const auto extensions = get_required_extensions();

    create_info.enabledExtensionCount   = extensions.size();
    create_info.ppEnabledExtensionNames = extensions.data();
#ifndef NDEBUG
    create_info.enabledLayerCount =
        static_cast<uint32_t>(validation_layers.size());
    create_info.ppEnabledLayerNames = validation_layers.data();
#else  // NDEBUG
    create_info.enabledLayerCount = 0;
#endif // NDEBUG

    if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
        error("failed to create instance!");
        std::terminate();
    }

    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> extensions_properties(extension_count);
    vkEnumerateInstanceExtensionProperties(
        nullptr, &extension_count, extensions_properties.data());

#ifndef NDEBUG
    debug("available extensions_properties:");

    for (const auto& extension : extensions_properties) {
        debug("\t{}", extension.extensionName);
    }
    debug("");
#endif // NDEBUG

    return std::move(instance);
}

GLFWwindow*
init_window(std::string_view caption, int width, int height) noexcept
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    return glfwCreateWindow(width, height, caption.data(), nullptr, nullptr);
}

#ifndef NDEBUG
void
setup_debug_messenger(
    VkInstance& instance, VkDebugUtilsMessengerEXT& debug_messenger) noexcept
{
    VkDebugUtilsMessengerCreateInfoEXT create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = debug_callback;

    if (create_debug_utils_messenger_EXT(
            instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS) {
        error("failed to set up debug messenger!");
        std::terminate();
    }
}
#endif // NDEBUG

VkInstance
init_vulkan(
#ifndef NDEBUG
    VkDebugUtilsMessengerEXT&       debug_messenger,
    const std::vector<const char*>& validation_layers
#endif // NDEBUG
    ) noexcept
{
    auto instance = std::move(create_instance(
#ifndef NDEBUG
        validation_layers
#endif // NDEBUG
        ));

#ifndef NDEBUG
    setup_debug_messenger(instance, debug_messenger);
#endif // NDEBUG

    return std::move(instance);
}

void
main_loop(GLFWwindow* window) noexcept
{
    assert(window);
    while (!glfwWindowShouldClose(window)) { glfwPollEvents(); }
}

void
cleanup(
    VkInstance&& instance,
    GLFWwindow*  window
#ifndef NDEBUG
    ,
    VkDebugUtilsMessengerEXT debug_messenger
#endif // NDEBUG
    ) noexcept
{
#ifndef NDEBUG
    destroy_debug_utils_messenger_EXT(instance, debug_messenger, nullptr);
#endif // NDEBUG

    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

std::vector<const char*>
get_required_extensions()
{
    uint32_t     glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions;
    extensions.reserve(
        glfwExtensionCount +
        1 /* for VK_EXT_DEBUG_UTILS_EXTENSION_NAME in debug builds */);
    std::copy(
        glfwExtensions,
        glfwExtensions + glfwExtensionCount,
        std::back_inserter(extensions));

#ifndef NDEBUG
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#else  // NDEBUG
    extensions.shrink_to_fit();
#endif // NDEBUG

    return extensions;
}

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
        debug("validation layer: {}", callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        info("validation layer {}", callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        warn("validation layer {}", callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        error("validation layer {}", callback_data->pMessage);
        break;
    default: critical("validation layer {}", callback_data->pMessage); break;
    }

    return VK_FALSE;
}
#endif // NDEBUG

} // namespace

} // namespace application

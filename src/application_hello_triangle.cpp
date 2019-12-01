#include "application_hello_triangle.hpp"

#include <algorithm>
#include <iterator>
#include <optional>
#include <set>
#include <string_view>
#include <tuple>

#include "spdlog_all.hpp"

using spdlog::critical;
using spdlog::debug;
using spdlog::error;
using spdlog::info;
using spdlog::log;
using spdlog::warn;

namespace application {

namespace /* anonymous */ {

#ifndef NDEBUG
void populate_debug_messenger_create_info(
    VkDebugUtilsMessengerCreateInfoEXT&) noexcept;

VkResult create_debug_utils_messenger_EXT(
    VkInstance,
    const VkDebugUtilsMessengerCreateInfoEXT*,
    const VkAllocationCallbacks*,
    VkDebugUtilsMessengerEXT*) noexcept;

void destroy_debug_utils_messenger_EXT(
    VkInstance,
    VkDebugUtilsMessengerEXT,
    const VkAllocationCallbacks*) noexcept;

bool check_validation_layer_support(const std::vector<const char*>&) noexcept;
#endif // NDEBUG

VkInstance create_instance(
#ifndef NDEBUG
    const std::vector<const char*>&
#endif // NDEBUG
    ) noexcept;

GLFWwindow* init_window(std::string_view, int, int) noexcept;

#ifndef NDEBUG
void setup_debug_messenger(VkInstance, VkDebugUtilsMessengerEXT&) noexcept;
#endif // NDEBUG

bool is_device_suitable(VkPhysicalDevice, VkSurfaceKHR) noexcept;

VkPhysicalDevice pick_physical_device(VkInstance, VkSurfaceKHR) noexcept;

std::tuple<VkDevice, VkQueue, VkQueue> create_logical_device(
    VkPhysicalDevice,
    VkSurfaceKHR
#ifndef NDEBUG
    ,
    const std::vector<const char*>&
#endif // NDEBUG
    ) noexcept;

VkSurfaceKHR create_surface(VkInstance, GLFWwindow*) noexcept;

struct queue_family_indices {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    operator bool() const { return graphics_family && present_family; }
};

queue_family_indices find_queue_families(VkPhysicalDevice, VkSurfaceKHR);

std::tuple<VkInstance, VkDevice, VkQueue, VkQueue, VkSurfaceKHR> init_vulkan(
    GLFWwindow*
#ifndef NDEBUG
    ,
    VkDebugUtilsMessengerEXT&,
    const std::vector<const char*>&
#endif // NDEBUG
    ) noexcept;

void main_loop(GLFWwindow*) noexcept;

void cleanup(
    VkInstance,
    VkDevice,
    VkSurfaceKHR,
    GLFWwindow*
#ifndef NDEBUG
    ,
    VkDebugUtilsMessengerEXT
#endif // NDEBUG
    ) noexcept;

std::vector<const char*> get_required_extensions() noexcept;

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
    _window = init_window("Vulkan", _WIDTH, _HEIGHT);
    std::tie(_instance, _device, _graphics_queue, _present_queue, _surface) =
        std::move(init_vulkan(
            _window
#ifndef NDEBUG
            ,
            _debug_messenger,
            _validation_layers
#endif // NDEBUG
            ));
    main_loop(_window);
    cleanup(
        _instance,
        _device,
        _surface,
        _window
#ifndef NDEBUG
        ,
        _debug_messenger
#endif // NDEBUG
    );
}

namespace /* anonymous */ {

#ifndef NDEBUG
void
populate_debug_messenger_create_info(
    VkDebugUtilsMessengerCreateInfoEXT& create_info) noexcept
{
    create_info       = {};
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
}

VkResult
create_debug_utils_messenger_EXT(
    VkInstance                                instance,
    const VkDebugUtilsMessengerCreateInfoEXT* create_info,
    const VkAllocationCallbacks*              allocator,
    VkDebugUtilsMessengerEXT*                 debug_messenger) noexcept
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
    VkInstance                   instance,
    VkDebugUtilsMessengerEXT     debug_messenger,
    const VkAllocationCallbacks* allocator) noexcept
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func) { func(instance, debug_messenger, allocator); }
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

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
    populate_debug_messenger_create_info(debug_create_info);
    create_info.pNext = reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(
        &debug_create_info);
#else  // NDEBUG
    create_info.enabledLayerCount = 0;
#endif // NDEBUG

    if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
        error("failed to create instance!");
        std::terminate();
    }

    uint32_t extension_props_count = 0;
    vkEnumerateInstanceExtensionProperties(
        nullptr, &extension_props_count, nullptr);
    std::vector<VkExtensionProperties> extensions_props(extension_props_count);
    vkEnumerateInstanceExtensionProperties(
        nullptr, &extension_props_count, extensions_props.data());

#ifndef NDEBUG
    debug("available extensions_properties:");

    for (const auto& extension : extensions_props) {
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
    VkInstance instance, VkDebugUtilsMessengerEXT& debug_messenger) noexcept
{
    VkDebugUtilsMessengerCreateInfoEXT create_info;
    populate_debug_messenger_create_info(create_info);

    if (create_debug_utils_messenger_EXT(
            instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS) {
        error("failed to set up debug messenger!");
        std::terminate();
    }
}
#endif // NDEBUG

bool
is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface) noexcept
{
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures   device_features;
    vkGetPhysicalDeviceProperties(device, &device_properties);
    vkGetPhysicalDeviceFeatures(device, &device_features);

    const auto indices = find_queue_families(device, surface);
    return indices &&
           device_properties.deviceType ==
               VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           device_features.geometryShader;
}

VkPhysicalDevice
pick_physical_device(VkInstance instance, VkSurfaceKHR surface) noexcept
{
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

    if (!device_count) {
        error("failed to find GPUs with Vulkan support");
        std::terminate();
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

    auto suitable_it = std::find_if(
        begin(devices), end(devices), [&surface](const auto& device) {
            return is_device_suitable(device, surface);
        });
    if (suitable_it != end(devices)) { physical_device = *suitable_it; }

    if (physical_device == VK_NULL_HANDLE) {
        error("failed to find suitable GPU");
        std::terminate();
    }

    return physical_device;
}

std::tuple<VkDevice, VkQueue, VkQueue>
create_logical_device(
    VkPhysicalDevice physical_device,
    VkSurfaceKHR     surface
#ifndef NDEBUG
    ,
    const std::vector<const char*>& validation_layers
#endif // NDEBUG
    ) noexcept
{
    queue_family_indices indices =
        find_queue_families(physical_device, surface);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    queue_create_infos.reserve(2);

    std::set<uint32_t> unique_queue_families = {*indices.graphics_family,
                                                *indices.present_family};

    float queue_priority = 1.0f;
    for (uint32_t queue_family : unique_queue_families) {
        VkDeviceQueueCreateInfo queue_create_info = {};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount       = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features = {};

    VkDeviceCreateInfo create_info = {};
    create_info.sType              = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount =
        static_cast<uint32_t>(queue_create_infos.size());
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.pEnabledFeatures  = &device_features;

    create_info.enabledExtensionCount = 0;

#ifndef NDEBUG
    create_info.enabledLayerCount =
        static_cast<uint32_t>(validation_layers.size());
    create_info.ppEnabledLayerNames = validation_layers.data();
#else  // NDEBUG
    create_info.enabledLayerCount = 0;
#endif // NDEBUG

    VkDevice device;
    if (vkCreateDevice(physical_device, &create_info, nullptr, &device) !=
        VK_SUCCESS) {
        error("failed to create logical device!");
        std::terminate();
    }

    VkQueue graphics_queue;
    vkGetDeviceQueue(device, *indices.graphics_family, 0, &graphics_queue);

    VkQueue present_queue;
    vkGetDeviceQueue(device, *indices.present_family, 0, &present_queue);

    return std::tuple{device, graphics_queue, present_queue};
}

VkSurfaceKHR
create_surface(VkInstance instance, GLFWwindow* window) noexcept
{
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) !=
        VK_SUCCESS) {
        error("failed to create window surface");
        std::terminate();
    }

    return surface;
}

queue_family_indices
find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    queue_family_indices indices;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, &queue_family_count, queue_families.data());

    auto queue_family_it = std::find_if(
        begin(queue_families),
        end(queue_families),
        [](const auto& queue_family) {
            return queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT;
        });

    if (queue_family_it != end(queue_families)) {
        indices.graphics_family =
            std::distance(begin(queue_families), queue_family_it);
    }

    VkBool32 present_support = false;
    for (int idx = 0; idx != static_cast<int>(queue_families.size()); ++idx) {
        vkGetPhysicalDeviceSurfaceSupportKHR(
            device, idx, surface, &present_support);
        if (present_support) {
            indices.present_family = idx;
            break;
        }
    };

    return indices;
}

std::tuple<VkInstance, VkDevice, VkQueue, VkQueue, VkSurfaceKHR>
init_vulkan(
    GLFWwindow* window
#ifndef NDEBUG
    ,
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

    VkSurfaceKHR surface         = create_surface(instance, window);
    auto         physical_device = pick_physical_device(instance, surface);
    auto [device, graphics_queue, present_queue] = create_logical_device(
        physical_device,
        surface
#ifndef NDEBUG
        ,
        validation_layers
#endif // NDEBUG
    );

    return std::tuple{std::move(instance),
                      std::move(device),
                      std::move(graphics_queue),
                      std::move(present_queue),
                      std::move(surface)};
}

void
main_loop(GLFWwindow* window) noexcept
{
    assert(window);
    while (!glfwWindowShouldClose(window)) { glfwPollEvents(); }
}

void
cleanup(
    VkInstance   instance,
    VkDevice     device,
    VkSurfaceKHR surface,
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

    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

std::vector<const char*>
get_required_extensions() noexcept
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

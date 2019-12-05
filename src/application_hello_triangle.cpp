#include "application_hello_triangle.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>
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

bool is_device_suitable(
    VkPhysicalDevice, VkSurfaceKHR, const std::vector<const char*>&) noexcept;

bool check_device_extension_support(
    VkPhysicalDevice, const std::vector<const char*>&) noexcept;

VkPhysicalDevice pick_physical_device(
    VkInstance, VkSurfaceKHR, const std::vector<const char*>&) noexcept;

std::tuple<VkDevice, VkQueue, VkQueue> create_logical_device(
    VkPhysicalDevice,
    VkSurfaceKHR,
    const std::vector<const char*>&
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

struct swap_chainsupport_details {
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   present_modes;

    operator bool() const { return !formats.empty() && !present_modes.empty(); }
};

queue_family_indices find_queue_families(VkPhysicalDevice, VkSurfaceKHR);

swap_chainsupport_details
    query_swap_chainsupport(VkPhysicalDevice, VkSurfaceKHR) noexcept;

VkSurfaceFormatKHR
choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>&);

VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>&);

VkExtent2D choose_swap_extent(
    const VkSurfaceCapabilitiesKHR&, int width, int height) noexcept;

std::tuple<VkSwapchainKHR, std::vector<VkImage>> create_swapchain(
    VkDevice,
    VkPhysicalDevice,
    VkSurfaceKHR,
    VkFormat&,
    VkExtent2D&,
    int,
    int) noexcept;

std::vector<VkImageView>
create_image_views(VkDevice, const std::vector<VkImage>&, VkFormat) noexcept;

void create_graphics_pipeline(VkDevice) noexcept;

std::vector<char> read_file(std::string_view filename) noexcept;

VkShaderModule
create_shader_module(VkDevice, const std::vector<char>&) noexcept;

std::tuple<
    VkInstance,
    VkDevice,
    VkQueue,
    VkQueue,
    VkSurfaceKHR,
    VkSwapchainKHR,
    std::vector<VkImage>,
    VkFormat,
    VkExtent2D,
    std::vector<VkImageView>>
init_vulkan(
    GLFWwindow*,
    const std::vector<const char*>&,
    int,
    int
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
    VkSwapchainKHR,
    std::vector<VkImageView>&,
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
    std::tie(
        _instance,
        _device,
        _graphics_queue,
        _present_queue,
        _surface,
        _swapchain,
        _swapchain_images,
        _swapchain_image_format,
        _swapchain_extent,
        _swapchain_image_views) =
        std::move(init_vulkan(
            _window,
            {VK_KHR_SWAPCHAIN_EXTENSION_NAME},
            _WIDTH,
            _HEIGHT
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
        _swapchain,
        _swapchain_image_views,
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

    create_info.enabledExtensionCount =
        static_cast<uint32_t>(extensions.size());
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
is_device_suitable(
    VkPhysicalDevice                physical_device,
    VkSurfaceKHR                    surface,
    const std::vector<const char*>& device_extensions) noexcept
{
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures   device_features;
    vkGetPhysicalDeviceProperties(physical_device, &device_properties);
    vkGetPhysicalDeviceFeatures(physical_device, &device_features);

    bool extensions_supported =
        check_device_extension_support(physical_device, device_extensions);

    const auto indices = find_queue_families(physical_device, surface);

    swap_chainsupport_details swap_chainsupport;
    if (extensions_supported) {
        swap_chainsupport = query_swap_chainsupport(physical_device, surface);
    }

    return indices &&
           device_properties.deviceType ==
               VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           device_features.geometryShader && swap_chainsupport;
}

bool
check_device_extension_support(
    VkPhysicalDevice                physical_device,
    const std::vector<const char*>& device_extensions) noexcept
{
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(
        physical_device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(
        physical_device,
        nullptr,
        &extension_count,
        available_extensions.data());

    std::set<std::string> required_extensions(
        begin(device_extensions), end(device_extensions));
    for (const auto& extension : available_extensions) {
        required_extensions.erase(extension.extensionName);
    }

#ifndef NDEBUG
    for (const auto& extension : required_extensions) {
        error("extension {} is not supported", extension);
    }
#endif // NDEBUG

    return required_extensions.empty();
}

VkPhysicalDevice
pick_physical_device(
    VkInstance                      instance,
    VkSurfaceKHR                    surface,
    const std::vector<const char*>& device_extensions) noexcept
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
        begin(devices),
        end(devices),
        [&surface, &device_extensions](const auto& device) {
            return is_device_suitable(device, surface, device_extensions);
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
    VkPhysicalDevice                physical_device,
    VkSurfaceKHR                    surface,
    const std::vector<const char*>& device_extensions
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

    create_info.enabledExtensionCount =
        static_cast<uint32_t>(device_extensions.size());
    create_info.ppEnabledExtensionNames = device_extensions.data();

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

swap_chainsupport_details
query_swap_chainsupport(
    VkPhysicalDevice physical_device, VkSurfaceKHR surface) noexcept
{
    swap_chainsupport_details details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physical_device, surface, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        physical_device, surface, &format_count, nullptr);

    if (format_count) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            physical_device, surface, &format_count, details.formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physical_device, surface, &present_mode_count, nullptr);
    if (present_mode_count) {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physical_device,
            surface,
            &present_mode_count,
            details.present_modes.data());
    }

    return details;
}

VkSurfaceFormatKHR
choose_swap_surface_format(
    const std::vector<VkSurfaceFormatKHR>& available_formats)
{
    for (const auto& available_format : available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_UNORM &&
            available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }

    return available_formats[0];
}

VkPresentModeKHR
choose_swap_present_mode(
    const std::vector<VkPresentModeKHR>& available_present_modes)
{
    for (const auto& available_present_mode : available_present_modes) {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
choose_swap_extent(
    const VkSurfaceCapabilitiesKHR& capabilities,
    int                             width,
    int                             height) noexcept
{
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }

    VkExtent2D actual_extent = {static_cast<uint32_t>(width),
                                static_cast<uint32_t>(height)};

    std::clamp(
        actual_extent.width,
        capabilities.minImageExtent.width,
        capabilities.maxImageExtent.width);

    std::clamp(
        actual_extent.height,
        capabilities.minImageExtent.height,
        capabilities.maxImageExtent.height);

    return actual_extent;
}

std::tuple<VkSwapchainKHR, std::vector<VkImage>>
create_swapchain(
    VkDevice         device,
    VkPhysicalDevice physical_device,
    VkSurfaceKHR     surface,
    VkFormat&        swapchain_image_format,
    VkExtent2D&      swapchain_extent,
    int              width,
    int              height) noexcept
{
    swap_chainsupport_details swap_chainsupport =
        query_swap_chainsupport(physical_device, surface);

    VkSurfaceFormatKHR surface_format =
        choose_swap_surface_format(swap_chainsupport.formats);
    VkPresentModeKHR present_mode =
        choose_swap_present_mode(swap_chainsupport.present_modes);
    VkExtent2D extent =
        choose_swap_extent(swap_chainsupport.capabilities, width, height);

    uint32_t image_count = swap_chainsupport.capabilities.minImageCount + 1u;
    if (swap_chainsupport.capabilities.maxImageCount > 0 &&
        image_count > swap_chainsupport.capabilities.maxImageCount) {
        image_count = swap_chainsupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;

    create_info.minImageCount    = image_count;
    create_info.imageFormat      = surface_format.format;
    create_info.imageColorSpace  = surface_format.colorSpace;
    create_info.imageExtent      = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    queue_family_indices indices =
        find_queue_families(physical_device, surface);
    auto queue_family_indices =
        std::array{*indices.graphics_family, *indices.present_family};

    if (*indices.graphics_family != *indices.present_family) {
        create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices   = queue_family_indices.data();
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    create_info.preTransform = swap_chainsupport.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode    = present_mode;
    create_info.clipped        = VK_TRUE;
    create_info.oldSwapchain   = VK_NULL_HANDLE;

    VkSwapchainKHR swapchain;
    if (vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain) !=
        VK_SUCCESS) {
        error("failed to create swap chain!");
        std::terminate();
    }

    std::vector<VkImage> swapchain_images;
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr);
    swapchain_images.resize(image_count);
    vkGetSwapchainImagesKHR(
        device, swapchain, &image_count, swapchain_images.data());
    swapchain_image_format = surface_format.format;
    swapchain_extent       = extent;

    return {std::move(swapchain), std::move(swapchain_images)};
}

std::vector<VkImageView>
create_image_views(
    VkDevice                    device,
    const std::vector<VkImage>& swapchain_images,
    VkFormat                    swapchain_image_format) noexcept
{
    std::vector<VkImageView> swapchain_image_views;
    swapchain_image_views.resize(swapchain_images.size());
    for (size_t i = 0; i < swapchain_images.size(); ++i) {
        VkImageViewCreateInfo create_info = {};
        create_info.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image        = swapchain_images[i];
        create_info.viewType     = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format       = swapchain_image_format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel   = 0;
        create_info.subresourceRange.levelCount     = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount     = 1;

        if (vkCreateImageView(
                device, &create_info, nullptr, &swapchain_image_views[i]) !=
            VK_SUCCESS) {
            error("failed to create image views");
            std::terminate();
        }
    }

    return swapchain_image_views;
}

void
create_graphics_pipeline(VkDevice device) noexcept
{
    auto vert_shader_code = read_file("shaders/shader.vert.spv");
    auto frag_shader_code = read_file("shaders/shader.frag.spv");

    VkShaderModule vert_shader_module =
        create_shader_module(device, vert_shader_code);
    VkShaderModule frag_shader_module =
        create_shader_module(device, frag_shader_code);

    VkPipelineShaderStageCreateInfo vert_shader_stage_info = {};
    vert_shader_stage_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = vert_shader_module;
    vert_shader_stage_info.pName  = "main";

    VkPipelineShaderStageCreateInfo frag_shader_stage_info = {};
    frag_shader_stage_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module = frag_shader_module;
    frag_shader_stage_info.pName  = "main";

    vkDestroyShaderModule(device, frag_shader_module, nullptr);
    vkDestroyShaderModule(device, vert_shader_module, nullptr);

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info,
                                                       frag_shader_stage_info};

    (void)shader_stages;
}

std::vector<char>
read_file(std::string_view filename) noexcept
{
    std::ifstream file(filename.data(), std::ios::ate | std::ios::binary);
    if (!file) {
        error("failed to open file {}", filename);
        std::terminate();
    }

    const auto        file_size = file.tellg();
    std::vector<char> buffer(file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();

    return buffer;
}

VkShaderModule
create_shader_module(VkDevice device, const std::vector<char>& code) noexcept
{
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode    = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shader_module;
    if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) !=
        VK_SUCCESS) {
        error("failed to create shader module");
        std::terminate();
    }

    return shader_module;
}

std::tuple<
    VkInstance,
    VkDevice,
    VkQueue,
    VkQueue,
    VkSurfaceKHR,
    VkSwapchainKHR,
    std::vector<VkImage>,
    VkFormat,
    VkExtent2D,
    std::vector<VkImageView>>
init_vulkan(
    GLFWwindow*                     window,
    const std::vector<const char*>& device_extensions,
    int                             width,
    int                             height
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

    VkSurfaceKHR surface = create_surface(instance, window);
    auto         physical_device =
        pick_physical_device(instance, surface, device_extensions);
    auto [device, graphics_queue, present_queue] = create_logical_device(
        physical_device,
        surface,
        device_extensions
#ifndef NDEBUG
        ,
        validation_layers
#endif // NDEBUG
    );

    VkFormat   swapchain_image_format;
    VkExtent2D swapchain_extent;

    auto [swapchain, swapchain_images] = create_swapchain(
        device,
        physical_device,
        surface,
        swapchain_image_format,
        swapchain_extent,
        width,
        height);

    auto swapchain_image_views =
        create_image_views(device, swapchain_images, swapchain_image_format);

    /* auto graphics_pipeline = */ create_graphics_pipeline(device);

    return std::tuple{std::move(instance),
                      std::move(device),
                      std::move(graphics_queue),
                      std::move(present_queue),
                      std::move(surface),
                      std::move(swapchain),
                      std::move(swapchain_images),
                      std::move(swapchain_image_format),
                      std::move(swapchain_extent),
                      std::move(swapchain_image_views)};
}

void
main_loop(GLFWwindow* window) noexcept
{
    assert(window);
    while (!glfwWindowShouldClose(window)) { glfwPollEvents(); }
}

void
cleanup(
    VkInstance                instance,
    VkDevice                  device,
    VkSurfaceKHR              surface,
    VkSwapchainKHR            swapchain,
    std::vector<VkImageView>& swapchain_image_views,
    GLFWwindow*               window
#ifndef NDEBUG
    ,
    VkDebugUtilsMessengerEXT debug_messenger
#endif // NDEBUG
    ) noexcept
{
#ifndef NDEBUG
    destroy_debug_utils_messenger_EXT(instance, debug_messenger, nullptr);
#endif // NDEBUG

    for (auto image_view : swapchain_image_views) {
        vkDestroyImageView(device, image_view, nullptr);
    }

    vkDestroySwapchainKHR(device, swapchain, nullptr);
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
        debug(callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        info(callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        warn(callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        error(callback_data->pMessage);
        break;
    default: critical(callback_data->pMessage); break;
    }

    return VK_FALSE;
}
#endif // NDEBUG

} // namespace
} // namespace application

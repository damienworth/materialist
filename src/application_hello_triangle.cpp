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

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

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

VkRenderPass create_render_pass(VkDevice, VkFormat) noexcept;

std::tuple<VkPipelineLayout, VkPipeline>
    create_graphics_pipeline(VkDevice, VkExtent2D, VkRenderPass) noexcept;

std::vector<VkFramebuffer> create_framebuffers(
    VkDevice,
    VkRenderPass,
    const std::vector<VkImageView>&,
    VkExtent2D) noexcept;

VkCommandPool
    create_command_pool(VkDevice, VkPhysicalDevice, VkSurfaceKHR) noexcept;

std::vector<VkCommandBuffer> create_command_buffers(
    VkDevice,
    const std::vector<VkFramebuffer>&,
    VkRenderPass,
    VkExtent2D,
    VkPipeline,
    VkCommandPool) noexcept;

std::tuple<
    std::vector<VkSemaphore>,
    std::vector<VkSemaphore>,
    std::vector<VkFence>,
    std::vector<VkFence>>
create_sync_objects(VkDevice, const std::vector<VkImage>&) noexcept;

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
    std::vector<VkImageView>,
    VkPipelineLayout,
    VkPipeline,
    VkRenderPass,
    std::vector<VkFramebuffer>,
    VkCommandPool,
    std::vector<VkCommandBuffer>,
    std::vector<VkSemaphore>,
    std::vector<VkSemaphore>,
    std::vector<VkFence>,
    std::vector<VkFence>>
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

void main_loop(
    VkDevice,
    VkQueue,
    VkQueue,
    VkSwapchainKHR,
    const std::vector<VkCommandBuffer>&,
    const std::vector<VkSemaphore>&,
    const std::vector<VkSemaphore>&,
    const std::vector<VkFence>&,
    std::vector<VkFence>&,
    size_t&,
    GLFWwindow*) noexcept;

void draw_frame(
    VkDevice,
    VkQueue,
    VkQueue,
    VkSwapchainKHR,
    const std::vector<VkCommandBuffer>&,
    const std::vector<VkSemaphore>&,
    const std::vector<VkSemaphore>&,
    const std::vector<VkFence>&,
    std::vector<VkFence>&,
    size_t&) noexcept;

void cleanup(
    VkInstance,
    VkDevice,
    VkSurfaceKHR,
    VkSwapchainKHR,
    std::vector<VkImageView>&,
    VkPipelineLayout,
    VkPipeline,
    VkRenderPass,
    std::vector<VkFramebuffer>&,
    VkCommandPool,
    std::vector<VkSemaphore>&,
    std::vector<VkSemaphore>&,
    std::vector<VkFence>&,
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
        _swapchain_image_views,
        _pipeline_layout,
        _graphics_pipeline,
        _render_pass,
        _swapchain_framebuffers,
        _command_pool,
        _command_buffers,
        _image_available_semaphores,
        _render_finished_semaphores,
        _inflight_fences,
        _images_inflight) =
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
    main_loop(
        _device,
        _graphics_queue,
        _present_queue,
        _swapchain,
        _command_buffers,
        _image_available_semaphores,
        _render_finished_semaphores,
        _inflight_fences,
        _images_inflight,
        _current_frame,
        _window);
    cleanup(
        _instance,
        _device,
        _surface,
        _swapchain,
        _swapchain_image_views,
        _pipeline_layout,
        _graphics_pipeline,
        _render_pass,
        _swapchain_framebuffers,
        _command_pool,
        _image_available_semaphores,
        _render_finished_semaphores,
        _inflight_fences,
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

VkRenderPass
create_render_pass(VkDevice device, VkFormat swapchain_image_format) noexcept
{
    VkAttachmentDescription color_attachment = {};
    color_attachment.format                  = swapchain_image_format;
    color_attachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment            = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &color_attachment_ref;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass          = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments    = &color_attachment;
    render_pass_info.subpassCount    = 1;
    render_pass_info.pSubpasses      = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies   = &dependency;

    VkRenderPass render_pass;
    if (vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass) !=
        VK_SUCCESS) {
        error("failed to create render pass");
        std::terminate();
    }

    return render_pass;
}

std::tuple<VkPipelineLayout, VkPipeline>
create_graphics_pipeline(
    VkDevice     device,
    VkExtent2D   swapchain_extent,
    VkRenderPass render_pass) noexcept
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

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount   = 0;
    vertex_input_info.vertexAttributeDescriptionCount = 0;
    (void)vertex_input_info;

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;
    (void)input_assembly;

    VkViewport viewport = {};
    viewport.x          = 0.f;
    viewport.y          = 0.f;
    viewport.width      = static_cast<float>(swapchain_extent.width);
    viewport.height     = static_cast<float>(swapchain_extent.height);
    viewport.minDepth   = 0.f;
    viewport.maxDepth   = 1.f;

    VkRect2D scissor = {};
    scissor.offset   = {0, 0};
    scissor.extent   = swapchain_extent;

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports    = &viewport;
    viewport_state.scissorCount  = 1;
    viewport_state.pScissors     = &scissor;
    (void)viewport_state;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.f;
    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;
    (void)rasterizer;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.f;
    multisampling.pSampleMask           = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable      = VK_FALSE;
    (void)multisampling;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable         = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp        = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp        = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blending = {};
    color_blending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable     = VK_FALSE;
    color_blending.logicOp           = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount   = 1;
    color_blending.pAttachments      = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.f;
    color_blending.blendConstants[1] = 0.f;
    color_blending.blendConstants[2] = 0.f;
    color_blending.blendConstants[3] = 0.f;
    (void)color_blending;

    VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                       VK_DYNAMIC_STATE_LINE_WIDTH};

    VkPipelineDynamicStateCreateInfo dynamic_state = {};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = 2;
    dynamic_state.pDynamicStates    = dynamic_states;
    (void)dynamic_state;

    VkPipelineLayout pipeline_layout;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount         = 0;
    pipeline_layout_info.pSetLayouts            = nullptr;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges    = nullptr;

    if (vkCreatePipelineLayout(
            device, &pipeline_layout_info, nullptr, &pipeline_layout) !=
        VK_SUCCESS) {
        error("failed to create pipeline layout");
        std::terminate();
    }

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info,
                                                       frag_shader_stage_info};

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages    = shader_stages;
    pipeline_info.pVertexInputState   = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState      = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState   = &multisampling;
    pipeline_info.pDepthStencilState  = nullptr;
    pipeline_info.pColorBlendState    = &color_blending;
    pipeline_info.pDynamicState       = nullptr;
    pipeline_info.layout              = pipeline_layout;
    pipeline_info.renderPass          = render_pass;
    pipeline_info.subpass             = 0;
    pipeline_info.basePipelineHandle  = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex   = -1;

    VkPipeline graphics_pipeline;
    if (vkCreateGraphicsPipelines(
            device,
            VK_NULL_HANDLE,
            1,
            &pipeline_info,
            nullptr,
            &graphics_pipeline) != VK_SUCCESS) {
        error("failed to create graphics pipeline");
        std::terminate();
    }

    vkDestroyShaderModule(device, frag_shader_module, nullptr);
    vkDestroyShaderModule(device, vert_shader_module, nullptr);

    return {pipeline_layout, graphics_pipeline};
}

std::vector<VkFramebuffer>
create_framebuffers(
    VkDevice                        device,
    VkRenderPass                    render_pass,
    const std::vector<VkImageView>& swapchain_image_views,
    VkExtent2D                      swapchain_extent) noexcept
{
    std::vector<VkFramebuffer> swapchain_framebuffers;
    swapchain_framebuffers.resize(swapchain_image_views.size());

    for (size_t i = 0; i != swapchain_image_views.size(); ++i) {
        VkImageView attachments[] = {swapchain_image_views[i]};

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType      = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments    = attachments;
        framebuffer_info.width           = swapchain_extent.width;
        framebuffer_info.height          = swapchain_extent.height;
        framebuffer_info.layers          = 1;

        if (vkCreateFramebuffer(
                device,
                &framebuffer_info,
                nullptr,
                &swapchain_framebuffers[i]) != VK_SUCCESS) {
            error("failed to create framebuffer");
            std::terminate();
        }
    }

    return swapchain_framebuffers;
}

VkCommandPool
create_command_pool(
    VkDevice         device,
    VkPhysicalDevice physical_device,
    VkSurfaceKHR     surface) noexcept
{
    auto indices = find_queue_families(physical_device, surface);

    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = *indices.graphics_family;
    pool_info.flags            = 0;

    VkCommandPool command_pool;
    if (vkCreateCommandPool(device, &pool_info, nullptr, &command_pool) !=
        VK_SUCCESS) {
        error("failed to create command pool");
        std::terminate();
    }

    return command_pool;
}

std::vector<VkCommandBuffer>
create_command_buffers(
    VkDevice                          device,
    const std::vector<VkFramebuffer>& swapchain_framebuffers,
    VkRenderPass                      render_pass,
    VkExtent2D                        swapchain_extent,
    VkPipeline                        graphics_pipeline,
    VkCommandPool                     command_pool) noexcept
{
    std::vector<VkCommandBuffer> command_buffers;
    command_buffers.resize(swapchain_framebuffers.size());

    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool;
    alloc_info.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount =
        static_cast<uint32_t>(command_buffers.size());

    if (vkAllocateCommandBuffers(device, &alloc_info, command_buffers.data()) !=
        VK_SUCCESS) {
        error("failed to allocate command buffers");
        std::terminate();
    }

    for (size_t i = 0; i != command_buffers.size(); ++i) {
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = 0;
        begin_info.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(command_buffers[i], &begin_info) !=
            VK_SUCCESS) {
            error("failed to begin recording command buffer");
            std::terminate();
        }

        VkRenderPassBeginInfo render_pass_info = {};
        render_pass_info.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass  = render_pass;
        render_pass_info.framebuffer = swapchain_framebuffers[i];
        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = swapchain_extent;

        VkClearValue clear_color         = {0.f, 0.f, 0.f, 1.f};
        render_pass_info.clearValueCount = 1;
        render_pass_info.pClearValues    = &clear_color;

        vkCmdBeginRenderPass(
            command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(
            command_buffers[i],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            graphics_pipeline);
        vkCmdDraw(command_buffers[i], 3, 1, 0, 0);
        vkCmdEndRenderPass(command_buffers[i]);

        if (vkEndCommandBuffer(command_buffers[i]) != VK_SUCCESS) {
            error("failed to record command buffer");
            std::terminate();
        }
    }

    return command_buffers;
}

std::tuple<
    std::vector<VkSemaphore>,
    std::vector<VkSemaphore>,
    std::vector<VkFence>,
    std::vector<VkFence>>
create_sync_objects(
    VkDevice device, const std::vector<VkImage>& swapchain_images) noexcept
{
    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags             = VK_FENCE_CREATE_SIGNALED_BIT;

    std::vector<VkSemaphore> image_available_semaphores(MAX_FRAMES_IN_FLIGHT);
    std::vector<VkSemaphore> render_finished_semaphores(MAX_FRAMES_IN_FLIGHT);
    std::vector<VkFence>     inflight_fences(MAX_FRAMES_IN_FLIGHT);
    std::vector<VkFence>     images_inflight(
        swapchain_images.size(), VK_NULL_HANDLE);

    for (size_t i = 0; i != MAX_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(
                device,
                &semaphore_info,
                nullptr,
                &image_available_semaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(
                device,
                &semaphore_info,
                nullptr,
                &render_finished_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fence_info, nullptr, &inflight_fences[i]) !=
                VK_SUCCESS) {
            error("failed to create synchronization objects for a frame");
            std::terminate();
        }
    }

    return {std::move(image_available_semaphores),
            std::move(render_finished_semaphores),
            std::move(inflight_fences),
            std::move(images_inflight)};
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
    std::vector<VkImageView>,
    VkPipelineLayout,
    VkPipeline,
    VkRenderPass,
    std::vector<VkFramebuffer>,
    VkCommandPool,
    std::vector<VkCommandBuffer>,
    std::vector<VkSemaphore>,
    std::vector<VkSemaphore>,
    std::vector<VkFence>,
    std::vector<VkFence>>
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

    auto render_pass = create_render_pass(device, swapchain_image_format);

    auto [pipeline_layout, graphics_pipeline] =
        create_graphics_pipeline(device, swapchain_extent, render_pass);

    auto swapchain_framebuffers = create_framebuffers(
        device, render_pass, swapchain_image_views, swapchain_extent);

    auto command_pool = create_command_pool(device, physical_device, surface);

    auto command_buffers = create_command_buffers(
        device,
        swapchain_framebuffers,
        render_pass,
        swapchain_extent,
        graphics_pipeline,
        command_pool);

    auto
        [image_available_semaphores,
         render_finished_semaphores,
         inflight_fences,
         images_inflight] = create_sync_objects(device, swapchain_images);

    return std::tuple{std::move(instance),
                      std::move(device),
                      std::move(graphics_queue),
                      std::move(present_queue),
                      std::move(surface),
                      std::move(swapchain),
                      std::move(swapchain_images),
                      std::move(swapchain_image_format),
                      std::move(swapchain_extent),
                      std::move(swapchain_image_views),
                      std::move(pipeline_layout),
                      std::move(graphics_pipeline),
                      std::move(render_pass),
                      std::move(swapchain_framebuffers),
                      std::move(command_pool),
                      std::move(command_buffers),
                      std::move(image_available_semaphores),
                      std::move(render_finished_semaphores),
                      std::move(inflight_fences),
                      std::move(images_inflight)};
}

void
main_loop(
    VkDevice                            device,
    VkQueue                             graphics_queue,
    VkQueue                             present_queue,
    VkSwapchainKHR                      swapchain,
    const std::vector<VkCommandBuffer>& command_buffers,
    const std::vector<VkSemaphore>&     image_available_semaphores,
    const std::vector<VkSemaphore>&     render_finished_semaphores,
    const std::vector<VkFence>&         inflight_fences,
    std::vector<VkFence>&               images_inflight,
    size_t&                             current_frame,
    GLFWwindow*                         window) noexcept
{
    assert(window);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        draw_frame(
            device,
            graphics_queue,
            present_queue,
            swapchain,
            command_buffers,
            image_available_semaphores,
            render_finished_semaphores,
            inflight_fences,
            images_inflight,
            current_frame);
    }

    vkDeviceWaitIdle(device);
}

void
draw_frame(
    VkDevice                            device,
    VkQueue                             graphics_queue,
    VkQueue                             present_queue,
    VkSwapchainKHR                      swapchain,
    const std::vector<VkCommandBuffer>& command_buffers,
    const std::vector<VkSemaphore>&     image_available_semaphores,
    const std::vector<VkSemaphore>&     render_finished_semaphores,
    const std::vector<VkFence>&         inflight_fences,
    std::vector<VkFence>&               images_inflight,
    size_t&                             current_frame) noexcept
{
    vkWaitForFences(
        device, 1, &inflight_fences[current_frame], VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    vkAcquireNextImageKHR(
        device,
        swapchain,
        UINT64_MAX,
        image_available_semaphores[current_frame],
        VK_NULL_HANDLE,
        &image_index);

    // check if a previous frame is using this image (i.e. there is its fence to
    // wait on)
    if (images_inflight[image_index] != VK_NULL_HANDLE) {
        vkWaitForFences(
            device, 1, &images_inflight[image_index], VK_TRUE, UINT64_MAX);
    }
    // mark the image as now being in use by this frame
    images_inflight[image_index] = inflight_fences[current_frame];

    VkSubmitInfo submit_info = {};
    submit_info.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {image_available_semaphores[current_frame]};
    VkPipelineStageFlags wait_stages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores    = wait_semaphores;
    submit_info.pWaitDstStageMask  = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers    = &command_buffers[image_index];

    VkSemaphore signal_semaphores[] = {
        render_finished_semaphores[current_frame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores    = signal_semaphores;

    vkResetFences(device, 1, &inflight_fences[current_frame]);

    if (vkQueueSubmit(
            graphics_queue, 1, &submit_info, inflight_fences[current_frame]) !=
        VK_SUCCESS) {
        error("failed to submit draw command buffer");
        std::terminate();
    }

    VkPresentInfoKHR present_info   = {};
    present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = signal_semaphores;

    VkSwapchainKHR swapchains[] = {swapchain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains    = swapchains;
    present_info.pImageIndices  = &image_index;
    present_info.pResults       = nullptr;

    vkQueuePresentKHR(present_queue, &present_info);

    vkQueueWaitIdle(present_queue);

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void
cleanup(
    VkInstance                  instance,
    VkDevice                    device,
    VkSurfaceKHR                surface,
    VkSwapchainKHR              swapchain,
    std::vector<VkImageView>&   swapchain_image_views,
    VkPipelineLayout            pipeline_layout,
    VkPipeline                  graphics_pipeline,
    VkRenderPass                render_pass,
    std::vector<VkFramebuffer>& swapchain_framebuffers,
    VkCommandPool               command_pool,
    std::vector<VkSemaphore>&   image_available_semaphores,
    std::vector<VkSemaphore>&   render_finished_semaphores,
    std::vector<VkFence>&       inflight_fences,
    GLFWwindow*                 window
#ifndef NDEBUG
    ,
    VkDebugUtilsMessengerEXT debug_messenger
#endif // NDEBUG
    ) noexcept
{
#ifndef NDEBUG
    destroy_debug_utils_messenger_EXT(instance, debug_messenger, nullptr);
#endif // NDEBUG

    // std::for_each(
    //     begin(images_inflight),
    //     end(images_inflight),
    //     [&device](auto& image_inflight) {
    //         vkDestroyFence(device, image_inflight, nullptr);
    //     });

    for (size_t i = 0; i != MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(device, render_finished_semaphores[i], nullptr);
        vkDestroySemaphore(device, image_available_semaphores[i], nullptr);
        vkDestroyFence(device, inflight_fences[i], nullptr);
    }

    vkDestroyCommandPool(device, command_pool, nullptr);

    for (auto framebuffer : swapchain_framebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    vkDestroyPipeline(device, graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
    vkDestroyRenderPass(device, render_pass, nullptr);

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

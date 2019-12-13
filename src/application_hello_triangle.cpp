#include "application_hello_triangle.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <optional>
#include <set>
#include <string_view>
#include <tuple>

#include <gsl/gsl_util>

#include "spdlog_all.hpp"

using spdlog::critical;
using spdlog::debug;
using spdlog::error;
using spdlog::info;
using spdlog::log;
using spdlog::warn;

#define ERROR(...)              \
    spdlog::error(__VA_ARGS__); \
    std::terminate();

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace application {

namespace /* anonymous */ {

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

#ifndef NDEBUG
vk::DebugUtilsMessengerCreateInfoEXT
populate_debug_messenger_create_info() noexcept;

auto create_debug_utils_messenger_EXT(vk::UniqueInstance&) noexcept;

bool check_validation_layer_support(const std::vector<const char*>&) noexcept;
#endif // NDEBUG

vk::UniqueInstance create_instance(
#ifndef NDEBUG
    const std::vector<const char*>&
#endif // NDEBUG
    ) noexcept;

#ifndef NDEBUG
auto setup_debug_messenger(vk::UniqueInstance&) noexcept;
#endif // NDEBUG

bool is_device_suitable(
    vk::PhysicalDevice,
    vk::UniqueSurfaceKHR&,
    const std::vector<const char*>&) noexcept;

bool check_device_extension_support(
    vk::PhysicalDevice, const std::vector<const char*>&) noexcept;

vk::PhysicalDevice pick_physical_device(
    vk::UniqueInstance&,
    vk::UniqueSurfaceKHR&,
    const std::vector<const char*>&) noexcept;

std::tuple<vk::UniqueDevice, vk::Queue, VkQueue> create_logical_device(
    vk::PhysicalDevice,
    vk::UniqueSurfaceKHR&,
    const std::vector<const char*>&
#ifndef NDEBUG
    ,
    const std::vector<const char*>&
#endif // NDEBUG
    ) noexcept;

vk::UniqueSurfaceKHR create_surface(vk::UniqueInstance&, GLFWwindow*) noexcept;

struct queue_family_indices {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    operator bool() const { return graphics_family && present_family; }
};

struct swapchain_support_details {
    vk::SurfaceCapabilitiesKHR        capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR>   present_modes;

    operator bool() const { return !formats.empty() && !present_modes.empty(); }
};

queue_family_indices
find_queue_families(vk::PhysicalDevice, vk::UniqueSurfaceKHR&);

swapchain_support_details
query_swapchain_support(vk::PhysicalDevice, vk::UniqueSurfaceKHR&) noexcept;

vk::SurfaceFormatKHR
choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR>&);

vk::PresentModeKHR
choose_swap_present_mode(const std::vector<vk::PresentModeKHR>&);

vk::Extent2D choose_swap_extent(
    const vk::SurfaceCapabilitiesKHR&, int width, int height) noexcept;

std::tuple<
    vk::UniqueSwapchainKHR,
    std::vector<vk::Image>,
    vk::Format,
    vk::Extent2D>
create_swapchain(
    vk::UniqueDevice&,
    vk::PhysicalDevice,
    vk::UniqueSurfaceKHR&,
    int,
    int) noexcept;

std::vector<VkImageView> create_image_views(
    vk::UniqueDevice&, const std::vector<vk::Image>&, vk::Format) noexcept;

VkRenderPass create_render_pass(vk::UniqueDevice&, VkFormat) noexcept;

std::tuple<VkPipelineLayout, VkPipeline>
create_graphics_pipeline(vk::UniqueDevice&, VkExtent2D, VkRenderPass) noexcept;

std::vector<VkFramebuffer> create_framebuffers(
    vk::UniqueDevice&,
    VkRenderPass,
    const std::vector<VkImageView>&,
    VkExtent2D) noexcept;

VkCommandPool create_command_pool(
    vk::UniqueDevice&, vk::PhysicalDevice, vk::UniqueSurfaceKHR&) noexcept;

std::vector<VkCommandBuffer> create_command_buffers(
    vk::UniqueDevice&,
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
create_sync_objects(vk::UniqueDevice&, const std::vector<vk::Image>&) noexcept;

std::vector<char> read_file(std::string_view filename) noexcept;

VkShaderModule
create_shader_module(vk::UniqueDevice&, const std::vector<char>&) noexcept;

std::tuple<
#ifndef NDEBUG
    vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic>,
#endif // NDEBUG
    vk::UniqueInstance,
    vk::UniqueDevice,
    vk::Queue,
    vk::Queue,
    vk::UniqueSurfaceKHR,
    vk::UniqueSwapchainKHR,
    std::vector<vk::Image>,
    vk::Format,
    vk::Extent2D,
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
    const std::vector<const char*>&
#endif // NDEBUG
    ) noexcept;

void main_loop(
    vk::UniqueDevice&,
    vk::Queue,
    vk::Queue,
    vk::UniqueSwapchainKHR&,
    const std::vector<VkCommandBuffer>&,
    const std::vector<VkSemaphore>&,
    const std::vector<VkSemaphore>&,
    const std::vector<VkFence>&,
    std::vector<VkFence>&,
    size_t&,
    GLFWwindow*) noexcept;

void draw_frame(
    vk::UniqueDevice&,
    vk::Queue,
    vk::Queue,
    vk::UniqueSwapchainKHR&,
    const std::vector<VkCommandBuffer>&,
    const std::vector<VkSemaphore>&,
    const std::vector<VkSemaphore>&,
    const std::vector<VkFence>&,
    std::vector<VkFence>&,
    size_t&) noexcept;

void cleanup(
    vk::UniqueDevice&,
    std::vector<VkImageView>&,
    VkPipelineLayout,
    VkPipeline,
    VkRenderPass,
    std::vector<VkFramebuffer>&,
    VkCommandPool,
    std::vector<VkSemaphore>&,
    std::vector<VkSemaphore>&,
    std::vector<VkFence>&) noexcept;

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
    if (!_window.create("Materialist", 800, 600)) {
        ERROR("failed to create window");
    }

    if (!*_window) { ERROR("failed to create window"); }
    std::tie(
#ifndef NDEBUG
        _debug_messenger,
#endif // NDEBUG
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
        init_vulkan(
            *_window,
            {VK_KHR_SWAPCHAIN_EXTENSION_NAME},
            _window.w(),
            _window.h()
#ifndef NDEBUG
                ,
            _validation_layers
#endif // NDEBUG
        );
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
        *_window);
    cleanup(
        _device,
        _swapchain_image_views,
        _pipeline_layout,
        _graphics_pipeline,
        _render_pass,
        _swapchain_framebuffers,
        _command_pool,
        _image_available_semaphores,
        _render_finished_semaphores,
        _inflight_fences);
}

namespace /* anonymous */ {

#ifndef NDEBUG
vk::DebugUtilsMessengerCreateInfoEXT
populate_debug_messenger_create_info() noexcept
{
    vk::DebugUtilsMessageSeverityFlagsEXT severity_flags(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose);

    vk::DebugUtilsMessageTypeFlagsEXT message_type_flags(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

    return vk::DebugUtilsMessengerCreateInfoEXT(
        {}, severity_flags, message_type_flags, debug_callback);
}

auto
create_debug_utils_messenger_EXT(vk::UniqueInstance& instance) noexcept
{
    auto dmci = populate_debug_messenger_create_info();
    auto [result, debug_utils_messenger] =
        instance->createDebugUtilsMessengerEXTUnique(dmci);
    if (result != vk::Result::eSuccess) {
        ERROR("failed to create debug-utils messenger");
    }

    return std::move(debug_utils_messenger);
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

vk::UniqueInstance
create_instance(
#ifndef NDEBUG
    const std::vector<const char*>& validation_layers
#endif // NDEBUG
    ) noexcept
{
#ifndef NDEBUG
    if (!check_validation_layer_support(validation_layers)) {
        ERROR("validation layers requested, but not available!");
    }
#endif // NDEBUG

    vk::ApplicationInfo app_info(
        "Materialist", 1, "No-engine", 1, VK_MAKE_VERSION(1, 1, 0));

    const auto extensions = get_required_extensions();

    vk::InstanceCreateInfo create_info(
        {},
        &app_info,
#ifndef NDEBUG
        gsl::narrow<uint32_t>(validation_layers.size()),
        validation_layers.data(),
#else  // Release
        0,
        nullptr,
#endif // NDEBUG
        gsl::narrow<uint32_t>(extensions.size()),
        extensions.data());

#ifndef NDEBUG

    auto dmci = populate_debug_messenger_create_info();
    create_info.setPNext(&dmci);

#endif // NDEBUG

    auto [result, instance] = vk::createInstanceUnique(create_info);
    if (result != vk::Result::eSuccess) { ERROR("failed to create instance!"); }

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

#ifndef NDEBUG
auto
setup_debug_messenger(vk::UniqueInstance& instance) noexcept
{
    return create_debug_utils_messenger_EXT(instance);
}
#endif // NDEBUG

bool
is_device_suitable(
    vk::PhysicalDevice              physical_device,
    vk::UniqueSurfaceKHR&           surface,
    const std::vector<const char*>& device_extensions) noexcept
{
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures   device_features;
    vkGetPhysicalDeviceProperties(physical_device, &device_properties);
    vkGetPhysicalDeviceFeatures(physical_device, &device_features);

    bool extensions_supported =
        check_device_extension_support(physical_device, device_extensions);

    const auto indices = find_queue_families(physical_device, surface);

    swapchain_support_details swapchain_support;
    if (extensions_supported) {
        swapchain_support = query_swapchain_support(physical_device, surface);
    }

    return indices &&
           device_properties.deviceType ==
               VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           device_features.geometryShader && swapchain_support;
}

bool
check_device_extension_support(
    vk::PhysicalDevice              physical_device,
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
        ERROR("extension {} is not supported", extension);
    }
#endif // NDEBUG

    return required_extensions.empty();
}

vk::PhysicalDevice
pick_physical_device(
    vk::UniqueInstance&             instance,
    vk::UniqueSurfaceKHR&           surface,
    const std::vector<const char*>& device_extensions) noexcept
{
    vk::PhysicalDevice physical_device;
    auto [result, devices] = instance->enumeratePhysicalDevices();
    if (result != vk::Result::eSuccess) {
        ERROR("failed to find GPUs with Vulkan support");
    }

    auto right_device = [&surface, &device_extensions](const auto& device) {
        return is_device_suitable(device, surface, device_extensions);
    };

    auto suitable_it = std::find_if(begin(devices), end(devices), right_device);
    if (suitable_it != end(devices)) { physical_device = *suitable_it; }

    if (!physical_device) { ERROR("failed to find suitable GPU"); }

    return physical_device;
}

std::tuple<vk::UniqueDevice, vk::Queue, VkQueue>
create_logical_device(
    vk::PhysicalDevice              physical_device,
    vk::UniqueSurfaceKHR&           surface,
    const std::vector<const char*>& device_extensions
#ifndef NDEBUG
    ,
    const std::vector<const char*>& validation_layers
#endif // NDEBUG
    ) noexcept
{
    auto queue_family_properties = physical_device.getQueueFamilyProperties();
    auto graphics_queue_family_index = gsl::narrow<uint32_t>(std::distance(
        begin(queue_family_properties),
        std::find_if(
            begin(queue_family_properties),
            end(queue_family_properties),
            [](const auto& qfp) {
                return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
            })));

    auto [spmresult, present_modes] =
        physical_device.getSurfacePresentModesKHR(*surface);
    if (spmresult != vk::Result::eSuccess) {
        ERROR("failed to get surface present modes");
    }

    auto present_queue_family_index =
        gsl::narrow<uint32_t>(present_modes.size()) <
                graphics_queue_family_index ?
            graphics_queue_family_index :
            0u;

    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
    queue_create_infos.reserve(2);

    std::set<uint32_t> unique_queue_families = {graphics_queue_family_index,
                                                present_queue_family_index};

    float queue_priority = 1.0f;
    for (uint32_t queue_family : unique_queue_families) {
        vk::DeviceQueueCreateInfo queue_create_info(
            {}, queue_family, 1, &queue_priority);
        queue_create_infos.push_back(queue_create_info);
    }

    vk::PhysicalDeviceFeatures device_features;

    vk::DeviceCreateInfo create_info(
        {},
        gsl::narrow<uint32_t>(queue_create_infos.size()),
        queue_create_infos.data(),
#ifndef NDEBUG
        gsl::narrow<uint32_t>(validation_layers.size()),
        validation_layers.data(),
#else  // Release
        0,
        nullptr,
#endif // NDEBUG
        gsl::narrow<uint32_t>(device_extensions.size()),
        device_extensions.data());

    create_info.setPEnabledFeatures(&device_features);

    auto [result, device] = physical_device.createDeviceUnique(create_info);
    if (result != vk::Result::eSuccess) {
        ERROR("failed to create logical device!");
    }

    auto graphics_queue = device->getQueue(graphics_queue_family_index, 0);
    auto present_queue  = device->getQueue(present_queue_family_index, 0);
    return std::tuple{std::move(device), graphics_queue, present_queue};
}

vk::UniqueSurfaceKHR
create_surface(vk::UniqueInstance& instance, GLFWwindow* window) noexcept
{
    VkSurfaceKHR surface_tmp;
    if (glfwCreateWindowSurface(*instance, window, nullptr, &surface_tmp) !=
        VK_SUCCESS) {
        ERROR("failed to create window surface");
    }

    vk::UniqueSurfaceKHR surface(surface_tmp, *instance);

    return std::move(surface);
}

queue_family_indices
find_queue_families(
    vk::PhysicalDevice physical_device, vk::UniqueSurfaceKHR& surface)
{
    queue_family_indices indices;

    auto queue_families = physical_device.getQueueFamilyProperties();

    auto queue_family_it = std::find_if(
        begin(queue_families),
        end(queue_families),
        [](const auto& queue_family) {
            return queue_family.queueFlags & vk::QueueFlagBits::eGraphics;
        });

    if (queue_family_it != end(queue_families)) {
        indices.graphics_family =
            std::distance(begin(queue_families), queue_family_it);
    }

    vk::Result result;
    vk::Bool32 present_support = false;
    for (int idx = 0; idx != static_cast<int>(queue_families.size()); ++idx) {
        std::tie(result, present_support) =
            physical_device.getSurfaceSupportKHR(idx, *surface);
        if (result != vk::Result::eSuccess) {
            ERROR("failed to get surface support KHR");
        }
        if (present_support) {
            indices.present_family = idx;
            break;
        }
    };

    return indices;
}

swapchain_support_details
query_swapchain_support(
    vk::PhysicalDevice physical_device, vk::UniqueSurfaceKHR& surface) noexcept
{
    swapchain_support_details details;
    auto [gscresult, capabilities] =
        physical_device.getSurfaceCapabilitiesKHR(*surface);
    if (gscresult != vk::Result::eSuccess) {
        ERROR("failed to get surface capabilities");
    }
    auto [gsfresult, formats] = physical_device.getSurfaceFormatsKHR(*surface);
    if (gsfresult != vk::Result::eSuccess) {
        ERROR("failed to get surface formats");
    }
    details.capabilities = std::move(capabilities);
    details.formats      = std::move(formats);

    auto [gspmresult, present_modes] =
        physical_device.getSurfacePresentModesKHR(*surface);
    if (gspmresult != vk::Result::eSuccess) {
        ERROR("failed to get surface present modes");
    }
    details.present_modes = std::move(present_modes);
    return details;
}

vk::SurfaceFormatKHR
choose_swap_surface_format(
    const std::vector<vk::SurfaceFormatKHR>& available_formats)
{
    auto it = std::find_if(
        begin(available_formats), end(available_formats), [](const auto& af) {
            return af.format == vk::Format::eB8G8R8A8Unorm &&
                   af.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
        });

    if (it != end(available_formats)) { return *it; }

    return available_formats.front();
}

vk::PresentModeKHR
choose_swap_present_mode(
    const std::vector<vk::PresentModeKHR>& available_present_modes)
{
    auto it = std::find_if(
        begin(available_present_modes),
        end(available_present_modes),
        [](const auto& apm) { return apm == vk::PresentModeKHR::eMailbox; });

    if (it != end(available_present_modes)) { return *it; }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D
choose_swap_extent(
    const vk::SurfaceCapabilitiesKHR& capabilities,
    int                               width,
    int                               height) noexcept
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

std::tuple<
    vk::UniqueSwapchainKHR,
    std::vector<vk::Image>,
    vk::Format,
    vk::Extent2D>
create_swapchain(
    vk::UniqueDevice&     device,
    vk::PhysicalDevice    physical_device,
    vk::UniqueSurfaceKHR& surface,
    int                   width,
    int                   height) noexcept
{
    swapchain_support_details swapchain_support =
        query_swapchain_support(physical_device, surface);

    vk::SurfaceFormatKHR surface_format =
        choose_swap_surface_format(swapchain_support.formats);
    vk::PresentModeKHR present_mode =
        choose_swap_present_mode(swapchain_support.present_modes);
    vk::Extent2D extent =
        choose_swap_extent(swapchain_support.capabilities, width, height);

    uint32_t image_count = swapchain_support.capabilities.minImageCount + 1u;
    if (swapchain_support.capabilities.maxImageCount > 0 &&
        image_count > swapchain_support.capabilities.maxImageCount) {
        image_count = swapchain_support.capabilities.maxImageCount;
    }

    auto      sharing_mode             = vk::SharingMode::eExclusive;
    auto      queue_family_index_count = 0;
    uint32_t* queue_family_indices_ptr = nullptr;
    auto      indices = find_queue_families(physical_device, surface);
    auto      q_family_indices =
        std::array{*indices.graphics_family, *indices.present_family};

    if (*indices.graphics_family != *indices.present_family) {
        sharing_mode             = vk::SharingMode::eConcurrent;
        queue_family_index_count = 2;
        queue_family_indices_ptr = q_family_indices.data();
    } else {
        sharing_mode = vk::SharingMode::eExclusive;
    }

    vk::SwapchainCreateInfoKHR create_info(
        {},
        *surface,
        image_count,
        surface_format.format,
        surface_format.colorSpace,
        extent,
        1,
        vk::ImageUsageFlagBits::eColorAttachment,
        sharing_mode,
        queue_family_index_count,
        queue_family_indices_ptr,
        swapchain_support.capabilities.currentTransform,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        present_mode,
        true,
        nullptr);

    auto [csresult, swapchain] = device->createSwapchainKHRUnique(create_info);
    if (csresult != vk::Result::eSuccess) {
        ERROR("failed to create swapchain");
    }
    auto [gsiresult, swapchain_images] =
        device->getSwapchainImagesKHR(*swapchain);
    if (gsiresult != vk::Result::eSuccess) {
        ERROR("failed to get swapchain images");
    }

    return {std::move(swapchain),
            std::move(swapchain_images),
            surface_format.format,
            extent};
}

std::vector<VkImageView>
create_image_views(
    vk::UniqueDevice&             device,
    const std::vector<vk::Image>& swapchain_images,
    vk::Format                    swapchain_image_format) noexcept
{
    std::vector<VkImageView> swapchain_image_views;
    swapchain_image_views.resize(swapchain_images.size());
    for (size_t i = 0; i < swapchain_images.size(); ++i) {
        VkImageViewCreateInfo create_info = {};
        create_info.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image    = swapchain_images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format   = static_cast<VkFormat>(swapchain_image_format);
        create_info.components.r                = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g                = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b                = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a                = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel   = 0;
        create_info.subresourceRange.levelCount     = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount     = 1;

        if (vkCreateImageView(
                *device, &create_info, nullptr, &swapchain_image_views[i]) !=
            VK_SUCCESS) {
            ERROR("failed to create image views");
        }
    }

    return swapchain_image_views;
}

VkRenderPass
create_render_pass(
    vk::UniqueDevice& device, VkFormat swapchain_image_format) noexcept
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
    if (vkCreateRenderPass(*device, &render_pass_info, nullptr, &render_pass) !=
        VK_SUCCESS) {
        ERROR("failed to create render pass");
    }

    return render_pass;
}

std::tuple<VkPipelineLayout, VkPipeline>
create_graphics_pipeline(
    vk::UniqueDevice& device,
    VkExtent2D        swapchain_extent,
    VkRenderPass      render_pass) noexcept
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
            *device, &pipeline_layout_info, nullptr, &pipeline_layout) !=
        VK_SUCCESS) {
        ERROR("failed to create pipeline layout");
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
            *device,
            VK_NULL_HANDLE,
            1,
            &pipeline_info,
            nullptr,
            &graphics_pipeline) != VK_SUCCESS) {
        ERROR("failed to create graphics pipeline");
    }

    vkDestroyShaderModule(*device, frag_shader_module, nullptr);
    vkDestroyShaderModule(*device, vert_shader_module, nullptr);

    return {pipeline_layout, graphics_pipeline};
}

std::vector<VkFramebuffer>
create_framebuffers(
    vk::UniqueDevice&               device,
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
                *device,
                &framebuffer_info,
                nullptr,
                &swapchain_framebuffers[i]) != VK_SUCCESS) {
            ERROR("failed to create framebuffer");
        }
    }

    return swapchain_framebuffers;
}

VkCommandPool
create_command_pool(
    vk::UniqueDevice&     device,
    vk::PhysicalDevice    physical_device,
    vk::UniqueSurfaceKHR& surface) noexcept
{
    auto indices = find_queue_families(physical_device, surface);

    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = *indices.graphics_family;
    pool_info.flags            = 0;

    VkCommandPool command_pool;
    if (vkCreateCommandPool(*device, &pool_info, nullptr, &command_pool) !=
        VK_SUCCESS) {
        ERROR("failed to create command pool");
    }

    return command_pool;
}

std::vector<VkCommandBuffer>
create_command_buffers(
    vk::UniqueDevice&                 device,
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

    if (vkAllocateCommandBuffers(
            *device, &alloc_info, command_buffers.data()) != VK_SUCCESS) {
        ERROR("failed to allocate command buffers");
    }

    for (size_t i = 0; i != command_buffers.size(); ++i) {
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = 0;
        begin_info.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(command_buffers[i], &begin_info) !=
            VK_SUCCESS) {
            ERROR("failed to begin recording command buffer");
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
            ERROR("failed to record command buffer");
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
    vk::UniqueDevice&             device,
    const std::vector<vk::Image>& swapchain_images) noexcept
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
                *device,
                &semaphore_info,
                nullptr,
                &image_available_semaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(
                *device,
                &semaphore_info,
                nullptr,
                &render_finished_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(*device, &fence_info, nullptr, &inflight_fences[i]) !=
                VK_SUCCESS) {
            ERROR("failed to create synchronization objects for a frame");
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
    if (!file) { ERROR("failed to open file {}", filename); }

    const auto        file_size = file.tellg();
    std::vector<char> buffer(file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();

    return buffer;
}

VkShaderModule
create_shader_module(
    vk::UniqueDevice& device, const std::vector<char>& code) noexcept
{
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode    = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shader_module;
    if (vkCreateShaderModule(*device, &create_info, nullptr, &shader_module) !=
        VK_SUCCESS) {
        ERROR("failed to create shader module");
    }

    return shader_module;
}

std::tuple<
#ifndef NDEBUG
    vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic>,
#endif // NDEBUG
    vk::UniqueInstance,
    vk::UniqueDevice,
    vk::Queue,
    vk::Queue,
    vk::UniqueSurfaceKHR,
    vk::UniqueSwapchainKHR,
    std::vector<vk::Image>,
    vk::Format,
    vk::Extent2D,
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
    const std::vector<const char*>& validation_layers
#endif // NDEBUG
    ) noexcept
{
    vk::DynamicLoader dl;
    auto              vkInstanceProcAddr =
        dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkInstanceProcAddr);
    auto instance = create_instance(
#ifndef NDEBUG
        validation_layers
#endif // NDEBUG
    );

    VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);
#ifndef NDEBUG
    auto debug_messenger = setup_debug_messenger(instance);
#endif // NDEBUG

    vk::UniqueSurfaceKHR surface = create_surface(instance, window);
    auto                 physical_device =
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

    auto
        [swapchain,
         swapchain_images,
         swapchain_image_format,
         swapchain_extent] =
            create_swapchain(device, physical_device, surface, width, height);

    auto swapchain_image_views =
        create_image_views(device, swapchain_images, swapchain_image_format);

    auto render_pass = create_render_pass(
        device, static_cast<VkFormat>(swapchain_image_format));

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

    return std::tuple{
#ifndef NDEBUG
        std::move(debug_messenger),
#endif // NDEBUG
        std::move(instance),
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
    vk::UniqueDevice&                   device,
    vk::Queue                           graphics_queue,
    vk::Queue                           present_queue,
    vk::UniqueSwapchainKHR&             swapchain,
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

    vkDeviceWaitIdle(*device);
}

void
draw_frame(
    vk::UniqueDevice&                   device,
    vk::Queue                           graphics_queue,
    vk::Queue                           present_queue,
    vk::UniqueSwapchainKHR&             swapchain,
    const std::vector<VkCommandBuffer>& command_buffers,
    const std::vector<VkSemaphore>&     image_available_semaphores,
    const std::vector<VkSemaphore>&     render_finished_semaphores,
    const std::vector<VkFence>&         inflight_fences,
    std::vector<VkFence>&               images_inflight,
    size_t&                             current_frame) noexcept
{
    vkWaitForFences(
        *device, 1, &inflight_fences[current_frame], VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    vkAcquireNextImageKHR(
        *device,
        *swapchain,
        UINT64_MAX,
        image_available_semaphores[current_frame],
        VK_NULL_HANDLE,
        &image_index);

    // check if a previous frame is using this image (i.e. there is its fence to
    // wait on)
    if (images_inflight[image_index] != VK_NULL_HANDLE) {
        vkWaitForFences(
            *device, 1, &images_inflight[image_index], VK_TRUE, UINT64_MAX);
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

    vkResetFences(*device, 1, &inflight_fences[current_frame]);

    if (vkQueueSubmit(
            graphics_queue, 1, &submit_info, inflight_fences[current_frame]) !=
        VK_SUCCESS) {
        ERROR("failed to submit draw command buffer");
    }

    VkPresentInfoKHR present_info   = {};
    present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = signal_semaphores;

    VkSwapchainKHR swapchains[] = {*swapchain};
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
    vk::UniqueDevice&           device,
    std::vector<VkImageView>&   swapchain_image_views,
    VkPipelineLayout            pipeline_layout,
    VkPipeline                  graphics_pipeline,
    VkRenderPass                render_pass,
    std::vector<VkFramebuffer>& swapchain_framebuffers,
    VkCommandPool               command_pool,
    std::vector<VkSemaphore>&   image_available_semaphores,
    std::vector<VkSemaphore>&   render_finished_semaphores,
    std::vector<VkFence>&       inflight_fences) noexcept
{
    for (size_t i = 0; i != MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(*device, render_finished_semaphores[i], nullptr);
        vkDestroySemaphore(*device, image_available_semaphores[i], nullptr);
        vkDestroyFence(*device, inflight_fences[i], nullptr);
    }

    vkDestroyCommandPool(*device, command_pool, nullptr);

    for (auto framebuffer : swapchain_framebuffers) {
        vkDestroyFramebuffer(*device, framebuffer, nullptr);
    }

    vkDestroyPipeline(*device, graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(*device, pipeline_layout, nullptr);
    vkDestroyRenderPass(*device, render_pass, nullptr);

    for (auto image_view : swapchain_image_views) {
        vkDestroyImageView(*device, image_view, nullptr);
    }
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

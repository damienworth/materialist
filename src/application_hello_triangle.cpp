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

#include "error_handling.hpp"

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

std::tuple<vk::UniqueDevice, vk::Queue, vk::Queue> create_logical_device(
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

std::vector<vk::UniqueImageView> create_image_views(
    vk::UniqueDevice&,
    const std::vector<vk::Image>&,
    const vk::Format&) noexcept;

vk::UniqueRenderPass
create_render_pass(vk::UniqueDevice&, const vk::Format&) noexcept;

std::tuple<vk::UniquePipelineLayout, vk::UniquePipeline>
create_graphics_pipeline(
    vk::UniqueDevice&,
    const vk::Extent2D&,
    const vk::UniqueRenderPass&) noexcept;

std::vector<vk::UniqueFramebuffer> create_framebuffers(
    vk::UniqueDevice&,
    const vk::UniqueRenderPass&,
    const std::vector<vk::UniqueImageView>&,
    const vk::Extent2D&) noexcept;

vk::UniqueCommandPool create_command_pool(
    vk::UniqueDevice&, vk::PhysicalDevice, vk::UniqueSurfaceKHR&) noexcept;

std::vector<vk::UniqueCommandBuffer> create_command_buffers(
    vk::UniqueDevice&,
    const std::vector<vk::UniqueFramebuffer>&,
    vk::UniqueRenderPass&,
    vk::Extent2D,
    vk::UniquePipeline&,
    vk::UniqueCommandPool&) noexcept;

std::tuple<
    std::vector<vk::UniqueSemaphore>,
    std::vector<vk::UniqueSemaphore>,
    std::vector<vk::UniqueFence>,
    std::vector<vk::Fence>>
create_sync_objects(vk::UniqueDevice&, const std::vector<vk::Image>&) noexcept;

std::vector<char> read_file(std::string_view filename) noexcept;

vk::UniqueShaderModule
create_shader_module(vk::UniqueDevice&, const std::vector<char>&) noexcept;

std::tuple<
#ifndef NDEBUG
    vk::UniqueDebugUtilsMessengerEXT,
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
    std::vector<vk::UniqueImageView>,
    vk::UniquePipelineLayout,
    vk::UniquePipeline,
    vk::UniqueRenderPass,
    std::vector<vk::UniqueFramebuffer>,
    vk::UniqueCommandPool,
    std::vector<vk::UniqueCommandBuffer>,
    std::vector<vk::UniqueSemaphore>,
    std::vector<vk::UniqueSemaphore>,
    std::vector<vk::UniqueFence>,
    std::vector<vk::Fence>>
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
    const std::vector<vk::UniqueCommandBuffer>&,
    const std::vector<vk::UniqueSemaphore>&,
    const std::vector<vk::UniqueSemaphore>&,
    const std::vector<vk::UniqueFence>&,
    std::vector<vk::Fence>&,
    size_t&,
    GLFWwindow*) noexcept;

void draw_frame(
    vk::UniqueDevice&,
    vk::Queue,
    vk::Queue,
    vk::UniqueSwapchainKHR&,
    const std::vector<vk::UniqueCommandBuffer>&,
    const std::vector<vk::UniqueSemaphore>&,
    const std::vector<vk::UniqueSemaphore>&,
    const std::vector<vk::UniqueFence>&,
    std::vector<vk::Fence>&,
    size_t&) noexcept;

std::vector<const char*> get_required_extensions() noexcept;

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
        _images,
        _image_format,
        _extent,
        _image_views,
        _pipeline_layout,
        _graphics_pipeline,
        _render_pass,
        _framebuffers,
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
    auto [result, available_layers] = vk::enumerateInstanceLayerProperties();
    if (result != vk::Result::eSuccess) {
        ERROR("failed to get instance layer properties count");
    }

    spdlog::debug("available validation layers:");
    for (const auto& layer_properties : available_layers) {
        spdlog::debug("\t{}", layer_properties.layerName);
    }
    spdlog::debug("");

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

    auto [eiepresult, extension_props] =
        vk::enumerateInstanceExtensionProperties();
    if (eiepresult != vk::Result::eSuccess) {
        ERROR("failed to enumerate instance extension properties");
    }

#ifndef NDEBUG
    spdlog::debug("available extensions_properties:");

    for (const auto& extension : extension_props) {
        spdlog::debug("\t{}", extension.extensionName);
    }
    spdlog::debug("");
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
    auto device_features   = physical_device.getFeatures();
    auto device_properties = physical_device.getProperties();

    bool extensions_supported =
        check_device_extension_support(physical_device, device_extensions);

    const auto indices = find_queue_families(physical_device, surface);

    swapchain_support_details swapchain_support;
    if (extensions_supported) {
        swapchain_support = query_swapchain_support(physical_device, surface);
    }

    return indices &&
           device_properties.deviceType ==
               vk::PhysicalDeviceType::eDiscreteGpu &&
           device_features.geometryShader && swapchain_support;
}

bool
check_device_extension_support(
    vk::PhysicalDevice              physical_device,
    const std::vector<const char*>& device_extensions) noexcept
{
    auto [result, available_extensions] =
        physical_device.enumerateDeviceExtensionProperties();
    if (result != vk::Result::eSuccess) {
        ERROR("failed to enumerate device extension properties");
    }

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

std::tuple<vk::UniqueDevice, vk::Queue, vk::Queue>
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

    vk::Extent2D actual_extent = {static_cast<uint32_t>(width),
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
    auto [gsiresult, images] = device->getSwapchainImagesKHR(*swapchain);
    if (gsiresult != vk::Result::eSuccess) {
        ERROR("failed to get swapchain images");
    }

    return {
        std::move(swapchain), std::move(images), surface_format.format, extent};
}

std::vector<vk::UniqueImageView>
create_image_views(
    vk::UniqueDevice&             device,
    const std::vector<vk::Image>& images,
    const vk::Format&             image_format) noexcept
{
    std::vector<vk::UniqueImageView> image_views;
    image_views.reserve(images.size());
    vk::ComponentMapping component_mapping(
        vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity);
    vk::ImageSubresourceRange subresource_range(
        vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
    for (auto image : images) {
        vk::ImageViewCreateInfo ivci(
            {},
            image,
            vk::ImageViewType::e2D,
            image_format,
            component_mapping,
            subresource_range);

        auto [result, view] = device->createImageViewUnique(ivci);
        if (result != vk::Result::eSuccess) {
            ERROR("failed to create image view");
        }
        image_views.push_back(std::move(view));
    }

    return image_views;
}

vk::UniqueRenderPass
create_render_pass(
    vk::UniqueDevice& device, const vk::Format& image_format) noexcept
{
    vk::AttachmentDescription attachment_desc(
        {},
        image_format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference color_ref(
        0, vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpass(
        {}, vk::PipelineBindPoint::eGraphics, 0, nullptr, 1, &color_ref);

    vk::SubpassDependency dependency(
        uint32_t(VK_SUBPASS_EXTERNAL),
        uint32_t(0u),
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::AccessFlagBits(),
        vk::AccessFlagBits::eColorAttachmentRead |
            vk::AccessFlagBits::eColorAttachmentWrite);

    vk::RenderPassCreateInfo rpci(
        {}, 1, &attachment_desc, 1, &subpass, 1, &dependency);

    auto [result, render_pass] = device->createRenderPassUnique(rpci);
    if (result != vk::Result::eSuccess) {
        ERROR("failed to create render pass");
    }

    return std::move(render_pass);
}

std::tuple<vk::UniquePipelineLayout, vk::UniquePipeline>
create_graphics_pipeline(
    vk::UniqueDevice&           device,
    const vk::Extent2D&         extent,
    const vk::UniqueRenderPass& render_pass) noexcept
{
    auto vert_shader_code = read_file("shaders/shader.vert.spv");
    auto frag_shader_code = read_file("shaders/shader.frag.spv");

    vk::UniqueShaderModule vert_shader_module =
        create_shader_module(device, vert_shader_code);
    vk::UniqueShaderModule frag_shader_module =
        create_shader_module(device, frag_shader_code);

    vk::PipelineShaderStageCreateInfo vert_pssci(
        {}, vk::ShaderStageFlagBits::eVertex, *vert_shader_module, "main");

    vk::PipelineShaderStageCreateInfo frag_pssci(
        {}, vk::ShaderStageFlagBits::eFragment, *frag_shader_module, "main");

    vk::PipelineVertexInputStateCreateInfo   pvisci;
    vk::PipelineInputAssemblyStateCreateInfo piasci(
        {}, vk::PrimitiveTopology::eTriangleList);

    vk::Viewport viewport(
        0.f,
        0.f,
        gsl::narrow<float>(extent.width),
        gsl::narrow<float>(extent.height),
        0.f,
        1.f);

    vk::Rect2D scissor(vk::Offset2D(0, 0), extent);

    vk::PipelineViewportStateCreateInfo pvsci({}, 1, &viewport, 1, &scissor);

    vk::PipelineRasterizationStateCreateInfo rasterizer(
        {},
        false,
        false,
        vk::PolygonMode::eFill,
        vk::CullModeFlagBits::eBack,
        vk::FrontFace::eClockwise,
        false,
        0.f,
        0.f,
        0.f,
        1.f);

    vk::PipelineMultisampleStateCreateInfo multisampling(
        {},
        vk::SampleCountFlagBits::e1,
        VK_FALSE,
        1.f,
        nullptr,
        VK_FALSE,
        VK_FALSE);

    vk::PipelineColorBlendAttachmentState color_blend_attachment(
        VK_FALSE,
        vk::BlendFactor::eOne,
        vk::BlendFactor::eZero,
        vk::BlendOp::eAdd,
        vk::BlendFactor::eOne,
        vk::BlendFactor::eZero,
        vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

    constexpr auto blend_constants = std::array{0.f, 0.f, 0.f, 0.f};
    vk::PipelineColorBlendStateCreateInfo color_blending(
        {},
        VK_FALSE,
        vk::LogicOp::eCopy,
        1,
        &color_blend_attachment,
        blend_constants);

    vk::DynamicState dynamic_states[] = {vk::DynamicState::eViewport,
                                         vk::DynamicState::eLineWidth};

    vk::PipelineDynamicStateCreateInfo dynamic_state = {};
    dynamic_state.dynamicStateCount                  = 2;
    dynamic_state.pDynamicStates                     = dynamic_states;
    (void)dynamic_state;

    auto [cplresult, pipeline_layout] =
        device->createPipelineLayoutUnique(vk::PipelineLayoutCreateInfo());
    if (cplresult != vk::Result::eSuccess) {
        ERROR("failed to create pipeline layout");
    }

    vk::PipelineShaderStageCreateInfo shader_stages[2] = {vert_pssci,
                                                          frag_pssci};

    vk::GraphicsPipelineCreateInfo gpci(
        {},
        2,
        shader_stages,
        &pvisci,
        &piasci,
        nullptr,
        &pvsci,
        &rasterizer,
        &multisampling,
        nullptr,
        &color_blending,
        nullptr,
        *pipeline_layout,
        *render_pass);

    auto [cgpresult, graphics_pipeline] =
        device->createGraphicsPipelinesUnique(nullptr, gpci);
    if (cgpresult != vk::Result::eSuccess) {
        ERROR("failed to create graphics pipeline");
    }

    return std::tuple{std::move(pipeline_layout),
                      std::move(graphics_pipeline.front())};
}

std::vector<vk::UniqueFramebuffer>
create_framebuffers(
    vk::UniqueDevice&                       device,
    const vk::UniqueRenderPass&             render_pass,
    const std::vector<vk::UniqueImageView>& image_views,
    const vk::Extent2D&                     extent) noexcept
{
    std::vector<vk::UniqueFramebuffer> framebuffers;
    framebuffers.reserve(image_views.size());

    for (auto const& view : image_views) {
        auto                      attachments = std::array{*view};
        vk::FramebufferCreateInfo fci(
            {},
            *render_pass,
            1,
            &attachments[0],
            extent.width,
            extent.height,
            1);

        auto [result, framebuffer] = device->createFramebufferUnique(fci);
        if (result != vk::Result::eSuccess) {
            ERROR("failed to create framebuffer");
        }
        framebuffers.push_back(std::move(framebuffer));
    }

    return std::move(framebuffers);
}

vk::UniqueCommandPool
create_command_pool(
    vk::UniqueDevice&     device,
    vk::PhysicalDevice    physical_device,
    vk::UniqueSurfaceKHR& surface) noexcept
{
    auto indices = find_queue_families(physical_device, surface);

    vk::CommandPoolCreateInfo cpci({}, *indices.graphics_family);

    auto [result, command_pool] = device->createCommandPoolUnique(cpci);
    if (result != vk::Result::eSuccess) {
        ERROR("failed to create command pool");
    }

    return std::move(command_pool);
}

std::vector<vk::UniqueCommandBuffer>
create_command_buffers(
    vk::UniqueDevice&                         device,
    const std::vector<vk::UniqueFramebuffer>& framebuffers,
    vk::UniqueRenderPass&                     render_pass,
    vk::Extent2D                              extent,
    vk::UniquePipeline&                       graphics_pipeline,
    vk::UniqueCommandPool&                    command_pool) noexcept
{
    vk::CommandBufferAllocateInfo cbai(
        *command_pool,
        vk::CommandBufferLevel::ePrimary,
        gsl::narrow<uint32_t>(framebuffers.size()));

    auto [result, command_buffers] = device->allocateCommandBuffersUnique(cbai);
    if (result != vk::Result::eSuccess) {
        ERROR("failed to allocate command buffers");
    }

    vk::ClearValue clear_color(std::array<float, 4>{0.f, 0.f, 0.f, 1.f});

    for (size_t i = 0; i != command_buffers.size(); ++i) {
        result = command_buffers[i]->begin(vk::CommandBufferBeginInfo());
        if (result != vk::Result::eSuccess) {
            ERROR("failed to begin recording command buffer");
        }

        vk::RenderPassBeginInfo render_pass_info(
            *render_pass,
            *framebuffers[i],
            vk::Rect2D(vk::Offset2D(0.f, 0.f), extent),
            1,
            &clear_color);

        command_buffers[i]->beginRenderPass(
            render_pass_info, vk::SubpassContents::eInline);

        command_buffers[i]->bindPipeline(
            vk::PipelineBindPoint::eGraphics, *graphics_pipeline);

        command_buffers[i]->draw(3, 1, 0, 0);
        command_buffers[i]->endRenderPass();

        if (command_buffers[i]->end() != vk::Result::eSuccess) {
            ERROR("failed to record command buffer");
        }
    }

    return std::move(command_buffers);
}

std::tuple<
    std::vector<vk::UniqueSemaphore>,
    std::vector<vk::UniqueSemaphore>,
    std::vector<vk::UniqueFence>,
    std::vector<vk::Fence>>
create_sync_objects(
    vk::UniqueDevice& device, const std::vector<vk::Image>& images) noexcept
{
    vk::SemaphoreCreateInfo sci;

    vk::FenceCreateInfo fci(vk::FenceCreateFlagBits::eSignaled);

    std::vector<vk::UniqueSemaphore> image_available_semaphores;
    image_available_semaphores.reserve(MAX_FRAMES_IN_FLIGHT);
    std::vector<vk::UniqueSemaphore> render_finished_semaphores;
    render_finished_semaphores.reserve(MAX_FRAMES_IN_FLIGHT);
    std::vector<vk::UniqueFence> inflight_fences;
    inflight_fences.reserve(MAX_FRAMES_IN_FLIGHT);
    std::vector<vk::Fence> images_inflight(images.size());

    for (size_t i = 0; i != MAX_FRAMES_IN_FLIGHT; ++i) {
        auto [iaresult, ia_semaphore] = device->createSemaphoreUnique(sci);
        if (iaresult != vk::Result::eSuccess) {
            ERROR("failed to create semaphore");
        }

        auto [rfresult, rf_semaphore] = device->createSemaphoreUnique(sci);
        if (rfresult != vk::Result::eSuccess) {
            ERROR("failed to create semaphore");
        }

        image_available_semaphores.push_back(std::move(ia_semaphore));
        render_finished_semaphores.push_back(std::move(rf_semaphore));

        auto [ifresult, if_fence] = device->createFenceUnique(fci);
        if (ifresult != vk::Result::eSuccess) {
            ERROR("failed to create fence");
        }

        inflight_fences.push_back(std::move(if_fence));
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

vk::UniqueShaderModule
create_shader_module(
    vk::UniqueDevice& device, const std::vector<char>& code) noexcept
{
    vk::ShaderModuleCreateInfo smci(
        {}, code.size(), reinterpret_cast<const uint32_t*>(code.data()));

    auto [result, shader_module] = device->createShaderModuleUnique(smci);
    if (result != vk::Result::eSuccess) {
        ERROR("failed to create shader module");
    }

    return std::move(shader_module);
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
    std::vector<vk::UniqueImageView>,
    vk::UniquePipelineLayout,
    vk::UniquePipeline,
    vk::UniqueRenderPass,
    std::vector<vk::UniqueFramebuffer>,
    vk::UniqueCommandPool,
    std::vector<vk::UniqueCommandBuffer>,
    std::vector<vk::UniqueSemaphore>,
    std::vector<vk::UniqueSemaphore>,
    std::vector<vk::UniqueFence>,
    std::vector<vk::Fence>>
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

    VULKAN_HPP_DEFAULT_DISPATCHER.init(*device);

    auto [swapchain, images, image_format, extent] =
        create_swapchain(device, physical_device, surface, width, height);

    std::vector<vk::UniqueImageView> image_views(
        create_image_views(device, images, image_format));

    vk::UniqueRenderPass render_pass(create_render_pass(device, image_format));

    auto [pipeline_layout, graphics_pipeline] =
        create_graphics_pipeline(device, extent, render_pass);

    auto framebuffers =
        create_framebuffers(device, render_pass, image_views, extent);

    auto command_pool = create_command_pool(device, physical_device, surface);

    auto command_buffers = create_command_buffers(
        device,
        framebuffers,
        render_pass,
        extent,
        graphics_pipeline,
        command_pool);

    auto
        [image_available_semaphores,
         render_finished_semaphores,
         inflight_fences,
         images_inflight] = create_sync_objects(device, images);

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
        std::move(images),
        std::move(image_format),
        std::move(extent),
        std::move(image_views),
        std::move(pipeline_layout),
        std::move(graphics_pipeline),
        std::move(render_pass),
        std::move(framebuffers),
        std::move(command_pool),
        std::move(command_buffers),
        std::move(image_available_semaphores),
        std::move(render_finished_semaphores),
        std::move(inflight_fences),
        std::move(images_inflight)};
}

void
main_loop(
    vk::UniqueDevice&                           device,
    vk::Queue                                   graphics_queue,
    vk::Queue                                   present_queue,
    vk::UniqueSwapchainKHR&                     swapchain,
    const std::vector<vk::UniqueCommandBuffer>& command_buffers,
    const std::vector<vk::UniqueSemaphore>&     image_available_semaphores,
    const std::vector<vk::UniqueSemaphore>&     render_finished_semaphores,
    const std::vector<vk::UniqueFence>&         inflight_fences,
    std::vector<vk::Fence>&                     images_inflight,
    size_t&                                     current_frame,
    GLFWwindow*                                 window) noexcept
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

    device->waitIdle();
}

void
draw_frame(
    vk::UniqueDevice&                           device,
    vk::Queue                                   graphics_queue,
    vk::Queue                                   present_queue,
    vk::UniqueSwapchainKHR&                     swapchain,
    const std::vector<vk::UniqueCommandBuffer>& command_buffers,
    const std::vector<vk::UniqueSemaphore>&     image_available_semaphores,
    const std::vector<vk::UniqueSemaphore>&     render_finished_semaphores,
    const std::vector<vk::UniqueFence>&         inflight_fences,
    std::vector<vk::Fence>&                     images_inflight,
    size_t&                                     current_frame) noexcept
{
    if (vk::Result::eSuccess !=
        device->waitForFences(
            *inflight_fences[current_frame], true, UINT64_MAX)) {
        ERROR("failed to wait for fence");
    }

    uint32_t image_index;
    device->acquireNextImageKHR(
        *swapchain,
        UINT64_MAX,
        *image_available_semaphores[current_frame],
        nullptr,
        &image_index);

    // check if a previous frame is using this image (i.e. there is its
    // fence to wait on)
    if (images_inflight[image_index]) {
        if (vk::Result::eSuccess !=
            device->waitForFences(
                images_inflight[image_index], true, UINT64_MAX)) {
            ERROR("failed to wait for dence");
        }
    }

    // mark the image as now being in use by this frame
    images_inflight[image_index] = *inflight_fences[current_frame];

    vk::Semaphore wait_semaphores[] = {
        *image_available_semaphores[current_frame]};
    vk::PipelineStageFlags wait_stages[] = {
        vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::CommandBuffer cmd_buffers[] = {*command_buffers[image_index]};

    vk::Semaphore signal_semaphores[] = {
        *render_finished_semaphores[current_frame]};

    vk::SubmitInfo submit_info(
        1, wait_semaphores, wait_stages, 1, cmd_buffers, 1, signal_semaphores);

    vk::Fence fences[] = {*inflight_fences[current_frame]};

    if (vk::Result::eSuccess != device->resetFences(1, fences)) {
        ERROR("failed to reset fences");
    }

    vk::SubmitInfo submits[] = {submit_info};

    if (vk::Result::eSuccess !=
        graphics_queue.submit(1, submits, *inflight_fences[current_frame])) {
        ERROR("failed to submit draw command buffer");
    }

    vk::SwapchainKHR swapchains[] = {*swapchain};

    vk::PresentInfoKHR present_info(
        1, signal_semaphores, 1, swapchains, &image_index);

    if (vk::Result::eSuccess != present_queue.presentKHR(present_info)) {
        ERROR("failed to presentKHR");
    }

    present_queue.waitIdle();

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
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

} // namespace
} // namespace application

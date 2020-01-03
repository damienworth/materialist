#include "vulkan_context.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <set>
#include <vector>

#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#include "error_handling.hpp"
#include "glfwwindow.hpp"
#include "spdlog_all.hpp"

namespace vulkan {

struct context {
    size_t current_frame = 0;

    glfw::window       window;
    vk::UniqueInstance instance;

#ifndef NDEBUG
    vk::UniqueDebugUtilsMessengerEXT debug_messenger;
#endif // NDEBUG

    vk::UniqueSurfaceKHR                 surface;
    vk::PhysicalDevice                   physical_device;
    vk::UniqueDevice                     device;
    vk::Queue                            graphics_queue;
    vk::Queue                            present_queue;
    vk::UniqueSwapchainKHR               swapchain;
    std::vector<vk::Image>               images;
    vk::Format                           format;
    vk::Extent2D                         extent;
    std::vector<vk::UniqueImageView>     views;
    vk::UniqueRenderPass                 render_pass;
    vk::UniquePipelineLayout             pipeline_layout;
    vk::UniquePipeline                   graphics_pipeline;
    std::vector<vk::UniqueFramebuffer>   framebuffers;
    vk::UniqueCommandPool                command_pool;
    std::vector<vk::UniqueCommandBuffer> command_buffers;
    std::vector<vk::UniqueSemaphore>     image_avail_semaphores;
    std::vector<vk::UniqueSemaphore>     render_finished_semaphores;
    std::vector<vk::UniqueFence>         inflight_fences;
    std::vector<vk::Fence>               images_inflight;

    context()               = default;
    context(const context&) = delete;
    context& operator=(const context&) = delete;
};

#ifndef NDEBUG

namespace /* anonymous */ {

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

constexpr auto g_validation_layers = std::array{"VK_LAYER_KHRONOS_validation",
                                                "VK_LAYER_LUNARG_api_dump",
                                                "VK_LAYER_LUNARG_demo_layer",
                                                "VK_LAYER_LUNARG_device_"
                                                "simulation",
                                                "VK_LAYER_LUNARG_monitor",
                                                "VK_LAYER_LUNARG_screenshot",
                                                "VK_LAYER_LUNARG_standard_"
                                                "validation",
                                                "VK_LAYER_LUNARG_starter_"
                                                "layer"};

constexpr auto g_device_extensions =
    std::array{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

std::vector<const char*>
get_required_extensions() noexcept
{
    uint32_t     glfw_extension_count = 0;
    const char** glfw_extensions;
    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    std::vector<const char*> extensions;
    extensions.reserve(
        glfw_extension_count +
        1 /* for VK_EXT_DEBUG_UTILS_EXTENSION_NAME in debug builds */);
    std::copy(
        glfw_extensions,
        glfw_extensions + glfw_extension_count,
        std::back_inserter(extensions));

#ifndef NDEBUG
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#else  // NDEBUG
    extensions.shrink_to_fit();
#endif // NDEBUG

    return extensions;
}

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

bool
check_device_extension_support(vk::PhysicalDevice physical_device) noexcept
{
    auto [result, available_extensions] =
        physical_device.enumerateDeviceExtensionProperties();
    if (result != vk::Result::eSuccess) {
        ERROR("failed to enumerate device extension properties");
    }

    std::set<std::string> required_extensions(
        begin(g_device_extensions), end(g_device_extensions));
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
find_queue_families(vk::PhysicalDevice physical_device, vk::SurfaceKHR& surface)
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
            physical_device.getSurfaceSupportKHR(idx, surface);
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
    vk::PhysicalDevice physical_device, vk::SurfaceKHR& surface) noexcept
{
    swapchain_support_details details;
    auto [gscresult, capabilities] =
        physical_device.getSurfaceCapabilitiesKHR(surface);
    if (gscresult != vk::Result::eSuccess) {
        ERROR("failed to get surface capabilities");
    }
    auto [gsfresult, formats] = physical_device.getSurfaceFormatsKHR(surface);
    if (gsfresult != vk::Result::eSuccess) {
        ERROR("failed to get surface formats");
    }
    details.capabilities = std::move(capabilities);
    details.formats      = std::move(formats);

    auto [gspmresult, present_modes] =
        physical_device.getSurfacePresentModesKHR(surface);
    if (gspmresult != vk::Result::eSuccess) {
        ERROR("failed to get surface present modes");
    }
    details.present_modes = std::move(present_modes);
    return details;
}

bool
is_device_suitable(
    vk::PhysicalDevice physical_device, vk::SurfaceKHR& surface) noexcept
{
    auto device_features   = physical_device.getFeatures();
    auto device_properties = physical_device.getProperties();

    bool extensions_supported = check_device_extension_support(physical_device);

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
create_shader_module(vk::Device& device, const std::vector<char>& code) noexcept
{
    vk::ShaderModuleCreateInfo smci(
        {}, code.size(), reinterpret_cast<const uint32_t*>(code.data()));

    auto [result, shader_module] = device.createShaderModuleUnique(smci);
    if (result != vk::Result::eSuccess) {
        ERROR("failed to create shader module");
    }

    return std::move(shader_module);
}

} // namespace

void
initialize_context(context& ctx, int width, int height) noexcept
{
    create_instance(ctx);

    VULKAN_HPP_DEFAULT_DISPATCHER.init(*ctx.instance);

#ifndef NDEBUG
    create_debug_utils_messenger_EXT(ctx);
#endif // NDEBUG

    create_surface(ctx);

    pick_physical_device(ctx);

    create_logical_device(ctx);

    VULKAN_HPP_DEFAULT_DISPATCHER.init(*ctx.device);

    create_swapchain(ctx, width, height);

    create_image_views(ctx);

    create_render_pass(ctx);

    create_graphics_pipeline(ctx);

    create_framebuffers(ctx);

    create_command_pool(ctx);

    create_command_buffers(ctx);

    create_sync_objects(ctx);
}

void
create_debug_utils_messenger_EXT(context& ctx) noexcept
{
    auto& instance = ctx.instance;
    auto  dmci     = populate_debug_messenger_create_info();
    auto [result, debug_messenger] =
        instance->createDebugUtilsMessengerEXTUnique(dmci);
    if (result != vk::Result::eSuccess) {
        ERROR("failed to create debug-utils messenger");
    }

    ctx.debug_messenger = std::move(debug_messenger);
}

bool
check_validation_layer_support() noexcept
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

    for (const char* layer_name : g_validation_layers) {
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

void
create_instance(context& ctx) noexcept
{
#ifndef NDEBUG
    if (!check_validation_layer_support()) {
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
        gsl::narrow<uint32_t>(g_validation_layers.size()),
        g_validation_layers.data(),
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

    ctx.instance = std::move(instance);
}

void
create_surface(context& ctx) noexcept
{
    VkSurfaceKHR surface_tmp;
    if (glfwCreateWindowSurface(
            *ctx.instance, *ctx.window, nullptr, &surface_tmp) != VK_SUCCESS) {
        ERROR("failed to create window surface");
    }

    vk::UniqueSurfaceKHR surface(surface_tmp, *ctx.instance);

    ctx.surface = std::move(surface);
}

void
pick_physical_device(context& ctx) noexcept
{
    vk::PhysicalDevice physical_device;
    auto [result, devices] = ctx.instance->enumeratePhysicalDevices();
    if (result != vk::Result::eSuccess) {
        ERROR("failed to find GPUs with Vulkan support");
    }

    auto right_device = [&ctx](const auto& device) {
        return is_device_suitable(device, *ctx.surface);
    };

    auto suitable_it = std::find_if(begin(devices), end(devices), right_device);
    if (suitable_it != end(devices)) { physical_device = *suitable_it; }

    if (!physical_device) { ERROR("failed to find suitable GPU"); }

    ctx.physical_device = physical_device;
}

void
create_logical_device(context& ctx) noexcept
{
    auto queue_family_properties =
        ctx.physical_device.getQueueFamilyProperties();
    auto graphics_queue_family_index = gsl::narrow<uint32_t>(std::distance(
        begin(queue_family_properties),
        std::find_if(
            begin(queue_family_properties),
            end(queue_family_properties),
            [](const auto& qfp) {
                return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
            })));

    auto [spmresult, present_modes] =
        ctx.physical_device.getSurfacePresentModesKHR(*ctx.surface);
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
        gsl::narrow<uint32_t>(g_validation_layers.size()),
        g_validation_layers.data(),
#else  // Release
        0,
        nullptr,
#endif // NDEBUG
        gsl::narrow<uint32_t>(g_device_extensions.size()),
        g_device_extensions.data());

    create_info.setPEnabledFeatures(&device_features);

    auto [result, device] = ctx.physical_device.createDeviceUnique(create_info);
    if (result != vk::Result::eSuccess) {
        ERROR("failed to create logical device!");
    }

    auto graphics_queue = device->getQueue(graphics_queue_family_index, 0);
    auto present_queue  = device->getQueue(present_queue_family_index, 0);
    ctx.device          = std::move(device);
    ctx.graphics_queue  = std::move(graphics_queue);
    ctx.present_queue   = std::move(present_queue);
}

void
create_swapchain(context& ctx, int width, int height) noexcept
{
    swapchain_support_details swapchain_support =
        query_swapchain_support(ctx.physical_device, *ctx.surface);

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
    auto      indices = find_queue_families(ctx.physical_device, *ctx.surface);
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
        *ctx.surface,
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

    auto [csresult, swapchain] =
        ctx.device->createSwapchainKHRUnique(create_info);
    if (csresult != vk::Result::eSuccess) {
        ERROR("failed to create swapchain");
    }
    auto [gsiresult, images] = ctx.device->getSwapchainImagesKHR(*swapchain);
    if (gsiresult != vk::Result::eSuccess) {
        ERROR("failed to get swapchain images");
    }

    ctx.swapchain = std::move(swapchain);
    ctx.images    = std::move(images);
    ctx.format    = surface_format.format;
    ctx.extent    = extent;
}

void
create_image_views(context& ctx) noexcept
{
    std::vector<vk::UniqueImageView> views;
    views.reserve(ctx.images.size());
    vk::ComponentMapping component_mapping(
        vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity);
    vk::ImageSubresourceRange subresource_range(
        vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
    for (auto image : ctx.images) {
        vk::ImageViewCreateInfo ivci(
            {},
            image,
            vk::ImageViewType::e2D,
            ctx.format,
            component_mapping,
            subresource_range);

        auto [result, view] = ctx.device->createImageViewUnique(ivci);
        if (result != vk::Result::eSuccess) {
            ERROR("failed to create image view");
        }
        views.push_back(std::move(view));
    }

    ctx.views = std::move(views);
}

void
create_render_pass(context& ctx) noexcept
{
    vk::AttachmentDescription attachment_desc(
        {},
        ctx.format,
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

    auto [result, render_pass] = ctx.device->createRenderPassUnique(rpci);
    if (result != vk::Result::eSuccess) {
        ERROR("failed to create render pass");
    }

    ctx.render_pass = std::move(render_pass);
}

void
create_graphics_pipeline(context& ctx) noexcept
{
    auto vert_shader_code = read_file("shaders/shader.vert.spv");
    auto frag_shader_code = read_file("shaders/shader.frag.spv");

    auto&       device = *ctx.device;
    const auto& extent = ctx.extent;

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

    auto dynamic_states =
        std::array{vk::DynamicState::eViewport, vk::DynamicState::eLineWidth};

    vk::PipelineDynamicStateCreateInfo dynamic_state = {};
    dynamic_state.dynamicStateCount                  = dynamic_states.size();
    dynamic_state.pDynamicStates                     = dynamic_states.data();

    auto [cplresult, pipeline_layout] =
        device.createPipelineLayoutUnique(vk::PipelineLayoutCreateInfo());
    if (cplresult != vk::Result::eSuccess) {
        ERROR("failed to create pipeline layout");
    }

    auto shader_stages = std::array{vert_pssci, frag_pssci};

    vk::GraphicsPipelineCreateInfo gpci(
        {},
        shader_stages.size(),
        shader_stages.data(),
        &pvisci,
        &piasci,
        nullptr,
        &pvsci,
        &rasterizer,
        &multisampling,
        nullptr,
        &color_blending,
        nullptr,
        *ctx.pipeline_layout,
        *ctx.render_pass);

    auto [cgpresult, graphics_pipeline] =
        device.createGraphicsPipelinesUnique(nullptr, gpci);
    if (cgpresult != vk::Result::eSuccess) {
        ERROR("failed to create graphics pipeline");
    }

    ctx.pipeline_layout   = std::move(pipeline_layout);
    ctx.graphics_pipeline = std::move(graphics_pipeline.front());
}

void
create_framebuffers(context& ctx) noexcept
{
    std::vector<vk::UniqueFramebuffer> framebuffers;
    framebuffers.reserve(ctx.views.size());

    for (auto const& view : ctx.views) {
        auto                      attachments = std::array{*view};
        vk::FramebufferCreateInfo fci(
            {},
            *ctx.render_pass,
            1,
            &attachments[0],
            ctx.extent.width,
            ctx.extent.height,
            1);

        auto [result, framebuffer] = ctx.device->createFramebufferUnique(fci);
        if (result != vk::Result::eSuccess) {
            ERROR("failed to create framebuffer");
        }
        framebuffers.push_back(std::move(framebuffer));
    }

    ctx.framebuffers = std::move(framebuffers);
}

void
create_command_pool(context& ctx) noexcept
{
    auto indices = find_queue_families(ctx.physical_device, *ctx.surface);

    vk::CommandPoolCreateInfo cpci({}, *indices.graphics_family);

    auto [result, command_pool] = ctx.device->createCommandPoolUnique(cpci);
    if (result != vk::Result::eSuccess) {
        ERROR("failed to create command pool");
    }

    ctx.command_pool = std::move(command_pool);
}

void
create_command_buffers(context& ctx) noexcept
{
    vk::CommandBufferAllocateInfo cbai(
        *ctx.command_pool,
        vk::CommandBufferLevel::ePrimary,
        gsl::narrow<uint32_t>(ctx.framebuffers.size()));

    auto [result, command_buffers] =
        ctx.device->allocateCommandBuffersUnique(cbai);
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
            *ctx.render_pass,
            *ctx.framebuffers[i],
            vk::Rect2D(vk::Offset2D(0.f, 0.f), ctx.extent),
            1,
            &clear_color);

        command_buffers[i]->beginRenderPass(
            render_pass_info, vk::SubpassContents::eInline);

        command_buffers[i]->bindPipeline(
            vk::PipelineBindPoint::eGraphics, *ctx.graphics_pipeline);

        command_buffers[i]->draw(3, 1, 0, 0);
        command_buffers[i]->endRenderPass();

        if (command_buffers[i]->end() != vk::Result::eSuccess) {
            ERROR("failed to record command buffer");
        }
    }

    ctx.command_buffers = std::move(command_buffers);
}

void
create_sync_objects(context& ctx) noexcept
{
    vk::SemaphoreCreateInfo sci;

    vk::FenceCreateInfo fci(vk::FenceCreateFlagBits::eSignaled);

    std::vector<vk::UniqueSemaphore> image_avail_semaphores;
    image_avail_semaphores.reserve(MAX_FRAMES_IN_FLIGHT);
    std::vector<vk::UniqueSemaphore> render_finished_semaphores;
    render_finished_semaphores.reserve(MAX_FRAMES_IN_FLIGHT);
    std::vector<vk::UniqueFence> inflight_fences;
    inflight_fences.reserve(MAX_FRAMES_IN_FLIGHT);
    std::vector<vk::Fence> images_inflight(ctx.images.size());

    auto& device = *ctx.device;

    for (size_t i = 0; i != MAX_FRAMES_IN_FLIGHT; ++i) {
        auto [iaresult, ia_semaphore] = device.createSemaphoreUnique(sci);
        if (iaresult != vk::Result::eSuccess) {
            ERROR("failed to create semaphore");
        }

        auto [rfresult, rf_semaphore] = device.createSemaphoreUnique(sci);
        if (rfresult != vk::Result::eSuccess) {
            ERROR("failed to create semaphore");
        }

        image_avail_semaphores.push_back(std::move(ia_semaphore));
        render_finished_semaphores.push_back(std::move(rf_semaphore));

        auto [ifresult, if_fence] = device.createFenceUnique(fci);
        if (ifresult != vk::Result::eSuccess) {
            ERROR("failed to create fence");
        }

        inflight_fences.push_back(std::move(if_fence));
    }

    ctx.image_avail_semaphores     = std::move(image_avail_semaphores);
    ctx.render_finished_semaphores = std::move(render_finished_semaphores);
    ctx.inflight_fences            = std::move(inflight_fences);
    ctx.images_inflight            = std::move(images_inflight);
}

} // namespace vulkan


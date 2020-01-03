#ifndef MATERIALIST_VULKAN_CONTEXT_HPP
#define MATERIALIST_VULKAN_CONTEXT_HPP

#include <gsl/span>

namespace vulkan {

struct context;

void initialize_context(context&, int, int) noexcept;

#ifndef NDEBUG
void create_debug_utils_messenger_EXT(context&) noexcept;
#endif // NDEBUG

void create_instance(context&) noexcept;

void create_surface(context&) noexcept;

void pick_physical_device(context&) noexcept;

void create_logical_device(context&) noexcept;

void create_swapchain(context&, int, int) noexcept;

void create_image_views(context&) noexcept;

void create_render_pass(context&) noexcept;

void create_graphics_pipeline(context&) noexcept;

void create_framebuffers(context&) noexcept;

void create_command_pool(context&) noexcept;

void create_command_buffers(context&) noexcept;

void create_sync_objects(context& ctx) noexcept;

} // namespace vulkan

#endif // MATERIALIST_VULKAN_CONTEXT_HPP


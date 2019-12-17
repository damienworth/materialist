#ifndef APPLICATION_HELLO_TRIANGLE_HPP
#define APPLICATION_HELLO_TRIANGLE_HPP

#include <vector>

#include <vulkan/vulkan.hpp>

#include "glfwwindow.hpp"

namespace application {

class hello_triangle {
    glfw::window _window;

    vk::UniqueInstance _instance;

#ifndef NDEBUG
    vk::UniqueDebugUtilsMessengerEXT _debug_messenger;
#endif // NDEBUG

    vk::UniqueSurfaceKHR _surface;
    vk::UniqueDevice     _device;

    vk::Queue                            _graphics_queue;
    vk::Queue                            _present_queue;
    vk::UniqueSwapchainKHR               _swapchain;
    std::vector<vk::Image>               _images;
    vk::Format                           _image_format;
    vk::Extent2D                         _extent;
    std::vector<vk::UniqueImageView>     _image_views;
    vk::UniqueRenderPass                 _render_pass;
    vk::UniquePipelineLayout             _pipeline_layout;
    vk::UniquePipeline                   _graphics_pipeline;
    std::vector<vk::UniqueFramebuffer>   _framebuffers;
    vk::UniqueCommandPool                _command_pool;
    std::vector<vk::UniqueCommandBuffer> _command_buffers;
    std::vector<vk::UniqueSemaphore>     _image_available_semaphores;
    std::vector<vk::UniqueSemaphore>     _render_finished_semaphores;
    std::vector<vk::UniqueFence>         _inflight_fences;
    std::vector<vk::Fence>               _images_inflight;

    size_t _current_frame = 0;

#ifndef NDEBUG
    const std::vector<const char*> _validation_layers = {
        "VK_LAYER_KHRONOS_validation",
        "VK_LAYER_LUNARG_api_dump",
        "VK_LAYER_LUNARG_demo_layer",
        "VK_LAYER_LUNARG_device_simulation",
        "VK_LAYER_LUNARG_monitor",
        "VK_LAYER_LUNARG_screenshot",
        "VK_LAYER_LUNARG_standard_validation",
        "VK_LAYER_LUNARG_starter_layer",
        "VK_LAYER_RENDERDOC_Capture"};
#endif // NDEBUG

public:
    void run() noexcept;
};

} // namespace application

#endif // APPLICATION_HELLO_TRIANGLE_HPP

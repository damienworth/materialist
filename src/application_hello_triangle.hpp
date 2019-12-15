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

    vk::UniqueDevice                   _device;
    vk::Queue                          _graphics_queue;
    vk::Queue                          _present_queue;
    vk::UniqueSurfaceKHR               _surface;
    vk::UniqueSwapchainKHR             _swapchain;
    std::vector<vk::Image>             _images;
    vk::Format                         _image_format;
    vk::Extent2D                       _extent;
    std::vector<vk::UniqueImageView>   _image_views;
    vk::UniqueRenderPass               _render_pass;
    VkPipelineLayout                   _pipeline_layout;
    VkPipeline                         _graphics_pipeline;
    std::vector<vk::UniqueFramebuffer> _framebuffers;
    VkCommandPool                      _command_pool;
    std::vector<VkCommandBuffer>       _command_buffers;
    std::vector<VkSemaphore>           _image_available_semaphores;
    std::vector<VkSemaphore>           _render_finished_semaphores;
    std::vector<VkFence>               _inflight_fences;
    std::vector<VkFence>               _images_inflight;

    size_t _current_frame = 0;

#ifndef NDEBUG
    const std::vector<const char*> _validation_layers = {
        "VK_LAYER_KHRONOS_validation"};
#endif // NDEBUG

public:
    void run() noexcept;
};

} // namespace application

#endif // APPLICATION_HELLO_TRIANGLE_HPP

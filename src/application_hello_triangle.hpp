#ifndef APPLICATION_HELLO_TRIANGLE_HPP
#define APPLICATION_HELLO_TRIANGLE_HPP

#include <vector>

#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

namespace application {

class hello_triangle {
    GLFWwindow* _window = nullptr;
    const int   _WIDTH  = 800;
    const int   _HEIGHT = 600;

    vk::UniqueInstance _instance;

#ifndef NDEBUG
    vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic>
        _debug_messenger;
#endif // NDEBUG

    vk::UniqueDevice             _device;
    vk::Queue                    _graphics_queue;
    vk::Queue                    _present_queue;
    vk::UniqueSurfaceKHR         _surface;
    vk::UniqueSwapchainKHR       _swapchain;
    std::vector<vk::UniqueImage> _swapchain_images;
    vk::Format                   _swapchain_image_format;
    vk::Extent2D                 _swapchain_extent;
    std::vector<VkImageView>     _swapchain_image_views;
    VkRenderPass                 _render_pass;
    VkPipelineLayout             _pipeline_layout;
    VkPipeline                   _graphics_pipeline;
    std::vector<VkFramebuffer>   _swapchain_framebuffers;
    VkCommandPool                _command_pool;
    std::vector<VkCommandBuffer> _command_buffers;
    std::vector<VkSemaphore>     _image_available_semaphores;
    std::vector<VkSemaphore>     _render_finished_semaphores;
    std::vector<VkFence>         _inflight_fences;
    std::vector<VkFence>         _images_inflight;
    size_t                       _current_frame = 0;

#ifndef NDEBUG
    const std::vector<const char*> _validation_layers = {
        "VK_LAYER_KHRONOS_validation"};
#endif // NDEBUG

public:
    void run() noexcept;
};

} // namespace application

#endif // APPLICATION_HELLO_TRIANGLE_HPP

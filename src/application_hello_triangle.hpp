#ifndef APPLICATION_HELLO_TRIANGLE_HPP
#define APPLICATION_HELLO_TRIANGLE_HPP

#include <vector>

#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

namespace application {

class hello_triangle {
    GLFWwindow*                  _window = nullptr;
    const int                    _WIDTH  = 800;
    const int                    _HEIGHT = 600;

#ifndef NDEBUG
    vk::DebugUtilsMessengerEXT _debug_messenger;
#endif // NDEBUG

    vk::UniqueInstance           _instance;
    vk::UniqueDevice             _device;
    VkQueue                      _graphics_queue;
    VkQueue                      _present_queue;
    VkSwapchainKHR               _swapchain;
    std::vector<VkImage>         _swapchain_images;
    VkFormat                     _swapchain_image_format;
    VkExtent2D                   _swapchain_extent;
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

    VkSurfaceKHR _surface;

#ifndef NDEBUG
    const std::vector<const char*> _validation_layers = {
        "VK_LAYER_KHRONOS_validation"};
#endif // NDEBUG

public:
    void run() noexcept;
};

} // namespace application

#endif // APPLICATION_HELLO_TRIANGLE_HPP

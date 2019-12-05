#ifndef APPLICATION_HELLO_TRIANGLE_HPP
#define APPLICATION_HELLO_TRIANGLE_HPP

#include <vector>

#include <GLFW/glfw3.h>

namespace application {

class hello_triangle {
    GLFWwindow*              _window = nullptr;
    const int                _WIDTH  = 800;
    const int                _HEIGHT = 600;
    VkInstance               _instance;
    VkDevice                 _device;
    VkQueue                  _graphics_queue;
    VkQueue                  _present_queue;
    VkSwapchainKHR           _swapchain;
    std::vector<VkImage>     _swapchain_images;
    VkFormat                 _swapchain_image_format;
    VkExtent2D               _swapchain_extent;
    std::vector<VkImageView> _swapchain_image_views;
    VkPipelineLayout         _pipeline_layout;

#ifndef NDEBUG
    VkDebugUtilsMessengerEXT _debug_messenger;
#endif // NDEBUG

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
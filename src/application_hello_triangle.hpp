#ifndef APPLICATION_HELLO_TRIANGLE_HPP
#define APPLICATION_HELLO_TRIANGLE_HPP

#ifndef NDEBUG
#include <vector>
#endif // NDEBUG

#include <GLFW/glfw3.h>

namespace application {

class hello_triangle {
    GLFWwindow* _window = nullptr;
    const int   _WIDTH  = 800;
    const int   _HEIGHT = 600;
    VkInstance  _instance;

#ifndef NDEBUG
    VkDebugUtilsMessengerEXT _debug_messenger;
#endif // NDEBUG

#ifndef NDEBUG
    const std::vector<const char*> _validation_layers = {
        "VK_LAYER_KHRONOS_validation"};
#endif // NDEBUG

public:
    void run() noexcept;
};

} // namespace application

#endif // APPLICATION_HELLO_TRIANGLE_HPP
#ifndef GLFWWINDOW_WRAPPER_HPP
#define GLFWWINDOW_WRAPPER_HPP

#include <string_view>

#include <GLFW/glfw3.h>

namespace glfw {

class window {
    GLFWwindow* _wnd    = nullptr;
    int         _width  = 800;
    int         _height = 600;

public:
    window() = default;

    bool
    create(std::string_view caption, int width, int height) noexcept
    {
        _width  = width;
        _height = height;
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        _wnd =
            glfwCreateWindow(_width, _height, caption.data(), nullptr, nullptr);
        return _wnd != nullptr;
    }

    ~window()
    {
        glfwDestroyWindow(_wnd);
        glfwTerminate();
    }

    int
    w() const noexcept
    {
        return _width;
    }

    int
    h() const noexcept
    {
        return _height;
    }

    window&
    operator=(GLFWwindow* glfwwindow)
    {
        _wnd = glfwwindow;
        return *this;
    }

    const GLFWwindow* operator*() const { return _wnd; }

    GLFWwindow* operator*() { return _wnd; }
};

} // namespace glfw

#endif // GLFWWINDOW_WRAPPER_HPP


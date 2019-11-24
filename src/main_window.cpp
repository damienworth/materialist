#include "main_window.hpp"

#include "spdlog_all.hpp"

namespace mw {

// the following globals are extern by design

// window size
int _width  = 640;
int _height = 480;

// window handle
GLFWwindow* _window = nullptr;

bool
create_window(std::string_view title, int width, int height) noexcept
{
    _window = glfwCreateWindow(
        _width = width, _height = height, title.data(), nullptr, nullptr);
    if (!_window) {
        spdlog::error("could not open main window with GLFW3");
        glfwTerminate();
        return false;
    }

    return true;
}

// a call-back function
void
window_size_callback(GLFWwindow*, int width, int height) noexcept
{
    _width  = width;
    _height = height;
    /* update any perspective matrices used here */
}

void
main_loop(GLuint vertex_array_object, GLuint shader_programme) noexcept
{
    while (!glfwWindowShouldClose(_window)) {
        // wipe the drawing surface clear
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, _width, _height);
        glUseProgram(shader_programme);
        glBindVertexArray(vertex_array_object);
        // draw points 0-3 from the currently bound VAO with current in-use
        // shader
        glDrawArrays(GL_TRIANGLES, 0, 3);
        // update other events like input handling
        glfwPollEvents();
        // put the stuff we've been drawing onto the display
        glfwSwapBuffers(_window);

        if (GLFW_PRESS == glfwGetKey(_window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(_window, 1);
        }
    }
}

} // namespace mw

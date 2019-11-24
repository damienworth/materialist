#include "main_window.hpp"

#include "spdlog_all.hpp"

#include "fps_counter.hpp"

namespace main_window {

using namespace std::string_literals;

// the following globals are extern by design

// window size
int _width  = 640;
int _height = 480;

// window variables
GLFWwindow* _window = nullptr;
auto        _title  = "untitled"s;

bool
create(std::string_view title, int width, int height) noexcept
{
    _title  = title;
    _window = glfwCreateWindow(
        _width = width, _height = height, _title.c_str(), nullptr, nullptr);
    if (!_window) {
        spdlog::error("could not open main window with GLFW3");
        glfwTerminate();
        return false;
    }

    glfwSetWindowSizeCallback(_window, resize_callback);
    glfwMakeContextCurrent(_window);

    return true;
}

// a call-back function
void
resize_callback(GLFWwindow*, int width, int height) noexcept
{
    _width  = width;
    _height = height;
    /* update any perspective matrices used here */
}

void
loop(GLuint vertex_array_object, GLuint shader_programme) noexcept
{
    while (!glfwWindowShouldClose(_window)) {
#ifndef NDEBUG
        fps_counter::update(_window, _title);
#endif // NDEBUG

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

} // namespace main_window

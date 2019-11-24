#include <array>
#include <cstdlib>

// logging
#include "spdlog_all.hpp"

// clang-format off
// opengl libs
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <glm/glm.hpp>

#include "error_callback.hpp"

using namespace glm;
using spdlog::debug;
using spdlog::error;
using spdlog::info;
using std::array;

int
main(int, char**)
{
    #ifndef NDEBUG
    spdlog::set_level(spdlog::level::debug);
    #endif // NDEBUG

    debug("starting GLFW3 {}", glfwGetVersionString());
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        error("could not start GLFW3");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window =
        glfwCreateWindow(640, 480, "hello triangle", nullptr, nullptr);
    if (!window) {
        error("could not open window with GLFW3");
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);

    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();

    // get version info
    const auto renderer = glGetString(GL_RENDERER); // get renderer string
    const auto version  = glGetString(GL_VERSION);  // version as a string
    info("Renderer: {}", renderer);
    info("OpenGL version supported {}", version);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    const auto points =
        array{0.0f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f};

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        points.size() * sizeof(float),
        points.data(),
        GL_STATIC_DRAW);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    const auto vertex_shader =
        "#version 400\n"
        "in vec3 vp;"
        "void main() {"
        "  gl_Position = vec4(vp, 1.0);"
        "}";

    const auto fragment_shader =
        "#version 400\n"
        "out vec4 frag_colour;"
        "void main() {"
        "  frag_colour = vec4(0.5, 0.0, 0.5, 1.0);"
        "}";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, nullptr);
    glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, nullptr);
    glCompileShader(fs);

    GLuint shader_programme = glCreateProgram();
    glAttachShader(shader_programme, fs);
    glAttachShader(shader_programme, vs);
    glLinkProgram(shader_programme);

    glClearColor(0.6f, 0.6f, 0.8f, 1.f);

    while(!glfwWindowShouldClose(window)) {
        // wipe the drawing surface clear
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader_programme);
        glBindVertexArray(vao);
        // draw points 0-3 from the currently bound VAO with current in-use shader
        glDrawArrays(GL_TRIANGLES, 0, 3);
        // update other events like input handling
        glfwPollEvents();
        // put the stuff we've been drawing onto the display
        glfwSwapBuffers(window);
    }

    // close GL context and any other GLFW resources
    glfwTerminate();
    return EXIT_SUCCESS;
}

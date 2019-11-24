#include <array>
#include <cstdlib>

#include "opengl_all.hpp"
#include "opengl_initialize.hpp"
#include "spdlog_all.hpp"

#include "log_gl_params.hpp"
#include "main_window.hpp"
#include "shaders.hpp"

#include <glm/glm.hpp>

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

    opengl::initialize();

    GLFWmonitor*       mon   = glfwGetPrimaryMonitor();
    const GLFWvidmode* vmode = glfwGetVideoMode(mon);
    if (!main_window::create("Materialist", vmode->width, vmode->height)) {
        return EXIT_FAILURE;
    }

    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();

    gl_params::log();

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

    glClearColor(0.6f, 0.6f, 0.8f, 1.f);

    auto shader_programme = shaders::create_programme(
        "../glsl/vertex.glsl", "../glsl/fragment.glsl", {1.f, 0.2f, 0.5f, 1.f});
    main_window::loop(vao, shader_programme);

    // close GL context and any other GLFW resources
    glfwTerminate();
    return EXIT_SUCCESS;
}

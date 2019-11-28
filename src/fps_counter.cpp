#include "fps_counter.hpp"

#include "fmtlib_all.hpp"

namespace fps_counter {

void
update(GLFWwindow* window, std::string_view title) noexcept
{
    static auto previous_seconds = glfwGetTime();
    static int  frame_count;
    auto        current_seconds = glfwGetTime();
    auto        elapsed_seconds = current_seconds - previous_seconds;
    if (elapsed_seconds > 0.25) {
        previous_seconds = current_seconds;
        double     fps   = static_cast<double>(frame_count) / elapsed_seconds;
        const auto title_fps = fmt::format("{} @ fps: {:.2f}", title, fps);
        glfwSetWindowTitle(window, title_fps.c_str());
        frame_count = 0;
    }
    frame_count++;
}

} // namespace fps_counter

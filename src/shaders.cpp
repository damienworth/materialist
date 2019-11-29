#include "shaders.hpp"

#include <fstream>
#include <iterator>
#include <string>
#include <string_view>

#include "fmtlib_all.hpp"
#include "spdlog_all.hpp"

#include "print_info_log.hpp"

using fmt::format;
using spdlog::error;
using std::istream_iterator;

namespace {

template <typename FileType>
size_t
file_size(FileType& file) noexcept
{
    std::streampos file_size;
    file.seekg(0, std::ios::end);
    file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    return file_size;
}

std::string
load_shader_text(std::string_view path) noexcept
{
    using std::ifstream;

    ifstream file(path.data(), std::ios::binary);
    if (!file) { error("failed to open shader file {}", path); }

    file.unsetf(std::ios::skipws); // don't skip whitespaces and eols

    std::string text;
    text.reserve(file_size(file));

    text.insert(
        text.begin(), istream_iterator<char>(file), istream_iterator<char>());

    file.close();

    return text;
}
} // namespace

namespace shaders {

std::optional<GLuint>
load_vertex_shader(std::string_view path) noexcept
{
    const auto text   = load_shader_text(path);
    const auto c_text = text.c_str();
    GLuint     vs     = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &c_text, nullptr);
    glCompileShader(vs);
    int params = -1;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &params);
    if (GL_TRUE != params) {
        error("shader {} did not compile", vs);
        print_shader_info_log(vs);
        return {};
    }

    return vs;
}

std::optional<GLuint>
load_fragment_shader(std::string_view path) noexcept
{
    const auto text   = load_shader_text(path);
    const auto c_text = text.c_str();
    GLuint     fs     = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &c_text, nullptr);
    glCompileShader(fs);
    int params = -1;
    glGetShaderiv(fs, GL_COMPILE_STATUS, &params);
    if (GL_TRUE != params) {
        error("shader {} did not compile", fs);
        print_shader_info_log(fs);
        return {};
    }

    return fs;
}

std::optional<GLuint>
create_programme(
    std::string_view vertex_path, std::string_view fragment_path) noexcept
{
    auto vs = load_vertex_shader(vertex_path);
    auto fs = load_fragment_shader(fragment_path);

    GLuint programme = glCreateProgram();
    glAttachShader(programme, *fs);
    glAttachShader(programme, *vs);
    glLinkProgram(programme);

    int params = -1;
    glGetProgramiv(programme, GL_LINK_STATUS, &params);
    if (GL_TRUE != params) {
        error("failed to link {}", programme);
        print_programme_info_log(programme);
        return {};
    }

    return programme;
}
} // namespace shaders
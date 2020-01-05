#ifndef MATERIALIST_HPP
#define MATERIALIST_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <limits>
#include <optional>
#include <set>
#include <string_view>
#include <tuple>
#include <vector>

#include <GLFW/glfw3.h>
#include <gsl/gsl>
#include <vulkan/vulkan.hpp>

#include "application.hpp"
#include "glfwwindow.hpp"
#include "vulkan_context.hpp"

#include "error_handling.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

// implementation starts here
#include "vulkan_context.inl"

#include "application.inl"

#endif // MATERIALIST_HPP


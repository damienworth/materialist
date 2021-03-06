#ifndef SPDLOG_ALL_HPP
#define SPDLOG_ALL_HPP

#ifdef __linux__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Winit-self"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wsign-promo"
#pragma GCC diagnostic ignored "-Wunreachable-code"
#endif // __linux__

#include <cassert>
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

#ifdef __linux__
#pragma GCC diagnostic pop
#endif // __linux__

#endif // SPDLOG_ALL_HPP

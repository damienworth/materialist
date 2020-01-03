#include <cstdlib> // EXIT_SUCCESS

#include "materialist.hpp"

int
main(int, char**)
{
#ifndef NDEBUG
    spdlog::set_level(spdlog::level::debug);
#endif // NDEBUG

    application::main_loop();

    return EXIT_SUCCESS;
}


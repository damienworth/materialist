#include <cstdlib> // EXIT_SUCCESS / EXIT_FAILURE

#include "spdlog_all.hpp"

#include "application_hello_triangle.hpp"

int
main(int, char**)
{
#ifndef NDEBUG
    spdlog::set_level(spdlog::level::debug);
#endif // NDEBUG

    application::hello_triangle app;

    app.run();

    return EXIT_SUCCESS;
}

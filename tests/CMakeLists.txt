include(GoogleTest)

add_subdirectory(extern)

add_executable(materialist_tests
    src/materialist_tests.cpp)

target_link_libraries(materialist_tests
PRIVATE
    gmock_main)

gtest_discover_tests(materialist_tests "" AUTO)

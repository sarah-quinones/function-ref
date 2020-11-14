include(../cmake/doctest.cmake)

include_directories(include)

add_library(doctest_main STATIC src/doctest_main.cpp)

add_executable(generic_test src/test_generic.cpp)
target_link_libraries(generic_test doctest_main function-ref)
doctest_discover_tests(generic_test)

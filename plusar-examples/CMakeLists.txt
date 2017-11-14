cmake_minimum_required(VERSION 3.10)
project(plusar-examples)

include_directories(
    ../plusar-core/include
)

add_executable(example-basic example_basic.cpp)
add_dependencies(example-basic plusar-core)
target_link_libraries(example-basic plusar-core)

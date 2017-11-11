cmake_minimum_required(VERSION 3.10)
project(plusar-tests)

set(HEADERS
)

set(SOURCES
    main.cpp
)

include_directories(
    ../plusar-core/include
)

add_executable(plusar-tests ${HEADERS} ${SOURCES})
add_dependencies(plusar-tests plusar-core)
target_link_libraries(plusar-tests plusar-core)

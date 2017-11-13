cmake_minimum_required(VERSION 3.10)
project(plusar-core)

set(HEADERS
    src/environment.hpp
    src/stream.hpp
    src/optional.hpp
    src/memory.hpp
    src/type_traits.hpp
)

set(SOURCES
    src/environment.cpp
)

add_library(plusar-core STATIC ${HEADERS} ${SOURCES})

cmake_minimum_required(VERSION 3.10)
project(plusar-core)

set(HEADERS
    src/environment.hpp
    src/stream.hpp
)

set(SOURCES
    src/environment.cpp
)

add_library(plusar-core STATIC ${HEADERS} ${SOURCES})

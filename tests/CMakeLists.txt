cmake_minimum_required(VERSION 3.10)
project(plusar-tests)

set(HEADERS
    catch.hpp
)

set(SOURCES
    main.cpp
    test_stream.cpp
)

include_directories(
    ../include
)

add_executable(plusar-tests ${HEADERS} ${SOURCES})

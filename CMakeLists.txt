# usage:
# cmake -DCMAKE_BUILD_TYPE=Release CMakeLists.txt
# cmake -DCMAKE_BUILD_TYPE=Debug CMakeLists.txt
# cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo CMakeLists.txt
# cmake -DCMAKE_BUILD_TYPE=MinSizeRel CMakeLists.txt
# cmake -G "Visual Studio 9 2008 Win64"
# cmake -G "Visual Studio 9 2008"
# cmake -G "Visual Studio 12"
# cmake -DCMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles" ..
# View help for cmake: "cmake --help"

cmake_minimum_required(VERSION 3.10)
project(plusar)

set(PLUSAR_MAJOR_VERSION 1)
set(PLUSAR_MINOR_VERSION 0)
set(PLUSAR_PATCH_VERSION 0)

set(PLUSAR_VERSION ${PLUSAR_MAJOR_VERSION}.${PLUSAR_MINOR_VERSION}.${PLUSAR_PATCH_VERSION})

#*********************************************************
# set build type
#*********************************************************
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

#*********************************************************
# module path
#*********************************************************
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

#*********************************************************
# libraries directory
#*********************************************************
set(LIBS_DIR "${CMAKE_SOURCE_DIR}/libs")

#*********************************************************
# output directory
#*********************************************************
set(PLUSAR_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/output)

#*********************************************************
# BITNESS
#*********************************************************
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(BITNESS 64)
    add_definitions(-DPLUSAR_X64)
    message("Bitness: 64 bit")
else()
    set(BITNESS 32)
    add_definitions(-DPLUSAR_X86)
    message("Bitness: 32 bit")
endif()

#*********************************************************
# common compiler flags
#*********************************************************
if (MSVC)
    add_definitions(/W4)
elseif (CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-Wall)
else ()
    message("Unknown compiler")
endif ()

#*********************************************************
# output file prefixes
#*********************************************************
if(BITNESS EQUAL 32)
    set(PLUSAR_RELEASE_POSTFIX        "")
    set(PLUSAR_DEBUG_POSTFIX          "-d")
    set(PLUSAR_MINSIZEREL_POSTFIX     "-rm")
    set(PLUSAR_RELWITHDEBINFO_POSTFIX "-rd")
else()
    set(PLUSAR_RELEASE_POSTFIX        "64")
    set(PLUSAR_DEBUG_POSTFIX          "64-d")
    set(PLUSAR_MINSIZEREL_POSTFIX     "64-rm")
    set(PLUSAR_RELWITHDEBINFO_POSTFIX "64-rd")
endif()

set(CMAKE_RELEASE_POSTFIX           ${PLUSAR_RELEASE_POSTFIX})
set(CMAKE_DEBUG_POSTFIX             ${PLUSAR_DEBUG_POSTFIX})
set(CMAKE_MINSIZEREL_POSTFIX        ${PLUSAR_MINSIZEREL_POSTFIX})
set(CMAKE_RELWITHDEBINFO_POSTFIX    ${PLUSAR_RELWITHDEBINFO_POSTFIX})

#*********************************************************
# output directories
#*********************************************************
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PLUSAR_OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PLUSAR_OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PLUSAR_OUTPUT_DIR})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(plusar-core     plusar-core)
add_subdirectory(plusar-tests    plusar-tests)
add_subdirectory(plusar-examples plusar-examples)

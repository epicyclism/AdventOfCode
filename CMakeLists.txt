cmake_minimum_required(VERSION 3.19.0)
project(AdventOfCode VERSION 0.1.0)

include(FetchContent)

FetchContent_Declare(
ctre
GIT_REPOSITORY https://github.com/hanickadot/compile-time-regular-expressions.git
)

find_path(BOOST_INC Boost)
include_directories(${BOOST_INC})
find_package(Boost 1.75)

FetchContent_MakeAvailable(ctre)
include_directories(${ctre_SOURCE_DIR})
message( STATUS "ctre=" ${ctre_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(2020)
add_subdirectory(2019)
add_subdirectory(2018)

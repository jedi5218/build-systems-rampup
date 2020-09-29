cmake_minimum_required(VERSION 3.10)

project(build-systems-rampup VERSION 1.0)
add_subdirectory(src/common)
add_subdirectory(src/server)
add_subdirectory(src/client)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
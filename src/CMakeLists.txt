cmake_minimum_required(VERSION 3.10)

project(build-systems-rampup VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
include_directories( .)
add_executable(client client/main.cpp)
add_executable(server server/main.cpp)
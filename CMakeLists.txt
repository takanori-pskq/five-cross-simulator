cmake_minimum_required(VERSION 3.10)

project(FiveCross)
add_executable(fc main.cpp fivecross.cpp)
set(CMAKE_CXX_FLAGS "-O3 -std=c++17 -Wall")

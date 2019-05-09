cmake_minimum_required(VERSION 3.12)
project(RayTracer)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(RayTracer main.cpp src/scene_io.cpp)
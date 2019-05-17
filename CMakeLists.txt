cmake_minimum_required(VERSION 3.8)
project(RayTracer)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(/usr/local/include)


add_executable(RayTracer main.cpp src/scene_io.cpp include/scene.hpp src/scene.cpp 
    include/renderer.hpp src/renderer.cpp include/camera.hpp include/light.hpp 
        include/primitives/sphere.hpp include/material.hpp include/primitives/mesh.hpp 
        include/vertex.hpp src/sphere.cpp src/mesh.cpp include/dir_light.hpp)
target_link_libraries(RayTracer ${OpenCV_LIBS})

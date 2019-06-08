cmake_minimum_required(VERSION 3.8)
project(PathTracer)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")

find_package(OpenCV REQUIRED)
find_package(ZLIB REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(/usr/local/include)

add_executable(PathTracer main.cpp src/scene_io.cpp include/scene.hpp src/scene.cpp 
    include/renderer.hpp src/renderer.cpp include/camera.hpp include/light.hpp 
        include/primitives/sphere.hpp include/material.hpp include/primitives/mesh.hpp 
        include/vertex.hpp src/sphere.cpp include/aabb.hpp include/kd_tree.hpp 
		src/kd_tree.cpp src/mesh.cpp include/dir_light.hpp include/tinyobjloader.hpp 
		include/scene_loaders.hpp src/material.cpp include/payload.hpp include/utils.hpp
        include/area_light.hpp	include/tinyxml/tinyxml2.h include/tinyxml/tinyxml2.cpp include/photon.hpp src/photon_integrator.cpp include/photon_integrator.hpp)
target_link_libraries(PathTracer ${OpenCV_LIBS})

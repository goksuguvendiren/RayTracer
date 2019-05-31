cmake_minimum_required(VERSION 3.8)
project(RayTracer)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")

find_package(OpenCV REQUIRED)
find_package(ZLIB REQUIRED)

#include_directories(${ZLIB_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/fbx)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(/usr/local/include)

add_executable(RayTracer main.cpp src/scene_io.cpp include/scene.hpp src/scene.cpp 
    include/renderer.hpp src/renderer.cpp include/camera.hpp include/light.hpp 
        include/primitives/sphere.hpp include/material.hpp include/primitives/mesh.hpp 
        include/vertex.hpp src/sphere.cpp include/aabb.hpp include/kd_tree.hpp 
		src/kd_tree.cpp src/mesh.cpp include/dir_light.hpp include/OBJ_Loader.hpp)
if (BVH_DISABLED)
	target_compile_definitions(RayTracer PUBLIC BVH_DISABLED)
endif()
if (THREADS_ENABLED)
target_compile_definitions(RayTracer PUBLIC THREADS_ENABLED)
endif()
target_link_libraries(RayTracer ${OpenCV_LIBS})
#target_link_libraries(RayTracer ${ZLIB_LIBRARIES})

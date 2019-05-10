//
// Created by Göksu Güvendiren on 2019-05-09.
//

#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include <opencv2/opencv.hpp>
#include "camera.hpp"
#include "light.hpp"
#include "scene_io.h"
#include "primitives/sphere.hpp"
#include "primitives/mesh.hpp"

namespace rtr
{
    class camera;
    class scene
    {
    public:
        scene(SceneIO* io);
//        scene(std::unique_ptr<SceneIO> io);

        const rtr::camera& get_camera() const { return camera; }

    private:
        glm::vec3 backgroundColor;
        glm::vec3 ambientLight;
        float shadowRayEpsilon          = 1e-3;
        float intersectionTestEpsilon   = 1e-6;
        float maxRecursionDepth;

        rtr::camera camera;
//        std::vector<gpt::Camera> cameras;
//        std::vector<glm::vec3> vertices;
//        std::map<std::string, glm::mat4> transformations;
//
        std::vector<rtr::primitives::sphere> spheres;
//        std::vector<gpt::Triangle> triangles;
        std::vector<rtr::primitives::mesh> meshes;
//
//        std::vector<gpt::Shape*> shapes;
//
//        std::map<int, std::unique_ptr<gpt::Material>> materials;
//        std::vector<std::unique_ptr<gpt::Light>> lights;
        std::vector<rtr::light> lights;
//        std::vector<gpt::LightMesh> light_meshes;
    };
}
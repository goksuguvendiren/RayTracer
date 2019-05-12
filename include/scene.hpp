//
// Created by Göksu Güvendiren on 2019-05-09.
//

#pragma once

#include <vector>
#include <optional>
#include <glm/vec3.hpp>
#include <opencv2/opencv.hpp>
#include "camera.hpp"
#include "light.hpp"
#include "scene_io.h"
#include "payload.hpp"
#include "primitives/sphere.hpp"
#include "primitives/mesh.hpp"

namespace rtr
{
    class camera;
    class ray;
    
    // namespace primitives{
    //     class sphere;
    //     class mesh;
    // }

    class scene
    {
    public:
        scene(SceneIO* io);
        const rtr::camera& get_camera() const { return camera; }

        std::optional<rtr::payload> hit(const rtr::ray& ray) const;

        const std::vector<rtr::light>& lights() const { return lghts; }

    private:
        glm::vec3 backgroundColor;
        glm::vec3 ambientLight;
        float shadowRayEpsilon          = 1e-3;
        float intersectionTestEpsilon   = 1e-6;
        float maxRecursionDepth;

        rtr::camera camera;

        std::vector<rtr::primitives::sphere> spheres;
        std::vector<rtr::primitives::mesh> meshes;
        std::vector<rtr::light> lghts;
    };
}
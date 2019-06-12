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
#include "dir_light.hpp"

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
        scene() = default;
        const rtr::camera& get_camera() const { return camera; }

        std::optional<rtr::payload> hit(const rtr::ray& ray) const;

        template <class FnT>
        void for_each_light(FnT&& func) const
        {
            std::for_each(lghts.begin(), lghts.end(), func);
            std::for_each(dir_lghts.begin(), dir_lghts.end(), func);
        }

        const std::vector<rtr::light>& lights() const { return lghts; }
        const std::vector<rtr::dir_light>& dir_lights() const { return dir_lghts; }

        rtr::camera camera;
        glm::vec3 trace(const rtr::ray& ray) const;
        glm::vec3 photon_trace(const rtr::ray& ray) const;
        glm::vec3 shadow_trace(const rtr::ray& ray, float light_distance) const;
        
        void print() const;
        
//    private:
        glm::vec3 background_color;
        glm::vec3 ambient_light;
        float shadow_ray_epsilon          = 1e-3;
        float intersection_test_epsilon   = 1e-6;
        float max_recursion_depth         = 5;

        std::vector<rtr::primitives::sphere> spheres;
        std::vector<rtr::primitives::mesh> meshes;
        std::vector<rtr::light> lghts;
        std::vector<rtr::dir_light> dir_lghts;
    };
}

//
// Created by Göksu Güvendiren on 2019-05-09.
//

#pragma once

#include <functional>
#include <string>
#include <optional>
#include <glm/vec3.hpp>
#include <material.hpp>
#include "ray.hpp"

namespace rtr
{
    struct payload;
    namespace primitives
    {
        
        class sphere
        {
        public:
            sphere(const std::string& nm, const glm::vec3& pos, float r, const glm::vec3& x_ax, float x_l,
                   const glm::vec3& y_ax, float y_l, const glm::vec3& z_ax, float z_l) :
                   name(nm), origin(pos), radius(r), x_axis(x_ax), x_len(x_l), y_axis(y_ax), y_len(y_l),
                   z_axis(z_ax), z_len(z_l) {}

            sphere(const std::string& nm, const glm::vec3& pos, float r, const rtr::material& m) :
            name(nm), origin(pos), radius(r)
            {
                materials.push_back(m);
            }

            std::vector<material> materials;
            std::optional<rtr::payload> hit(const rtr::ray& ray) const;

        // private:
            std::string name;
            int id;

            glm::vec3 origin;
            float radius;

            glm::vec3 x_axis;
            float x_len;
            glm::vec3 y_axis;
            float y_len;
            glm::vec3 z_axis;
            float z_len;
        };
    }
}

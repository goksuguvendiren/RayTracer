//
// Created by Göksu Güvendiren on 2019-05-10.
//
#pragma once

#include <glm/vec3.hpp>
#include <variant>
#include <primitives/mesh.hpp>
#include <primitives/sphere.hpp>
#include "ray.hpp"

namespace rtr
{
    struct payload
    {
        glm::vec3 hit_normal;
        glm::vec3 hit_pos;
        rtr::ray ray;
        float param;
        std::variant<const rtr::primitives::mesh*, const rtr::primitives::sphere*> object;
    };
} // namespace rtr

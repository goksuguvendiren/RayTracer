//
// Created by Göksu Güvendiren on 2019-05-10.
//
#pragma once

#include <glm/vec3.hpp>
#include "ray.hpp"

namespace rtr
{
    struct payload
    {
        glm::vec3 hit_normal;
        glm::vec3 hit_pos;
        rtr::ray ray;
        float param;
    };
} // namespace rtr

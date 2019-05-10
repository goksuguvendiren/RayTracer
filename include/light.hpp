//
// Created by Göksu Güvendiren on 2019-05-09.
//

#pragma once

#include <glm/vec3.hpp>

namespace rtr
{
    class light
    {
    public:
        light(const glm::vec3& pos, const glm::vec3& col) : position(pos), color(col) {}
        glm::vec3 position;
        glm::vec3 color;
    };
}
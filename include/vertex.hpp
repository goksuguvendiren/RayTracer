//
// Created by Göksu Güvendiren on 2019-05-09.
//

#pragma once

#include <glm/vec3.hpp>

namespace rtr
{
    class vertex
    {
        glm::vec3 position;
        glm::vec3 normal;

        long material_index;
        float u, v;

    public:
        vertex(const glm::vec3& pos, const glm::vec3& n, long mi, float s, float t) : position(pos), normal(n),
        material_index(mi), u(s), v(t) {}
    };
}
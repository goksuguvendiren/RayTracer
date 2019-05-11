//
// Created by Göksu Güvendiren on 2019-05-09.
//

#pragma once

#include <glm/vec3.hpp>

namespace rtr
{
    class vertex
    {
        glm::vec3 poss;

        long material_index;
        float u, v;

    public:
        vertex() = default;
        vertex(const glm::vec3& pos, const glm::vec3& n, long mi, float s, float t) : poss(pos), normal(n),
        material_index(mi), u(s), v(t) {}

        glm::vec3 normal;

        glm::vec3 position() const { return poss; }
//        glm::vec3 normal() const { return norm; }

    };
}
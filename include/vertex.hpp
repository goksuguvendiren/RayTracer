//
// Created by Göksu Güvendiren on 2019-05-09.
//

#pragma once

#include <glm/vec3.hpp>
#include "material.hpp"

namespace rtr
{
    class vertex
    {
        glm::vec3 poss;

        float u, v;

    public:
        vertex() = default;
        vertex(const glm::vec3& pos) : poss(pos), normal({0, 0, 0}), mat(nullptr), u(0), v(0) {}
        vertex(const glm::vec3& pos, const glm::vec3& n, material* m, float s, float t) : poss(pos), normal(n),
        mat(m), u(s), v(t) {}

        rtr::material* mat;
        glm::vec3 normal;

        glm::vec3 position() const { return poss; }
//        glm::vec3 normal() const { return norm; }

    };
}

//
// Created by Göksu Güvendiren on 2019-05-10.
//

#pragma once

#include <glm/glm.hpp>

namespace rtr
{
class ray
{
    glm::vec3   org;
    glm::vec3   dir;
    glm::vec3   invdir;

    bool        is_prim;

public:
    ray() : ray({0, 0, 0}, {1, 0, 0}) {};
    ray(glm::vec3 o, glm::vec3 d, bool isp = true) : org(o), dir(glm::normalize(d)), is_prim(isp)
    {
        invdir = glm::vec3(1.f, 1.f, 1.f) / dir;
    };

    glm::vec3 origin() const { return org; }
    glm::vec3 direction() const { return dir; }
    glm::vec3 inv_direction() const { return invdir; }

    bool is_primary() const { return is_prim; }
};
}
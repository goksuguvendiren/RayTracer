//
// Created by goksu on 6/7/19.
//

#pragma once

#include <glm/vec3.hpp>

namespace rtr
{
    class photon
    {
        float power;
        glm::vec3 origin;
        glm::vec3 direction;
    public:
    	photon(float p, const glm::vec3& o, const glm::vec3& d) : power(p), origin(o), direction(d) {}
    };
}
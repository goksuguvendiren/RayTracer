//
// Created by Göksu Güvendiren on 2019-05-09.
//

#pragma once

#include <glm/vec3.hpp>

namespace rtr
{
    class material
    {
    public:
        material(const glm::vec3& diff, const glm::vec3& amb, const glm::vec3& spec, const glm::vec3& ems, float p, float t) : diffuse(diff), ambient(amb), specular(spec), emissive(ems), exp(p), trans(t) {}
        glm::vec3 diffuse;
        glm::vec3 ambient;
        glm::vec3 specular;
        glm::vec3 emissive;

        float exp;
        float trans;
    };
}
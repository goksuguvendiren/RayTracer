//
// Created by Göksu Güvendiren on 2019-05-09.
//

#pragma once

#include <glm/vec3.hpp>
#include <vector>

namespace rtr
{
    class scene;
    class payload;
    class material
    {
    public:
        material() = default;
        material(const glm::vec3& diff, const glm::vec3& amb, const glm::vec3& spec, const glm::vec3& ems, float p, float t) : diffuse(diff), ambient(amb), specular(spec), emissive(ems), exp(p), trans(t), refr_index(1.0f)
        {
            if (trans > 0.f) refr_index = 1.5f;
        }
        
        glm::vec3 shade(const scene& scene, const payload& pld) const;
        
        glm::vec3 diffuse;
        glm::vec3 ambient;
        glm::vec3 specular;
        glm::vec3 emissive;
        
        float exp;
        float trans;
        float refr_index;
        
    private:

        
        
    };
}

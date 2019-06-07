//
// Created by Göksu Güvendiren on 2019-06-04.
//

#pragma once

#include <glm/glm.hpp>

namespace rtr
{
    class area_light
    {
    public:
        area_light(const glm::vec3& pos, const glm::vec3& col, const glm::vec3& u1, const glm::vec3& u2) : position(pos), color(col), side1(u1), side2(u2) {}
        
        glm::vec3 position;
        glm::vec3 color;
		glm::vec3 side1;
		glm::vec3 side2;

        glm::vec3 direction(const glm::vec3& hit_pos) const
        {
			// random sample the area light, return direction towards that.
            return -direc;
        }

        float distance(const glm::vec3& hit_pos) const
        {
			// return distance to random sample location
            return std::numeric_limits<float>::infinity();
        }

        float attenuate(const glm::vec3& hit_pos) const
        {
			// attenuate towards the random sampled position
            return 1.0f;
        }
    };
}


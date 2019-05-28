#pragma once

#include <array>
#include <glm/glm.hpp>
#include <array>
#include "vertex.hpp"
#include "ray.hpp"

namespace rtr
{
    class ray;
    namespace primitives
    {
        class face;
    }
    
    class aabb
    {
    public:
        aabb() = default;
        aabb(const std::array<rtr::vertex, 3>& faces)
        {
            min = glm::min(glm::min(std::get<0>(faces).position(), std::get<1>(faces).position()), std::get<2>(faces).position());
            max = glm::max(glm::max(std::get<0>(faces).position(), std::get<1>(faces).position()), std::get<2>(faces).position());
        }
        
        glm::vec3 min;
        glm::vec3 max;
        
        friend aabb combine(const aabb& left, const aabb& right)
        {
            aabb box;
            box.min.x = std::min(left.min.x, right.min.x);
            box.min.y = std::min(left.min.y, right.min.y);
            box.min.z = std::min(left.min.z, right.min.z);
            
            box.max.x = std::max(left.max.x, right.max.x);
            box.max.y = std::max(left.max.y, right.max.y);
            box.max.z = std::max(left.max.z, right.max.z);
            
            return box;
        }
        
        bool hit(const rtr::ray& ray) const
        {
            auto inv = ray.inv_direction();
            
            auto diff1 = min - ray.origin();
            auto diff2 = max - ray.origin();
            
            auto t  = diff1 * inv;
            auto tt = diff2 * inv;
            
            auto t1 = t[0];
            auto t2 = tt[0];
            
            float t_min = std::min(t1, t2);
            float t_max = std::max(t1, t2);
            
            for (int i = 1; i < 3; ++i)
            {
                auto& t1 = t[i];
                auto& t2 = tt[i];
                
                t_min = std::max(t_min, std::min(t1, t2));
                t_max = std::min(t_max, std::max(t1, t2));
            }
            
            return t_max >= std::max(.0f, t_min);
        }
    };
}

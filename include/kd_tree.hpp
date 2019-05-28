#pragma once

#include <optional>
#include <memory>
#include "aabb.hpp"
#include <memory>

namespace rtr
{
    namespace primitives
    {
        class mesh;
        class face;
    }
    
    class ray;
    struct payload;
    
    class kd_tree
    {
        std::unique_ptr<kd_tree> left;
        std::unique_ptr<kd_tree> right;
        
        aabb box;
        
    public:
        kd_tree() : left(nullptr), right(nullptr) {}
        kd_tree(const std::vector<rtr::primitives::face*>& faces);
        
        std::optional<rtr::payload> hit(const rtr::ray& ray) const;
        
        rtr::primitives::face* object;
    };
}

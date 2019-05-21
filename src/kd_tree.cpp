#include <vector>
#include <random>
#include <optional>
#include <iostream>
#include "kd_tree.hpp"
#include "primitives/mesh.hpp"
#include "aabb.hpp"
#include "ray.hpp"
#include "payload.hpp"

int get_random(int min, int max) // box inclusive
{
    static std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(min, max); // distribution in range [1, 6]
    
    return dist(rng);
}

float get_random(float min, float max) // box inclusive
{
    static std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<std::mt19937::result_type> dist(min, max); // distribution in range [1, 6]
    
    return dist(rng);
}

bool y_compare(rtr::primitives::face* f1, rtr::primitives::face* f2)
{
    return f1->box.min.y < f2->box.min.y;
}

bool z_compare(rtr::primitives::face* f1, rtr::primitives::face* f2)
{
    return f1->box.min.z < f2->box.min.z;
}

rtr::kd_tree::kd_tree(const std::vector<rtr::primitives::face*>& faces)
{
    auto axis = get_random(0, 2);
    
    std::vector<rtr::primitives::face*> sfaces = faces;
    switch(axis)
    {
        case 0:
            std::sort(sfaces.begin(), sfaces.end(), [](auto& f1, auto& f2) {return f1->box.min.x < f2->box.min.x;});
            break;
        case 1:
            std::sort(sfaces.begin(), sfaces.end(), [](auto& f1, auto& f2) {return f1->box.min.y < f2->box.min.y;});
            break;
        case 2:
            std::sort(sfaces.begin(), sfaces.end(), [](auto& f1, auto& f2) {return f1->box.min.z < f2->box.min.z;});
            break;
    }

    left = nullptr;
    right = nullptr;

    if (faces.size() == 1)
    {
        box = faces[0]->box;
        object = faces[0];
        
        return;
    }
    else if (faces.size() == 2)
    {
        left  = std::make_unique<kd_tree>(std::vector{faces[0]});
        right = std::make_unique<kd_tree>(std::vector{faces[1]});
        
        box = combine(left->box, right->box);
        return;
    }
    else
    {
        auto beginning = sfaces.begin();
        auto middling  = sfaces.begin() + (sfaces.size() / 2);
        auto ending    = sfaces.end();
        
        auto leftshapes  = std::vector<rtr::primitives::face*>(beginning, middling);
        auto rightshapes = std::vector<rtr::primitives::face*>(middling, ending);
        
        assert(faces.size() == (leftshapes.size() + rightshapes.size()));

        left  = std::make_unique<kd_tree>(leftshapes);
        right = std::make_unique<kd_tree>(rightshapes);
        
        box = combine(left->box, right->box);
    }
}

std::optional<rtr::payload> rtr::kd_tree::hit(const rtr::ray& ray) const
{
    if (!box.hit(ray)) return std::nullopt;
    if (!left && !right) // then a leaf box
    {
        auto hit = object->hit(ray);
        return hit;
    }
    
    auto l_rec = left  ? left->hit(ray)  : std::nullopt;
    auto r_rec = right ? right->hit(ray) : std::nullopt;
    
    if (!l_rec && !r_rec) return std::nullopt;
    
    payload ultimate;
    if (l_rec)
    {
        ultimate = *l_rec;
    }
    if (r_rec && r_rec->param < ultimate.param)
    {
        ultimate = *r_rec;
    }
    
    return ultimate;
}

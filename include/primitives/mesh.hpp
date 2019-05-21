//
// Created by Göksu Güvendiren on 2019-05-09.
//

#pragma once

#include <array>
#include <optional>
#include <vector>
#include <material.hpp>
#include "vertex.hpp"
#include "aabb.hpp"
#include "kd_tree.hpp"

namespace rtr
{
    struct payload;
    class ray;
    namespace primitives
    {
        struct face
        {
            face(std::array<rtr::vertex, 3> vert) : vertices(std::move(vert)), box(vertices)
            {
                set_normal();
            }
            std::array<rtr::vertex, 3> vertices;
            std::optional<rtr::payload> hit(const rtr::ray& ray) const;
            
            aabb box;

            void set_normal();
        };

        class mesh
        {
        public:
            mesh(std::vector<rtr::primitives::face> fcs, const std::string& nm) : faces(std::move(fcs)), name(nm)
            {
                auto begin = std::chrono::system_clock::now();
                std::vector<rtr::primitives::face*> face_ptrs;
                face_ptrs.reserve(faces.size());
                for (auto& f : faces)
                {
                    face_ptrs.push_back(&f);
                }
                tree = rtr::kd_tree(face_ptrs);
                
                auto end = std::chrono::system_clock::now();
                std::cout << "BVH construction took : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " millisecs.\n";
            }

            std::vector<rtr::material> materials;
            std::optional<rtr::payload> hit(const rtr::ray& ray) const;

            int id;
            std::string name;

        private:
            std::vector<rtr::primitives::face> faces;
            rtr::kd_tree tree;
        };
    }
}

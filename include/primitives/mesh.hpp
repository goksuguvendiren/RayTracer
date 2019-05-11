//
// Created by Göksu Güvendiren on 2019-05-09.
//

#pragma once

#include <array>
#include <optional>
#include <vector>
#include "vertex.hpp"

namespace rtr
{
    struct payload;
    class ray;
    namespace primitives
    {
        struct face
        {
            face() = default;
            std::array<rtr::vertex, 3> vertices;
            std::optional<rtr::payload> hit(const rtr::ray& ray) const;

            void set_normal();
        };

        class mesh
        {
        public:
            mesh(std::vector<rtr::primitives::face> fcs) : faces(std::move(fcs)) {}

//            std::vector<material> materials;
            std::optional<rtr::payload> hit(const rtr::ray& ray) const;

        private:
            std::vector<rtr::primitives::face> faces;
        };
    }
}
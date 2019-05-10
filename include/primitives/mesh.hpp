//
// Created by Göksu Güvendiren on 2019-05-09.
//

#pragma once

#include "vertex.hpp"

namespace rtr
{
    namespace primitives
    {
        class mesh
        {
        public:
            mesh(std::vector<rtr::vertex> verts) : vertices(std::move(verts)) {}

        private:
            std::vector<rtr::vertex> vertices;
        };
    }
}
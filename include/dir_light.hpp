//
// Created by Göksu Güvendiren on 2019-05-14.
//

#pragma once

#include <glm/glm.hpp>

namespace rtr
{
    class dir_light
    {
    public:
        dir_light(const glm::vec3& dir, const glm::vec3& col) : direc(glm::normalize(dir)), color(col) {}
        glm::vec3 direc;
        glm::vec3 color;

        glm::vec3 direction(const glm::vec3& hit_pos) const
        {
            return -direc;
        }

        float distance(const glm::vec3& hit_pos) const
        {
            return std::numeric_limits<float>::infinity();
        }

        float attenuate(const glm::vec3& hit_pos) const
        {
            return 1.0f;
        }

        std::vector<rtr::photon> distribute_photons(int num_photons)
        {
            std::vector<photon> photons;

            return photons;
        }

    };
}
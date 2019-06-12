//
// Created by Göksu Güvendiren on 2019-05-09.
//

#pragma once

#include <glm/vec3.hpp>
#include "photon.hpp"

namespace rtr
{
    class light
    {
    public:
        light(const glm::vec3& pos, const glm::vec3& col) : position(pos), color(col) {}
        glm::vec3 position;
        glm::vec3 color;

        glm::vec3 direction(const glm::vec3& hit_pos) const
        {
            return glm::normalize(position - hit_pos);
        }

        float distance(const glm::vec3& hit_pos) const
        {
            return glm::length(position - hit_pos);
        }

        float attenuate(const glm::vec3& hit_pos) const
        {
            auto distance = glm::length(position - hit_pos);
            float c_1 = 0.25;
            float c_2 = 0.1;
            float c_3 = 0.01;

            auto attenuation = 1.f / float(c_1 + c_2 * distance + c_3 * distance * distance);
            return std::min(1.f, attenuation);
        }

        std::vector<rtr::photon> distribute_photons(int num_photons)
        {
            std::vector<photon> photons;

            for (int i = 0; i < num_photons; ++i)
            {
                bool found_photon = false;
                float x, y, z;
                while(!found_photon)
                {
                    x = get_random_float(-1, 1);
                    y = get_random_float(-1, 1);
                    z = get_random_float(-1, 1);

                    found_photon = (x*x + y*y + z*z <= 1);  // rejection sampling the sphere
                }

                photons.emplace_back(power / float(num_photons), position, glm::normalize(glm::vec3{x, y, z}));
            }

            return photons;
        }

    private:
        float power;
    };
}
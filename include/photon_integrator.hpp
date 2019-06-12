//
// Created by goksu on 6/7/19.
//

#pragma once

#include <glm/vec3.hpp>

namespace rtr
{
class scene;
class photon_integrator
{
public:
    std::vector<glm::vec3> render(const rtr::scene& scene);
    
private:
    int num_photons;
};
}

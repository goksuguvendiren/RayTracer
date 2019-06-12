//
// Created by goksu on 6/7/19.
//

#include <vector>
#include <photon_integrator.hpp>
#include "photon.hpp"
#include "scene.hpp"
#include "material.hpp"

void trace_photons(const rtr::scene& scene, const rtr::photon& photon, std::vector<rtr::photon>& hit_photons)
{
//    auto photon_ray = rtr::ray(photon.origin, photon.direction, 0, false);
//    auto hit = scene.photon_trace(photon_ray);
//
//    if (!hit) return;
//
//    // there's a hit, save the photon in the photon map
//    hit_photons.emplace_back(photon_ray.power, hit->position, photon_ray.direction);
//
//    // decide the photon's fate with russian roulette
//    PathType decision = hit->material.russian_roulette();
//    if(decision == PathType::Absorbed) return;
//
//    auto direction = hit->material.sample();
//
////  TODO: update the power now!
//    rtr::photon new_photon(photon.power, hit->position, direction);
//    trace_photons(scene, new_photon, hit_photons);
}

std::vector<glm::vec3> rtr::photon_integrator::render(const rtr::scene& scene)
{
    // Phase 1 for photon mapping:
    // Iterate through all the lights, and shoot photons in random directions.
    std::vector<rtr::photon> emitted_photons;
    scene.for_each_light([this, &emitted_photons](auto light)
    {
        auto new_photons = light.distribute_photons(num_photons);
        emitted_photons.insert(emitted_photons.end(), std::make_move_iterator(new_photons.begin()), std::make_move_iterator(new_photons.end()));
    });
    
    // actual photon mapping. Trace all the photons through their paths and
    // save their positions and directions.
    std::vector<rtr::photon> hit_photons;
    for (auto& photon : emitted_photons)
    {
        trace_photons(scene, photon, hit_photons);
    }

    // kd_tree photon_map(hit_photons);
}

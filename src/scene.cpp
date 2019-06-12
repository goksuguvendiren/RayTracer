#include <memory>
#include <scene_io.h>
#include <vertex.hpp>
#include "scene.hpp"
#include "primitives/sphere.hpp"
#include "primitives/mesh.hpp"
#include "utils.hpp"

static glm::vec3 reflect(const glm::vec3& light, const glm::vec3& normal)
{
    return glm::normalize(2 * glm::dot(normal, light) * normal - light);
}

glm::vec3 refract(const glm::vec3 &I, const glm::vec3 &N, const float &ior)
{
    float cosi = std::clamp(glm::dot(I, N), -1.f, 1.f);
    float etai = 1, etat = ior;
    glm::vec3 n = N;
    if (cosi < 0) { cosi = -cosi; } else { std::swap(etai, etat); n = -N; }
    float eta = etai / etat;
    float k = 1 - eta * eta * (1 - cosi * cosi);
    return k < 0 ? glm::vec3{0, 0, 0} : eta * I + (eta * cosi - sqrtf(k)) * n;
}

std::optional<rtr::payload> rtr::scene::hit(const rtr::ray& ray) const
{
    std::optional<rtr::payload> min_hit = std::nullopt;

    for (auto& sphere : spheres)
    {
        auto hit = sphere.hit(ray);
        if (!hit) continue;
        if (!min_hit || hit->param < min_hit->param)
        {
            min_hit = *hit;
        }
    }

    for (auto& mesh : meshes)
    {
        auto hit = mesh.hit(ray);
        if (!hit) continue;
        if (std::isnan(hit->param)) throw std::runtime_error("param is nan in scene::hit()!");

        if (!min_hit || hit->param < min_hit->param)
        {
            if (std::isnan(hit->param)) throw std::runtime_error("param is nan in scene::hit() 2!");
            min_hit = *hit;
        }
    }

    if (min_hit && std::isnan(min_hit->param))throw std::runtime_error("param is nan in scene::hit() 3!");

    return min_hit;
}



glm::vec3 rtr::scene::trace(const rtr::ray& ray) const
{
    auto color = glm::vec3{0.f, 0.f, 0.f};
    std::optional<rtr::payload> pld = hit(ray);
    
    if (!pld) return color;
    if (pld->ray.rec_depth >= max_recursion_depth) return pld->material->shade(*this, *pld);
    
//    return glm::vec3(1.0f);
//    return (pld->material->diffuse);
    
    auto sample_direction = sample_hemisphere(pld->hit_normal);
    auto reflection_ray = rtr::ray(pld->hit_pos + (sample_direction * 1e-3f), sample_direction, pld->ray.rec_depth + 1, false);
    
    return  pld->material->shade(*this, *pld) * trace(reflection_ray);
    
//
//    color = pld->material->shade(*this, *pld);
//
//    // Reflection :
//    if (pld->material->specular.x > 0.f || pld->material->specular.y > 0.f || pld->material->specular.z > 0.f)
//    {
//        auto reflection_direction = reflect(ray.direction(), pld->hit_normal);
//        rtr::ray reflected_ray(pld->hit_pos + (reflection_direction * 1e-3f), reflection_direction, pld->ray.rec_depth + 1, false);
//        color += pld->material->specular * trace(reflected_ray);
//    }
//
//    // Refraction
//    if (pld->material->trans > 0.f)
//    {
//        auto eta_1 = 1.f;
//        auto eta_2 = 1.5f;
//
//        auto refraction_direction = refract(ray.direction(), pld->hit_normal, eta_2 / eta_1);
//        if (glm::length(refraction_direction) > 0.1)
//        {
//            rtr::ray refracted_ray(pld->hit_pos + (refraction_direction * 1e-3f), refraction_direction, pld->ray.rec_depth + 1, false);
//            color += pld->material->trans * trace(refracted_ray);
//        }
//    }

    return color;
}

// Do it recursively
glm::vec3 rtr::scene::shadow_trace(const rtr::ray& ray, float light_distance) const
{
    auto shadow = glm::vec3{1.f, 1.f, 1.f};
    std::optional<rtr::payload> pld = hit(ray);
    
    if (!pld) return shadow;
    if (pld && (pld->param < light_distance)) // some base case checks to terminate
    {
        auto& diffuse = pld->material->diffuse;
        auto normalized_diffuse = diffuse / std::max(std::max(diffuse.x, diffuse.y), diffuse.z);
        return shadow * normalized_diffuse * pld->material->trans;
    }
    
    auto hit_position = pld->hit_pos + ray.direction() * 1e-4f;
    rtr::ray shadow_ray = rtr::ray(hit_position, ray.direction(), ray.rec_depth, false);
    
    return shadow * shadow_trace(shadow_ray, light_distance - glm::length(pld->hit_pos - ray.origin()));
}

glm::vec3 rtr::scene::photon_trace(const rtr::ray& photon_ray) const
{
    
}

void rtr::scene::print() const
{
    std::cerr << "Camera : \n";
    std::cerr << "\t Pos : " << camera.center() << '\n';
    std::cerr << "Meshes : \n";
    int i = 0;
    for (auto& m : meshes)
    {
        std::cerr << "\t\t Mesh " << i++ << " : \n";
        std::cerr << "\t\t Material " << m.materials[0].diffuse << " : \n";
        std::cerr << "\t\t " << m.materials[0].ambient << " : \n";
        std::cerr << "\t\t " << m.materials[0].specular << " : \n";
        for (auto& face : m.faces)
        {
            std::cerr << "\t\t\t Position_a : " << face.vertices[0].position() << '\n';
            std::cerr << "\t\t\t Position_b : " << face.vertices[1].position() << '\n';
            std::cerr << "\t\t\t Position_c : " << face.vertices[2].position() << '\n';
        }
    }
    std::cerr << "Spheres : \n";
    i = 0;
    for (auto& s : spheres)
    {
        std::cerr << "\t\t Sphere " << i++ << " : \n";
        std::cerr << "\t\t Material " << s.materials[0].diffuse << " : \n";
        std::cerr << "\t\t " << s.materials[0].ambient << " : \n";
        std::cerr << "\t\t " << s.materials[0].specular << " : \n";
//        for (auto& face : m.faces)
        {
            std::cerr << "\t\t\t Center : " << s.origin << '\n';
            std::cerr << "\t\t\t Radius : " << s.radius << '\n';
        }
    }
}

//
// Created by Göksu Güvendiren on 2019-05-10.
//

#include <optional>
#include <iostream>
#include <payload.hpp>
#include <primitives/mesh.hpp>
#include <ray.hpp>


inline float determinant(const glm::vec3& col1, const glm::vec3& col2, const glm::vec3& col3)
{
    return col1.x * (col2.y * col3.z - col2.z * col3.y) -
           col2.x * (col1.y * col3.z - col1.z * col3.y) +
           col3.x * (col1.y * col2.z - col1.z * col2.y);

}

inline bool is_back_face(const glm::vec3& surface_normal, const glm::vec3& direction)
{
    return glm::dot(surface_normal, direction) < 0;
}

std::optional<rtr::payload> rtr::primitives::face::hit(const rtr::ray &ray) const
{
    const auto& a = vertices[0];
    const auto& b = vertices[1];
    const auto& c = vertices[2];
    const auto& surface_normal = vertices[0].normal;

    glm::vec3 col1 = a.position() - b.position();
    glm::vec3 col2 = a.position() - c.position();
    glm::vec3 col3 = ray.direction();
    glm::vec3 col4 = a.position() - ray.origin();

    auto epsilon = -1e-4;
    auto detA  = determinant(col1, col2, col3);
    if (detA == 0) return std::nullopt;

    auto oneOverDetA = 1 / detA;

    auto beta  = determinant(col4, col2, col3) * oneOverDetA;
    auto gamma = determinant(col1, col4, col3) * oneOverDetA;
    auto param = determinant(col1, col2, col4) * oneOverDetA;
    auto alpha = 1 - beta - gamma;

    if (alpha < epsilon || gamma < epsilon|| beta < epsilon || param < epsilon)
    {
        return std::nullopt;
    }
    if (ray.is_primary() && !is_back_face(surface_normal, ray.direction()))
    {
        return std::nullopt;
    }

    auto point = ray.origin() + param * ray.direction();
    glm::vec3 normal = glm::normalize(alpha * surface_normal + beta * surface_normal + gamma * surface_normal);

    return rtr::payload{normal, point, ray, param};
}

void rtr::primitives::face::set_normal()
{
    auto normal = glm::normalize(glm::cross(vertices[1].position() - vertices[0].position(),
                                            vertices[2].position() - vertices[0].position()));

    for (auto& vert : vertices) vert.normal = normal;
}

std::optional<rtr::payload> rtr::primitives::mesh::hit(const rtr::ray &ray) const
{
    std::optional<rtr::payload> min_hit;
    for (auto& face : faces)
    {
        auto hit = face.hit(ray);
        if (!hit) continue;
        if (!min_hit || hit->param < min_hit->param)
        {
            min_hit = *hit;
        }
    }

    if (min_hit) min_hit->object = this;
    return min_hit;
}
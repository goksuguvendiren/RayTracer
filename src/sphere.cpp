//
// Created by Göksu Güvendiren on 2019-05-10.
//

#include <iostream>
#include "payload.hpp"
#include "primitives/sphere.hpp"

constexpr double pi = 3.14159265359;

static glm::vec2 get_lat_long(const glm::vec3& point, const glm::vec3& center)
{
    auto N = glm::normalize(point - center);
    auto u = std::atan2(N.x, N.z) / (2 * pi) + 0.5;
    auto v = N.y * 0.5 + 0.5;
    
    return {u, v};
}

std::optional<rtr::payload> rtr::primitives::sphere::hit(const rtr::ray& ray) const
{
    auto eminc = ray.origin() - origin;

    auto A = 1;
    auto B = 2.0f * glm::dot(ray.direction(), eminc);
    auto C = glm::dot(eminc, eminc) - radius * radius;

    auto delta = B * B - 4 * A * C;

    if (delta < 0) return std::nullopt;

    auto param = (- B - std::sqrt(delta));

    if (param < 0)
    {
        param = (- B + std::sqrt(delta));

        if (param < 0)
            return std::nullopt;
    }

    param *= 0.5f;

    auto hit_point      = ray.origin() + param * ray.direction();
    auto surface_normal = glm::normalize(hit_point - origin);

    if (std::isinf(param) || std::isnan(param))
    {
        return std::nullopt;
    }
    
    auto uv = get_lat_long(hit_point, origin);
    
    auto pld = rtr::payload{surface_normal, hit_point, ray, param, &materials.front(), uv, id};

    return pld;
}

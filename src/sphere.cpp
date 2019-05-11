//
// Created by Göksu Güvendiren on 2019-05-10.
//

#include <iostream>
#include "payload.hpp"
#include "primitives/sphere.hpp"

std::optional<rtr::payload> rtr::primitives::sphere::hit(const rtr::ray& ray) const
{
    auto eminc = ray.origin() - origin;

    auto A = 1.f;//glm::dot(ray.direction(), ray.direction());
    auto B = 2.0f * glm::dot(ray.direction(), eminc);
    auto C = glm::dot(eminc, eminc) - radius * radius;

    auto delta = B * B - 4 * A * C;

    if (delta < 1e-4) return std::nullopt;

    auto param = (- B - std::sqrt(delta)) / (2.0f * A);

    if (param < 0)
    {
        param = (- B + std::sqrt(delta)) / (2.0f * A);

        if (param < 0)
            return std::nullopt;
    }

    auto model_point    = ray.origin() + param * ray.direction();
    auto surface_normal = glm::normalize(model_point - origin);
    auto hit_point      = model_point;//glm::vec3(transformation_matrix * glm::vec4(model_point, 1));

    if (std::isinf(param) || std::isnan(param))
    {
        return std::nullopt;
    }

    return payload{surface_normal, hit_point, ray, param, this};
}

//
// Created by Göksu Güvendiren on 2019-05-09.
//

#include <optional>
#include <renderer.hpp>
#include "scene.hpp"
#include "ray.hpp"
#include "utils.hpp"

static glm::vec3 reflect(const glm::vec3& light, const glm::vec3& normal)
{
    return glm::normalize(2 * glm::dot(normal, light) * normal - light);
}

static glm::vec3 refract(const glm::vec3& light, const glm::vec3& normal, float eta_1, float eta_2)
{
    assert(glm::length(light) < 1.01f && glm::length(normal) < 1.01f);

    auto cos_theta_1 = glm::dot(light, normal);
    auto sin_theta_1 = std::sin(std::acos(cos_theta_1));
    auto sin_theta_2 = sin_theta_1 * (eta_1 / eta_2);

    if (sin_theta_2 >= 1) return reflect(light, normal);

    auto cos_theta_2 = std::cos(std::asin(sin_theta_2));

    auto Q = cos_theta_1 * normal;
    auto M = (eta_1 / eta_2) * (Q - light);

    auto P = - cos_theta_2 * normal;

    return M + P;
}

std::optional<rtr::payload> shadow_trace(const rtr::scene& scene, const rtr::ray& ray)
{
    auto color = glm::vec3{0.f, 0.f, 0.f};
    std::optional<rtr::payload> hit = scene.hit(ray);

    return hit;
}

glm::vec3 shade(const rtr::scene& scene, const rtr::payload& payload)
{
    auto& mat = payload.material;

    auto ambient = (1 - mat->trans) * mat->ambient * mat->diffuse;
    glm::vec3 color = ambient;

    for (auto& light : scene.lights())
    {
        float epsilon = 1e-5;
        auto hit_position = payload.hit_pos + payload.hit_normal * epsilon;
        rtr::ray shadow_ray = rtr::ray(hit_position, light.position - hit_position, false);
        auto in_shadow = shadow_trace(scene, shadow_ray);

        if(in_shadow && (in_shadow->param < glm::length(light.position - hit_position))) continue; //point is in shadow

        auto diffuse = (1 - mat->trans) * mat->diffuse * std::max(glm::dot(payload.hit_normal, light.direction(payload.hit_pos)), 0.0f);
        auto specular = mat->specular * std::pow(std::max(glm::dot(-payload.ray.direction(), reflect(light.direction(payload.hit_pos), payload.hit_normal)), 0.0f), mat->exp);

        color += (diffuse + specular);
    }

    return color;
}

glm::vec3 rtr::renderer::trace(const rtr::scene& scene, const rtr::ray& ray, int rec_depth, int max_rec_depth)
{
    auto color = glm::vec3{0.f, 0.f, 0.f};  
    std::optional<rtr::payload> hit = scene.hit(ray);

    if (!hit) return color;

    color = shade(scene, *hit);
    if (rec_depth >= max_rec_depth) return color;

    // Reflection :
    if (hit->material->specular.x > 0.f || hit->material->specular.y > 0.f || hit->material->specular.z > 0.f)
    {
        auto reflection_direction = reflect(-hit->ray.direction(), hit->hit_normal);
        rtr::ray reflected_ray(hit->hit_pos + (reflection_direction * 1e-3f), reflection_direction, false);

        color += hit->material->specular * trace(scene, reflected_ray, rec_depth + 1, max_rec_depth);
    }

    // Refraction
    if (hit->material->trans > 0.f)
    {
        bool entering = glm::dot(hit->ray.direction(), hit->hit_normal) < 0.0f;
        auto normal = hit->hit_normal;
        float eta_1, eta_2;
        if (entering)
        {
            eta_1 = 1.f;//refr_indices.top();
            eta_2 = 1.5f;//hit->material->refractive_index;
//            refr_indices.push(eta_2);
        }
        else // exiting
        {
            eta_1 = 1.5f;//refr_indices.top();
//            refr_indices.pop();
            eta_2 = 1.0f;//refr_indices.top();
            normal = -normal;
//            std::cout << eta_1 << ", " << eta_2 << '\n';
        }

        auto refraction_direction = refract(-hit->ray.direction(), normal, eta_1, eta_2); // current_index, next_index
//        std::cout << hit->ray.direction() << " - " << refraction_direction << '\n';
        rtr::ray refracted_ray(hit->hit_pos + (refraction_direction * 1e-3f), refraction_direction, false);

        color += hit->material->trans * trace(scene, refracted_ray, rec_depth + 1, max_rec_depth);
    }

    return color;
}

void rtr::renderer::render(const rtr::scene &scene)
{
    const auto& camera = scene.get_camera();
    rtr::image_plane plane(camera, width, height);

    auto right = (1 / float(width)) * plane.right;
    auto below = -(1 / float(height)) * plane.up;

    auto pix_center = plane.top_left_position();
    pix_center -= right * 0.5f;
    pix_center -= below * 0.5f;

    auto row_begin = pix_center;
    for (int i = 0; i < height; ++i)
    {
        row_begin += below;
        pix_center = row_begin;
        for (int j = 0; j < width; ++j)
        {
            pix_center += right;
            auto pixel_position = get_pixel_pos(pix_center, right, below);
            //create the ray
            auto ray = rtr::ray(camera.position(), pixel_position - camera.position(), true);

            auto color = trace(scene, ray, 0, 4);
            frame_buffer[i * width + j] = color;
        }
    }
    
    cv::Mat image(width, height, CV_32FC3, frame_buffer.data());
    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
    cv::imshow("window", image);
    cv::waitKey(0);
}

glm::vec3 rtr::renderer::get_pixel_pos(const glm::vec3& pix_center, const glm::vec3& right, const glm::vec3& below)
{
    auto x_offset = 0.5f;
    auto y_offset = 0.5f;
    // auto random_x = utils::sample_float() - 0.5f;
    // auto random_y = utils::sample_float() - 0.5f;

    auto sample = pix_center + right * x_offset + below * y_offset;
    return sample;
}
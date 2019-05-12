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
    return 2 * glm::dot(normal, light) * normal - light;
}

float shadow_trace(const rtr::scene& scene, const rtr::ray& ray)
{
    auto color = glm::vec3{0.f, 0.f, 0.f};
    std::optional<rtr::payload> hit = scene.hit(ray);

    if (!hit) return 1.f;

    return 0.f;
}

glm::vec3 shade(const rtr::scene& scene, const rtr::payload& payload)
{
    auto& mat = payload.material;

    auto ambient = (1 - mat->trans) * mat->ambient * mat->diffuse;
    glm::vec3 color = ambient;

    for (auto& light : scene.lights())
    {
        float epsilon = 1e-4;
        rtr::ray shadow_ray = rtr::ray(payload.hit_pos + (payload.hit_normal * epsilon), light.position - payload.hit_pos, false);
        auto shadow_term = shadow_trace(scene, shadow_ray);

        auto diffuse = (1 - mat->trans) * mat->diffuse * std::max(glm::dot(payload.hit_normal, light.direction(payload.hit_pos)), 0.0f);
        auto specular = mat->specular * std::pow(std::max(glm::dot(-payload.ray.direction(), reflect(light.direction(payload.hit_pos), payload.hit_normal)), 0.0f), mat->exp);

        color += shadow_term * (diffuse + specular);
    }

    return color;
}

glm::vec3 trace(const rtr::scene& scene, const rtr::ray& ray, int rec_depth, int max_rec_depth)
{
    auto color = glm::vec3{0.f, 0.f, 0.f};  
    std::optional<rtr::payload> hit = scene.hit(ray);

    if (!hit) return color;

    return shade(scene, *hit);
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
//
// Created by Göksu Güvendiren on 2019-05-09.
//

#include <optional>
#include <renderer.hpp>
#include "scene.hpp"
#include "ray.hpp"
#include "utils.hpp"

glm::vec3 trace(const rtr::scene& scene, const rtr::ray& ray, int rec_depth, int max_rec_depth)
{
    auto color = glm::vec3{0.f, 0.f, 0.f};  
    std::optional<rtr::payload> hit = scene.hit(ray);

    if (!hit) return color;

    if (hit->material) return hit->material->diffuse;
    return glm::vec3(1.f, 1.f, 1.f);
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
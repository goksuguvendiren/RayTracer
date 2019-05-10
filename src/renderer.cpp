//
// Created by Göksu Güvendiren on 2019-05-09.
//

#include <renderer.hpp>
#include "scene.hpp"
#include "ray.hpp"

void rtr::renderer::render(const rtr::scene &scene)
{
    const auto& camera = scene.get_camera();
    rtr::image_plane plane(camera, width, height);

    auto right = (1 / float(width)) * camera.right();
    auto below = -(1 / float(height)) * camera.up();

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
            auto ray = rtr::ray(camera.position(), pixel_position - camera.position(), true);
            //create the ray
        }
    }
    
    cv::Mat image(width, height, CV_32FC3, frame_buffer.data());
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
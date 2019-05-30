//
// Created by Göksu Güvendiren on 2019-05-09.
//

#pragma once

#include <glm/glm.hpp>

#include "utils.hpp"

namespace rtr
{
    class camera
    {
    public:
        camera() = default;
        camera(const glm::vec3& pos, const glm::vec3& view, const glm::vec3& up, float foc, float fov, float ipd = 1.0f, bool pin = true) :
                eye_pos(pos), view_dir(glm::normalize(view)), up_dir(glm::normalize(up)), focal_dist(foc), vert_fov(fov), pinhole(pin), image_plane_dist(ipd)
        {
            if (pin)
            {
                // if the camera model is a pinhole camera, then the distance to the image plane is
                // the focal distance
                image_plane_dist = focal_dist;
            }
            else
            {
                //TODO : make this variable
                lens_width = 0.1;
            }

            right_dir = glm::normalize(glm::cross(view_dir, up_dir));
            up_dir    = glm::normalize(glm::cross(right_dir, view_dir));
        }

        auto up() const { return up_dir; }
        auto right() const { return right_dir; }
        auto view() const { return view_dir; }

        auto center() const { return eye_pos; }
        glm::vec3 position() const { return pinhole ? eye_pos : point_sample_lens(); }

        auto focal_distance() const { return focal_dist; }
        auto image_plane_distance() const { return image_plane_dist; }
        auto fov() const { return vert_fov; }

        auto is_pinhole() const { return pinhole; }

        bool pinhole;
        float image_plane_dist;
        float lens_width;
        float focal_dist;
        
    private:
        glm::vec3 eye_pos;
        glm::vec3 view_dir;
        glm::vec3 up_dir;
        glm::vec3 right_dir; 


        float vert_fov;

        glm::vec3 point_sample_lens() const
        {
            auto u = get_random_float(-1, 1);
            auto v = get_random_float(-1, 1);

            return eye_pos + u * right_dir * lens_width / 2.f + v * up_dir * lens_width / 2.f;
        }
    };

    class image_plane
    {
    public:
        image_plane(const camera& cam, unsigned int w, unsigned int h)
        {
            float scale = cam.image_plane_distance();
            up = scale * std::tan(cam.fov() / 2.f) * cam.up();

            float aspect_ratio = w / float(h);
            auto horizontal_fov = cam.fov() * aspect_ratio;
            right = scale * std::tan(horizontal_fov / 2.f) * cam.right();

            glm::vec3 center;
            if (cam.is_pinhole())
            {
                center = cam.center() + scale * cam.view();
            }
            else
            {
                // The camera has a lens, in this case the image plane is behind the camera
                center = cam.center() - scale * cam.view();
            }
            top_left = center + up - right; // CAN BE DIFFERENT! Check!

            up *= 2;
            right *= 2;
        }

        glm::vec3 right;
        glm::vec3 up;
        glm::vec3 top_left_position() const { return top_left; }
    private:
        glm::vec3 top_left;
    };
}

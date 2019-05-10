//
// Created by Göksu Güvendiren on 2019-05-09.
//

#pragma once

#include <glm/glm.hpp>

namespace rtr
{
    class camera
    {
    public:
        camera() = default;
        camera(const glm::vec3& pos, const glm::vec3& view, const glm::vec3& up, float foc, float fov) : 
                eye_pos(pos), view_dir(glm::normalize(view)), up_dir(glm::normalize(up)), focal_dist(foc), vert_fov(fov) 
        {
            right_dir = glm::normalize(glm::cross(view_dir, up_dir));
            up_dir = glm::normalize(glm::cross(right_dir, view_dir));
        }

        auto up() const { return up_dir; }
        auto right() const { return right_dir; }
        auto position() const { return eye_pos; }

        glm::vec3 eye_pos;
        glm::vec3 view_dir;
        glm::vec3 up_dir;
        glm::vec3 right_dir; 

        float focal_dist;
        float vert_fov;
    };

    class image_plane
    {
    public:
        image_plane(const camera& cam, unsigned int w, unsigned int h)
        {
            float scale = cam.focal_dist;
            auto center = cam.eye_pos + scale * cam.view_dir;
            auto up = scale * std::tan(glm::radians(cam.vert_fov / 2.f)) * cam.up_dir;

            float aspect_ratio = w / float(h);
            auto horizontal_fov = cam.vert_fov * aspect_ratio;
            auto right = scale * std::tan(glm::radians(horizontal_fov / 2.f)) * cam.right_dir;

            top_left = center + up - right;
        }

        glm::vec3 top_left_position() const { return top_left; }
    private:
        glm::vec3 top_left;
    };

}
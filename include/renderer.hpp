//
// Created by Göksu Güvendiren on 2019-05-09.
//

#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <stack>

namespace rtr
{
    class scene;
    class ray;

    class renderer
    {
    public:
        renderer(unsigned int w, unsigned int h) : width(w), height(h)
        {
            frame_buffer.resize(width * height);
            refr_indices.push(1.f);
        }

        std::vector<glm::vec3> render(const rtr::scene& scene);
        glm::vec3 get_pixel_pos(const glm::vec3& top_left, const glm::vec3& right, const glm::vec3& below);

    private:
        std::vector<glm::vec3> frame_buffer;
        unsigned int width;
        unsigned int height;

        std::stack<float> refr_indices;

        glm::vec3 trace(const rtr::scene& scene, const rtr::ray& ray, int rec_depth, int max_rec_depth);
    };
}

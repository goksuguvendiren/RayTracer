//
// Created by Göksu Güvendiren on 2019-05-09.
//

#pragma once
#include <vector>
#include <glm/glm.hpp>

namespace rtr
{
    class scene;

    class renderer
    {
    public:
        renderer(unsigned int w, unsigned int h) : width(w), height(h)
        {
            frame_buffer.resize(width * height);
        }

        void render(const rtr::scene& scene);
        glm::vec3 get_pixel_pos(const glm::vec3& top_left, const glm::vec3& right, const glm::vec3& below);

    private:
        std::vector<glm::vec3> frame_buffer;
        unsigned int width;
        unsigned int height;
    };
}
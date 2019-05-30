//
// Created by Göksu Güvendiren on 2019-05-09.
//

#pragma once
#include <vector>
#include <random>
#include <glm/glm.hpp>
#include <stack>
#include "camera.hpp"
#include "utils.hpp"

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
        void render_line(const rtr::scene &scene, const glm::vec3& row_begin, int i);
        glm::vec3 render_pixel(const rtr::scene& scene, const camera& camera, const glm::vec3& pix_center,
                               const rtr::image_plane& plane, const glm::vec3& right, const glm::vec3& below);

        template <int sq_sample_pp>
        glm::vec3 get_pixel_pos(const glm::vec3& top_left, const rtr::image_plane& plane, const rtr::camera& camera,
                                const glm::vec3& right, const glm::vec3& below, int u, int v, std::bool_constant<true>);

        template <int sq_sample_pp>
        glm::vec3 get_pixel_pos(const glm::vec3& top_left, const rtr::image_plane& plane, const rtr::camera& camera,
                                const glm::vec3& right, const glm::vec3& below, int u, int v, std::bool_constant<false>);

    private:
        std::vector<glm::vec3> frame_buffer;
        unsigned int width;
        unsigned int height;

        std::stack<float> refr_indices;

        glm::vec3 trace(const rtr::scene& scene, const rtr::ray& ray, int rec_depth, int max_rec_depth);
    };

    template <int sq_sample_pp>
    glm::vec3 rtr::renderer::get_pixel_pos(const glm::vec3& top_left, const rtr::image_plane& plane, const rtr::camera& camera,
                                           const glm::vec3& right, const glm::vec3& below, int u, int v, std::bool_constant<true>)
    {
        auto random_u = get_random_float();
        auto random_v = get_random_float();

        auto x_offset = (random_u * (1.f / sq_sample_pp)) + ((float)u / sq_sample_pp);
        auto y_offset = (random_v * (1.f / sq_sample_pp)) + ((float)v / sq_sample_pp);

        // stratified sampling of the pixel. -> random p location on the film plane
        auto sample = top_left + right * x_offset + below * y_offset;

        auto line_through_lens_center = glm::normalize(camera.center() - sample); // direction of the ray from sample through the center of the lens
        auto point_on_plane = camera.center() + camera.focal_distance() * camera.view();
        
        auto plane_normal = camera.view();
        float nom = glm::dot(plane_normal, point_on_plane - sample);
        float denom = glm::dot(plane_normal, line_through_lens_center);
        
        auto t = nom / denom;
        
        return sample + t * line_through_lens_center; // returns the q
    }

    template <int sq_sample_pp>
    glm::vec3 rtr::renderer::get_pixel_pos(const glm::vec3& top_left, const rtr::image_plane& plane, const rtr::camera& camera,
                                           const glm::vec3& right, const glm::vec3& below, int u, int v, std::bool_constant<false>)
    {
        if constexpr (sq_sample_pp == 1)
        {
            return top_left + right * 0.5f + below * 0.5f;
        }

        auto random_u = get_random_float();
        auto random_v = get_random_float();

        auto x_offset = (random_u * (1.f / sq_sample_pp)) + ((float)u / sq_sample_pp);
        auto y_offset = (random_v * (1.f / sq_sample_pp)) + ((float)v / sq_sample_pp);

        auto sample = top_left + right * x_offset + below * y_offset;
        return sample;
    }

}

//
// Created by Göksu Güvendiren on 2019-05-09.
//

#pragma once
#include <vector>
#include <random>
#include <thread>
#include <glm/glm.hpp>
#include <stack>
#include "camera.hpp"
#include "utils.hpp"
#include "scene.hpp"
#include "ray.hpp"

namespace rtr
{
    // class scene;
    // class ray;

    template <typename Integrator>
    class renderer
    {
    public:
        renderer(unsigned int w, unsigned int h) : width(w), height(h)
        {
            frame_buffer.resize(width * height);
            refr_indices.push(1.f);
        }

        std::vector<glm::vec3> render(const rtr::scene& scene) { return integrator.render(scene); }
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

        Integrator integrator;

        std::stack<float> refr_indices;
    };

    template <typename Integrator>
    template <int sq_sample_pp>
    glm::vec3 renderer<Integrator>::get_pixel_pos(const glm::vec3& top_left, const rtr::image_plane& plane, const rtr::camera& camera,
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

    template <typename Integrator>
    template <int sq_sample_pp>
    glm::vec3 renderer<Integrator>::get_pixel_pos(const glm::vec3& top_left, const rtr::image_plane& plane, const rtr::camera& camera,
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

    inline void CallBackFunc(int event, int x, int y, int flags, void* userdata)
    {
        if  ( event == cv::EVENT_LBUTTONDOWN )
        {
            std::cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << std::endl;
        }
    }

    inline void UpdateProgress(float progress)
    {
        int barWidth = 70;
        
        std::cout << "[";
        int pos = barWidth * progress;
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(progress * 100.0) << " %\r";
        std::cout.flush();
    };

    template <typename Integrator>
    glm::vec3 renderer<Integrator>::render_pixel(const rtr::scene& scene, const rtr::camera& camera, const glm::vec3& pix_center,
                                          const rtr::image_plane& plane, const glm::vec3& right, const glm::vec3& below)
    {
        // supersampling - jittered stratified
        constexpr int sq_sample_pp = 2;
        auto is_lens = std::bool_constant<true>();

        glm::vec3 color = {0, 0, 0};
        
        for (int k = 0; k < sq_sample_pp; ++k)
        {
            for (int m = 0; m < sq_sample_pp; ++m)
            {
                auto camera_pos = camera.position(); // random sample on the lens if not pinhole
    //            std::cout << camera_pos << '\n';
                auto sub_pix_position = get_pixel_pos<sq_sample_pp>(pix_center, plane, camera, right, below, k, m, is_lens); // get the q
                auto ray = rtr::ray(camera_pos, sub_pix_position - camera_pos, 0, true);

                color += scene.trace(ray);
                
                return color;
            }
        }
        
        return color / float(sq_sample_pp * sq_sample_pp);
    }

    template <typename Integrator>
    void renderer<Integrator>::render_line(const rtr::scene &scene, const glm::vec3& row_begin, int i)
    {
        const auto& camera = scene.get_camera();
        rtr::image_plane plane(camera, width, height);
        
        auto right =  (1 / float(width))  * plane.right;
        auto below = -(1 / float(height)) * plane.up;
        
        glm::vec3 pix_center = row_begin;
        for (int j = 0; j < width; ++j)
        {
            pix_center += right;
            auto color = render_pixel(scene, camera, pix_center, plane, right, below);

            frame_buffer[i * width + j] = color;
        }
    }

    // template <typename Integrator>
    // std::vector<glm::vec3> renderer<Integrator>::render(const rtr::scene &scene)
    // {
    //     // Phase 1 for photon mapping:
    //     const auto& camera = scene.get_camera();
    //     rtr::image_plane plane(camera, width, height);

    //     auto right = (1 / float(width)) * plane.right;
    //     auto below = -(1 / float(height)) * plane.up;

    //     auto pix_center = plane.top_left_position();

    //     cv::namedWindow("window");
    //     cv::setMouseCallback("window", CallBackFunc, NULL);

    //     int number_of_threads = std::thread::hardware_concurrency();
    //     std::cerr << "Threads enabled! Running " << number_of_threads << " threads!\n";
    //     std::vector<std::thread> threads;
    //     int n = 0;
    //     for (int i = 0; i < number_of_threads; ++i)
    //     {
    //         threads.push_back(std::thread([i, &scene, pix_center, this, &below, &n, number_of_threads]
    //         {
    //         //            std::cerr << "thread " << i << " started!\n";
    //             for (int j = i; j < height; j += number_of_threads)
    //             {
    //                auto row_begin = pix_center + below * float(j);
    //                render_line(scene, row_begin, j);
    //                n++;
    //                UpdateProgress(n / (float)height);
    //             }
    //         }));
    //     }

    //     for (auto& thread : threads) { thread.join(); }

    //     return frame_buffer;
    // }

}

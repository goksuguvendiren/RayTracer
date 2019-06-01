//
// Created by Göksu Güvendiren on 2019-05-09.
//

#include <optional>
#include <thread>

#include <renderer.hpp>
#include "scene.hpp"
#include "ray.hpp"
#include "utils.hpp"

//#define THREADS_DISABLED

static glm::vec3 reflect(const glm::vec3& light, const glm::vec3& normal)
{
    return glm::normalize(2 * glm::dot(normal, light) * normal - light);
}

glm::vec3 refract(const glm::vec3 &I, const glm::vec3 &N, const float &ior)
{
    float cosi = std::clamp(glm::dot(I, N), -1.f, 1.f);
    float etai = 1, etat = ior;
    glm::vec3 n = N;
    if (cosi < 0) { cosi = -cosi; } else { std::swap(etai, etat); n = -N; }
    float eta = etai / etat;
    float k = 1 - eta * eta * (1 - cosi * cosi);
    return k < 0 ? glm::vec3{0, 0, 0} : eta * I + (eta * cosi - sqrtf(k)) * n;
}

glm::vec3 rtr::renderer::trace(const rtr::scene& scene, const rtr::ray& ray, int rec_depth, int max_rec_depth)
{
    auto color = glm::vec3{0.f, 0.f, 0.f};  
    std::optional<rtr::payload> hit = scene.hit(ray);

    if (!hit) return color;

//    return (hit->hit_normal + glm::vec3(1, 1, 1)) / 2.f;

    if (rec_depth >= max_rec_depth) return color;
    color = hit->material->shade(scene, *hit);
//    color = shade(scene, *hit);

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
        auto eta_1 = 1.f;
        auto eta_2 = 1.5f;
        auto refraction_direction = refract(ray.direction(), hit->hit_normal, eta_2 / eta_1);
        if (glm::length(refraction_direction) > 0.1)
        {
            rtr::ray refracted_ray(hit->hit_pos + (refraction_direction * 1e-3f), refraction_direction, false);
            color += hit->material->trans * trace(scene, refracted_ray, rec_depth + 1, max_rec_depth);
        }
    }

    return color;
}

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
    if  ( event == cv::EVENT_LBUTTONDOWN )
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << std::endl;
    }
}

static void UpdateProgress(float progress)
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

glm::vec3 rtr::renderer::render_pixel(const rtr::scene& scene, const camera& camera, const glm::vec3& pix_center,
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
            auto ray = rtr::ray(camera_pos, sub_pix_position - camera_pos, true);

            color += trace(scene, ray, 0, 5);
        }
    }
    
    return color / float(sq_sample_pp * sq_sample_pp);
}

void rtr::renderer::render_line(const rtr::scene &scene, const glm::vec3& row_begin, int i)
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

std::vector<glm::vec3> rtr::renderer::render(const rtr::scene &scene)
{
    const auto& camera = scene.get_camera();
    rtr::image_plane plane(camera, width, height);

    auto right = (1 / float(width)) * plane.right;
    auto below = -(1 / float(height)) * plane.up;

    auto pix_center = plane.top_left_position();
    
    cv::namedWindow("window");
    cv::setMouseCallback("window", CallBackFunc, NULL);

    int number_of_threads = std::thread::hardware_concurrency();
    std::cerr << "Threads enabled! Running " << number_of_threads << " threads!\n";
    std::vector<std::thread> threads;
    int n = 0;
    for (int i = 0; i < number_of_threads; ++i)
    {
        threads.push_back(std::thread([i, &scene, pix_center, this, &below, &n, number_of_threads]
        {
//            std::cerr << "thread " << i << " started!\n";
            for (int j = i; j < height; j += number_of_threads)
            {
                auto row_begin = pix_center + below * float(j);
                render_line(scene, row_begin, j);
                n++;
                UpdateProgress(n / (float)height);
            }
        }));
    }
    
    for (auto& thread : threads) { thread.join(); }

    return frame_buffer;
}

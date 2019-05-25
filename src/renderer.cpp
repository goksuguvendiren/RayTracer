//
// Created by Göksu Güvendiren on 2019-05-09.
//

#include <optional>
#include <thread>
#include <random>

#include <renderer.hpp>
#include "scene.hpp"
#include "ray.hpp"
#include "utils.hpp"

#define THREADS_ENABLED

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

// Do it recursively
glm::vec3 shadow_trace(const rtr::scene& scene, const rtr::ray& ray, float light_distance)
{
    auto shadow = glm::vec3{1.f, 1.f, 1.f};
    std::optional<rtr::payload> hit = scene.hit(ray);
    
    if (!hit) return shadow;
    if (hit && (hit->param < light_distance)) // some base case checks to terminate
    {
        auto& diffuse = hit->material->diffuse;
        auto normalized_diffuse = diffuse / std::max(std::max(diffuse.x, diffuse.y), diffuse.z);
        return shadow * normalized_diffuse * hit->material->trans;
    }
    
    auto hit_position = hit->hit_pos + ray.direction() * 1e-4f;
    rtr::ray shadow_ray = rtr::ray(hit_position, ray.direction(), false);
    
    return shadow * shadow_trace(scene, shadow_ray, light_distance - glm::length(hit->hit_pos - ray.origin()));
}

glm::vec3 shade(const rtr::scene& scene, const rtr::payload& payload)
{
    auto& mat = payload.material;

    auto ambient = (1 - mat->trans) * mat->ambient * mat->diffuse;
    glm::vec3 color = ambient;

    scene.for_each_light([&payload, &color, &mat, &scene](auto light)
    {
        float epsilon = 1e-4;
        auto hit_position = payload.hit_pos + payload.hit_normal * epsilon;
        rtr::ray shadow_ray = rtr::ray(hit_position, light.direction(hit_position), false);
        auto shadow = shadow_trace(scene, shadow_ray, light.distance(hit_position));

        if(shadow.x <= epsilon && shadow.y <= epsilon && shadow.z <= epsilon) return;// complete shadow, no need to compute the rest
        
        auto reflection_vector = reflect(light.direction(payload.hit_pos), payload.hit_normal);
        auto cos_angle = glm::dot(reflection_vector, -payload.ray.direction());
        auto highlight = std::max(0.f, std::pow(cos_angle, mat->exp * 120));

        auto diffuse = (1 - mat->trans) * mat->diffuse * std::max(glm::dot(payload.hit_normal, light.direction(payload.hit_pos)), 0.0f);
        auto specular = mat->specular * highlight;

        auto attenuation = light.attenuate(payload.hit_pos);

        color += (diffuse + specular) * light.color * attenuation * shadow;
    });

    return color;
}

glm::vec3 rtr::renderer::trace(const rtr::scene& scene, const rtr::ray& ray, int rec_depth, int max_rec_depth)
{
    auto color = glm::vec3{0.f, 0.f, 0.f};  
    std::optional<rtr::payload> hit = scene.hit(ray);

    if (!hit) return color;

//    return (hit->hit_normal + glm::vec3(1, 1, 1)) / 2.f;

    if (rec_depth >= max_rec_depth) return color;
    color = shade(scene, *hit);

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

glm::vec3 rtr::renderer::render_pixel(const rtr::scene& scene, const glm::vec3& camera_pos, const glm::vec3& pix_center, const glm::vec3& right, const glm::vec3& below)
{
    // supersampling - jittered stratified
    constexpr int sq_sample_pp = 1;
    glm::vec3 color = {0, 0, 0};
    
    for (int k = 0; k < sq_sample_pp; ++k)
    {
        for (int m = 0; m < sq_sample_pp; ++m)
        {
            auto sub_pix_position = get_pixel_pos(pix_center, right, below, k, m, sq_sample_pp);
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
    
    auto right = (1 / float(width)) * plane.right;
    auto below = -(1 / float(height)) * plane.up;
    
    glm::vec3 pix_center = row_begin;
    for (int j = 0; j < width; ++j)
    {
        pix_center += right;
        auto color = render_pixel(scene, camera.position(), pix_center, right, below);

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

#ifdef THREADS_ENABLED
    constexpr int number_of_threads = 2;
    std::vector<std::thread> threads;
    int n = 0;
    for (int i = 0; i < number_of_threads; ++i)
    {
        threads.push_back(std::thread([i, &scene, pix_center, this, &below, &n]
        {
            std::cerr << "thread " << i << " started!\n";
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
#else
    auto row_begin = pix_center;// + i * below;
    for (int i = 0; i < height; ++i)
    {
        row_begin += below;// * 4;
        render_line(scene, row_begin, i);

        UpdateProgress(i / (float)height);
    }
#endif

    return frame_buffer;
}

float get_random_float()
{
    static std::random_device rd;  //Will be used to obtain a seed for the random number engine
    static std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<float> dis(0, 1.0);
    
    return dis(gen);
}

glm::vec3 rtr::renderer::get_pixel_pos(const glm::vec3& top_left, const glm::vec3& right, const glm::vec3& below, int u, int v, int sq_sample_pp)
{
    auto random_u = get_random_float();
    auto random_v = get_random_float();
    
    auto x_offset = (random_u * (1.f / sq_sample_pp)) + ((float)u / sq_sample_pp);
    auto y_offset = (random_v * (1.f / sq_sample_pp)) + ((float)v / sq_sample_pp);

    auto sample = top_left + right * x_offset + below * y_offset;
    return sample;
}

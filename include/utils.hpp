#include <iostream>
#include <random>
#include <glm/gtc/quaternion.hpp>

#pragma once

inline std::ostream& operator<<(std::ostream& os, const glm::vec3& vec)
{
    os << vec.x << ", " << vec.y << ", " << vec.z;
    return os;
}

inline std::random_device rd;  //Will be used to obtain a seed for the random number engine
inline std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
inline float get_random_float()
{
    std::uniform_real_distribution<float> dis(0, 1.0);
    return dis(gen);
}

inline float get_random_float(float min, float max)
{
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

inline glm::vec3 sample_hemisphere(int ms_id, int max_ms)
{
    static std::mt19937 rng(0);
    static std::uniform_real_distribution<float> dist1(0, 1);
    static std::uniform_real_distribution<float> dist2(0, 1);
    
    static constexpr double pi = 3.14159265358979323846;
    
    auto ksi1 = dist1(rng);
    auto ksi2 = dist2(rng);
//    return sample_cosine(ksi1, ksi2);
    
    /*ksi1 = lerp(float(ms_id) / max_ms, float(ms_id + 1) / max_ms, ksi1);
     ksi2 = lerp(float(ms_id) / max_ms, float(ms_id + 1) / max_ms, ksi2);*/
    
    auto v = glm::vec3(
                       std::cos(2 * pi * ksi2) * std::sqrt(1 - ksi1 * ksi1),
                       ksi1,
                       std::sin(2 * pi * ksi2) * std::sqrt(1 - ksi2 * ksi2));
    return v;
}

inline glm::vec3 sample_hemisphere(const glm::vec3& towards, int ms_id = 0, int max_ms = 0)
{
    auto base_sample = sample_hemisphere(ms_id, max_ms);
    if (towards == glm::vec3(0, 0, -1))
    {
        return -base_sample;
    }
    
    auto axis = glm::cross(glm::vec3(0, 0, 1), towards);
    auto angle = std::acos(glm::dot(towards, glm::vec3(0, 0, 1)));
    
    auto rotate = glm::angleAxis(angle, axis);
    return rotate * base_sample;
}

inline bool hasEnding (std::string const &fullString, std::string const &ending)
{
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

inline glm::vec3 to_vec3(float* vert)
{
    return {vert[0], vert[1], vert[2]};
}

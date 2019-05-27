#include <iostream>
#include <random>

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


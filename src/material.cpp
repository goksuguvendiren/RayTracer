#include "material.hpp"
#include "payload.hpp"
#include "scene.hpp"

glm::vec3 rtr::material::shade(const rtr::scene &scene, const rtr::payload &pld) const
{
    auto amb = (1 - trans) * ambient * diffuse;
    glm::vec3 color = amb;
    
    scene.for_each_light([&pld, &color, this, &scene](auto light)
     {
         float epsilon = 1e-4;
         auto hit_position = pld.hit_pos + pld.hit_normal * epsilon;
         rtr::ray shadow_ray = rtr::ray(hit_position, light.direction(hit_position), false);
         auto shadow = scene.shadow_trace(shadow_ray, light.distance(hit_position));
         
         if(shadow.x <= epsilon && shadow.y <= epsilon && shadow.z <= epsilon) return;// complete shadow, no need to compute the rest
         
         auto reflection_vector = reflect(light.direction(pld.hit_pos), pld.hit_normal);
         auto cos_angle = glm::dot(reflection_vector, -pld.ray.direction());
         auto highlight = std::max(0.f, std::pow(cos_angle, exp * 120));
         
         auto diffuse = (1 - trans) * this->diffuse * std::max(glm::dot(pld.hit_normal, light.direction(pld.hit_pos)), 0.0f);
         auto specular = this->specular * highlight;
         
         auto attenuation = light.attenuate(pld.hit_pos);
         
         color += (diffuse + specular) * light.color * attenuation * shadow;
     });
    
    return color;

}

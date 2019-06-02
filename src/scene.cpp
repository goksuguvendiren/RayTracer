#include <memory>
#include <scene_io.h>
#include <vertex.hpp>
#include "scene.hpp"
#include "primitives/sphere.hpp"
#include "primitives/mesh.hpp"
#include "utils.hpp"

#define OBJL_CONSOLE_OUTPUT

#include "OBJ_Loader.hpp"

glm::vec3 to_vec3(float* vert)
{
    return {vert[0], vert[1], vert[2]};
}

glm::vec3 to_vec3(const objl::Vector3& vert)
{
    return {vert.X, vert.Y, vert.Z};
}

rtr::primitives::face::normal_types to_rtr(NormType normal)
{
    using rtr::primitives::face;
    switch (normal)
    {
        case NormType::PER_FACE_NORMAL:
            return face::normal_types::per_face;
        case NormType::PER_VERTEX_NORMAL:
            return face::normal_types::per_vertex;
    }
}

rtr::primitives::face::material_binding to_rtr(MaterialBinding material)
{
    using rtr::primitives::face;
    switch (material)
    {
        case MaterialBinding::PER_OBJECT_MATERIAL:
            return face::material_binding::per_object;
        case MaterialBinding::PER_VERTEX_MATERIAL:
            return face::material_binding::per_vertex;
    }
}

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

std::optional<rtr::payload> rtr::scene::hit(const rtr::ray& ray) const
{
    std::optional<rtr::payload> min_hit = std::nullopt;

    for (auto& sphere : spheres)
    {
        auto hit = sphere.hit(ray);
        if (!hit) continue;
        if (!min_hit || hit->param < min_hit->param)
        {
            min_hit = *hit;
        }
    }

    for (auto& mesh : meshes)
    {
        auto hit = mesh.hit(ray);
        if (!hit) continue;
        if (std::isnan(hit->param)) throw std::runtime_error("param is nan in scene::hit()!");

        if (!min_hit || hit->param < min_hit->param)
        {
            if (std::isnan(hit->param)) throw std::runtime_error("param is nan in scene::hit() 2!");
            min_hit = *hit;
        }
    }

    if (min_hit && std::isnan(min_hit->param))throw std::runtime_error("param is nan in scene::hit() 3!");

    return min_hit;
}

glm::vec3 rtr::scene::trace(const rtr::ray& ray) const
{
    auto color = glm::vec3{0.f, 0.f, 0.f};
    std::optional<rtr::payload> pld = hit(ray);
    
    if (!pld) return color;
    
    if (pld->ray.rec_depth >= maxRecursionDepth) return color;
    color = pld->material->shade(*this, *pld);
    
    // Reflection :
    if (pld->material->specular.x > 0.f || pld->material->specular.y > 0.f || pld->material->specular.z > 0.f)
    {
        auto reflection_direction = reflect(ray.direction(), pld->hit_normal);
        rtr::ray reflected_ray(pld->hit_pos + (reflection_direction * 1e-3f), reflection_direction, pld->ray.rec_depth + 1, false);
        color += pld->material->specular * trace(reflected_ray);
    }
    
    // Refraction
    if (pld->material->trans > 0.f)
    {
        auto eta_1 = 1.f;
        auto eta_2 = 1.5f;
        
        auto refraction_direction = refract(ray.direction(), pld->hit_normal, eta_2 / eta_1);
        if (glm::length(refraction_direction) > 0.1)
        {
            rtr::ray refracted_ray(pld->hit_pos + (refraction_direction * 1e-3f), refraction_direction, pld->ray.rec_depth + 1, false);
            color += pld->material->trans * trace(refracted_ray);
        }
    }

    return color;
}

// Do it recursively
glm::vec3 rtr::scene::shadow_trace(const rtr::ray& ray, float light_distance) const
{
    auto shadow = glm::vec3{1.f, 1.f, 1.f};
    std::optional<rtr::payload> pld = hit(ray);
    
    if (!pld) return shadow;
    if (pld && (pld->param < light_distance)) // some base case checks to terminate
    {
        auto& diffuse = pld->material->diffuse;
        auto normalized_diffuse = diffuse / std::max(std::max(diffuse.x, diffuse.y), diffuse.z);
        return shadow * normalized_diffuse * pld->material->trans;
    }
    
    auto hit_position = pld->hit_pos + ray.direction() * 1e-4f;
    rtr::ray shadow_ray = rtr::ray(hit_position, ray.direction(), ray.rec_depth, false);
    
    return shadow * shadow_trace(shadow_ray, light_distance - glm::length(pld->hit_pos - ray.origin()));
}

rtr::scene::scene(SceneIO* io) // Load veach scene.
{
    auto& cam = io->camera;
    camera = rtr::camera(to_vec3(cam->position), to_vec3(cam->viewDirection), to_vec3(cam->orthoUp), cam->focalDistance, cam->verticalFOV);
//    camera = rtr::camera(to_vec3(cam->position), to_vec3(cam->viewDirection), to_vec3(cam->orthoUp), cam->focalDistance, cam->verticalFOV, 12.f, false);

    auto* light = io->lights;
    while(light != nullptr)
    {
        if (light->type == LightType::POINT_LIGHT)
        {
            lghts.emplace_back(to_vec3(light->position), to_vec3(light->color));
        }
        else if (light->type == LightType::DIRECTIONAL_LIGHT)
        {
            dir_lghts.emplace_back(to_vec3(light->direction), to_vec3(light->color));
        }
        light = light->next;
    }

    int id = 0;

    auto* obj = io->objects;
    while(obj != nullptr)  // iterate through the objects
    {
        if (obj->type == ObjType::SPHERE_OBJ)
        {
            auto data = reinterpret_cast<SphereIO*>(obj->data);

            spheres.emplace_back(obj->name ? obj->name : "", to_vec3(data->origin), data->radius,
                                 to_vec3(data->xaxis), data->xlength,
                                 to_vec3(data->yaxis), data->ylength,
                                 to_vec3(data->zaxis), data->zlength);

            auto& sph = spheres.back();

//            std::cerr << glm::length(sph.origin - to_vec3(cam->position)) << '\n';
            sph.id = id++;
            for (int i = 0; i < obj->numMaterials; ++i)
            {
                sph.materials.emplace_back(to_vec3(obj->material->diffColor), to_vec3(obj->material->ambColor),
                                           to_vec3(obj->material->specColor), to_vec3(obj->material->emissColor),
                                           obj->material->shininess, obj->material->ktran);
            }
        }
        else
        {
            auto data = reinterpret_cast<PolySetIO*>(obj->data);
            assert(data->type == PolySetType::POLYSET_TRI_MESH);

            std::vector<material> materials;
            materials.reserve(obj->numMaterials);
            for (int i = 0; i < obj->numMaterials; ++i)
            {
                materials.emplace_back(to_vec3(obj->material[i].diffColor), to_vec3(obj->material[i].ambColor),
                                       to_vec3(obj->material[i].specColor), to_vec3(obj->material[i].emissColor),
                                       obj->material[i].shininess, obj->material[i].ktran);
            }

            std::vector<rtr::primitives::face> faces;
            for (int i = 0; i < data->numPolys; ++i)
            {
                auto polygon = data->poly[i];
                assert(polygon.numVertices == 3);

                std::array<rtr::vertex, 3> vertices;
                for (int j = 0; j < polygon.numVertices; ++j)
                {
                    auto& poly = polygon.vert[j];
                    vertices.at(j) = rtr::vertex(to_vec3(poly.pos), to_vec3(poly.norm), &materials.at(poly.materialIndex), poly.s, poly.t);
                }
                faces.emplace_back(vertices, to_rtr(data->normType), to_rtr(data->materialBinding));
            }

            meshes.emplace_back(faces, obj->name ? obj->name : "");
            auto& mesh = meshes.back();
            mesh.id = id++;
            mesh.materials = std::move(materials);
        }
        obj = obj->next;
    }
}

bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

rtr::scene::scene(const std::string &filename) {

    if (hasEnding(filename, ".obj"))
    {
        std::cerr << "Loading the obj file\n";
        load_obj(filename);
        std::cerr << "Finished loading!\n";
    }
    else if (hasEnding(filename, ".ascii"))
    {
        *this = scene(readScene(filename.c_str()));
    }
    else
    {
        throw std::runtime_error("unknown file type!");
    }
}

void rtr::scene::load_obj(const std::string& filename)
{
    objl::Loader loader;
    loader.LoadFile(filename);
    
    // create a default camera located at the origin, looking at the -z direction.
    auto focal_distance = 12.2118f;
    auto vertical_fov = 0.785398f;
    camera = rtr::camera(glm::vec3{-1, 3, 10}, glm::vec3{0, 0, -1}, glm::vec3{0, 1, 0}, focal_distance, vertical_fov, focal_distance, false ); // focal dist = image plane dist
//    camera = rtr::camera(glm::vec3{-1, 3, 10}, glm::vec3{0, 0, -1}, glm::vec3{0, 1, 0}, focal_distance, vertical_fov);

    // create default light sources
    lghts.emplace_back(glm::vec3{-1.84647, 0.778452, 2.67544}, glm::vec3{1, 1, 1});
    lghts.emplace_back(glm::vec3{2.09856, 1.43311, 0.977627}, glm::vec3{1, 1, 1});

    int id = 0;
    for(auto& mesh : loader.LoadedMeshes)
    {
        std::cerr << "Loading mesh : " << id << "\n";
        std::vector<rtr::primitives::face> faces;
        for(int i = 0; i < mesh.Vertices.size(); i += 3)
        {
            std::array<rtr::vertex, 3> face_vertices;
            for(int j = 0; j < 3; j++)
            {
                face_vertices[j] = rtr::vertex(to_vec3(mesh.Vertices[i+j].Position), to_vec3(mesh.Vertices[i+j].Normal), nullptr,
                        mesh.Vertices[i+j].TextureCoordinate.X, mesh.Vertices[i+j].TextureCoordinate.Y);
            }
        
            rtr::primitives::face face_new(face_vertices, rtr::primitives::face::normal_types::per_vertex, rtr::primitives::face::material_binding::per_object);
            faces.push_back(face_new);
        }

        meshes.emplace_back(faces, "");
        auto& m = meshes.back();
        m.id = id++;
        // obj loader allows for only one material per mesh
        // also, emitted color is 0, meaning that these meshes cannot emit color right now.

        auto& material = mesh.MeshMaterial;
        if (material)
            m.materials.emplace_back(to_vec3(material->Kd), to_vec3(material->Ka), to_vec3(material->Ks), glm::vec3{0, 0, 0}, material->Ns, 0);
        else
        {
            m.materials.emplace_back(glm::vec3{0.5, 0.5, 0.5}, glm::vec3{0.2, 0.2, 0.2}, glm::vec3{0, 0, 0}, glm::vec3{0, 0, 0}, 0, 0);
            std::cerr << "This obj doesn't have any materials, default diffuse material will be used!" << '\n';
        }
    }
}

void rtr::scene::build_acceleration()
{}

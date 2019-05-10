#include <memory>
#include <scene_io.h>
#include <vertex.hpp>
#include "scene.hpp"

glm::vec3 to_vec3(float* vert)
{
    return {vert[0], vert[1], vert[2]};
}

rtr::scene::scene(SceneIO* io)
{
    auto& cam = io->camera;
    camera = rtr::camera(to_vec3(cam->position), to_vec3(cam->viewDirection), to_vec3(cam->orthoUp), cam->focalDistance, cam->verticalFOV);

    auto* light = io->lights;
    while(light != nullptr)
    {
        lights.emplace_back(to_vec3(light->position), to_vec3(light->color));
        light = light->next;
    }

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
            assert(data->normType == NormType::PER_FACE_NORMAL);
            assert(data->materialBinding == MaterialBinding::PER_OBJECT_MATERIAL);

            for (int i = 0; i < data->numPolys; ++i)
            {
                auto polygon = data->poly[i];
                std::vector<rtr::vertex> vertices;
                for (int j = 0; j < polygon.numVertices; ++j)
                {
                    vertices.emplace_back(to_vec3(polygon.vert[j].pos), to_vec3(polygon.vert[j].norm), polygon.vert[j].materialIndex, polygon.vert[j].s, polygon.vert[j].t);
                }

                meshes.emplace_back(vertices);
            }
        }
        obj = obj->next;
    }
}

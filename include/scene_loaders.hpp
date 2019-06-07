#include <iostream>
#include <map>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader.hpp"

#include "tinyxml/tinyxml2.h"


inline rtr::primitives::face::normal_types to_rtr(NormType normal)
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

inline rtr::primitives::face::material_binding to_rtr(MaterialBinding material)
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

inline rtr::vertex get_vertex(std::istringstream& stream, int id)
{
    rtr::vertex vert;
    
    float datax;
    float datay;
    float dataz;
    
    stream >> datax;
    stream >> datay;
    stream >> dataz;
    
    return rtr::vertex{glm::vec3{datax, datay, dataz}};
}

std::vector<rtr::vertex> LoadVertexData(tinyxml2::XMLElement *elem)
{
    std::istringstream stream { elem->GetText() };
    
    std::vector<rtr::vertex> verts;
    int id = 0;
    while(stream){
        verts.push_back(get_vertex(stream, id++));
        assert(id == verts.size());
    }
    
    return verts;
}

inline std::optional<rtr::primitives::face> get_face(std::istringstream& stream, const std::vector<rtr::vertex>& vertices, int vertexOffset, int texCoordsOffset)
{
    int x, y, z;
    if (!(stream >> x)) return std::nullopt;
    if (!(stream >> y)) return std::nullopt;
    if (!(stream >> z)) return std::nullopt;
    
    x += vertexOffset;
    y += vertexOffset;
    z += vertexOffset;
    
    auto pos_a = vertices[x-1];
    auto pos_b = vertices[y-1];
    auto pos_c = vertices[z-1];
    
    std::array<rtr::vertex, 3> face_vertices;
    face_vertices[0] = pos_a;
    face_vertices[1] = pos_b;
    face_vertices[2] = pos_c;
    
    return rtr::primitives::face{face_vertices, rtr::primitives::face::normal_types::per_face, rtr::primitives::face::material_binding::per_object};
}

inline auto GetTransformations(std::istringstream& stream)
{
    std::vector<std::string> result;
    
    while(stream.good()){
        std::string tr;
        stream >> tr;
        result.push_back(tr);
    }
    
    return result;
}

std::vector<rtr::primitives::sphere> LoadSpheres(tinyxml2::XMLElement *elem, const std::vector<rtr::vertex>& vertices, const std::map<int, rtr::material>& materials)
{
    std::vector<rtr::primitives::sphere> spheres;
    
    for (auto child = elem->FirstChildElement("Sphere"); child != NULL; child = child->NextSiblingElement("Sphere"))
    {
        int id;
        child->QueryIntAttribute("id", &id);
        int matID = child->FirstChildElement("Material")->IntText(0);
        int centerID = child->FirstChildElement("Center")->IntText(0);
        float radius = child->FirstChildElement("Radius")->FloatText(0);
        
        glm::vec3 center = vertices[centerID - 1].position();
        
        rtr::primitives::sphere sp("", center, radius, materials.at(matID));
        
        spheres.push_back(std::move(sp));
    }
    
    return spheres;
}

std::vector<rtr::primitives::mesh> LoadMeshes(tinyxml2::XMLElement *elem, const std::vector<rtr::vertex>& vertices, const std::map<int, rtr::material>& materials)
{
    std::vector<rtr::primitives::mesh> meshes;
    
    for (auto child = elem->FirstChildElement("Mesh"); child != nullptr; child = child->NextSiblingElement("Mesh")) {
        int id;
        child->QueryIntAttribute("id", &id);
        
        bool is_art = false;
        if (child->QueryBoolText(&is_art))
        {
            is_art = true;
        }
        
        auto FaceData = child->FirstChildElement("Faces");
        std::istringstream stream { FaceData->GetText() };
        int vertexOffset = 0;
        int texCoordOffset = 0;
        
        if (FaceData->QueryIntAttribute("vertexOffset", &vertexOffset));
        if (FaceData->QueryIntAttribute("textureOffset", &texCoordOffset));
        
        std::optional<rtr::primitives::face> fc;
        int index = 1;
        
        int matID = child->FirstChildElement("Material")->IntText(0);
        
        std::vector<rtr::primitives::face> faces;
        while((fc = get_face(stream, vertices, vertexOffset, texCoordOffset)))
        {
            auto tri = *fc;
            faces.push_back(std::move(tri));
        }
        
        meshes.emplace_back(faces, "");
        auto& mesh = meshes.back();
        mesh.materials.push_back(materials.at(matID));
//        mesh.configure_materials();
    }
    
    return meshes;
}

namespace rtr
{
namespace loaders
{
    inline std::string GetBaseDir(const std::string& filepath)
    {
        if (filepath.find_last_of("/\\") != std::string::npos)
            return filepath.substr(0, filepath.find_last_of("/\\"));
        return "";
    }
    
	inline rtr::scene load_from_tinyobj(const std::string& filename)
	{
//        objl::Loader loader;
//        loader.LoadFile(filename);
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn;
        std::string err;
        
        auto base_dir = GetBaseDir(filename);
        if (base_dir.empty())
        {
            base_dir = ".";
        }
        
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str(), base_dir.c_str());
        if (!warn.empty()) {
            std::cout << "WARN: " << warn << std::endl;
        }
        if (!err.empty()) {
            std::cerr << err << std::endl;
        }
        
        rtr::scene scene;
        
        // create a default camera located at the origin, looking at the -z direction.
        auto focal_distance = 12.2118f;
        auto vertical_fov = 0.785398f;

        scene.camera = rtr::camera(glm::vec3{278, 273, -800}, glm::vec3{0, 0, 1}, glm::vec3{0, 1, 0}, focal_distance, vertical_fov, focal_distance, false ); // focal dist = image plane dist
//        //    camera = rtr::camera(glm::vec3{-1, 3, 10}, glm::vec3{0, 0, -1}, glm::vec3{0, 1, 0}, focal_distance, vertical_fov, focal_distance, false ); // focal dist = image plane dist
//        //    camera = rtr::camera(glm::vec3{-1, 3, 10}, glm::vec3{0, 0, -1}, glm::vec3{0, 1, 0}, focal_distance, vertical_fov);

        // create default light sources
        scene.lghts.emplace_back(glm::vec3{-1.84647, 0.778452, 2.67544}, glm::vec3{1, 1, 1});
        scene.lghts.emplace_back(glm::vec3{2.09856, 1.43311, 0.977627}, glm::vec3{1, 1, 1});

        int id = 0;
        for(auto& shape : shapes)
        {
            std::cerr << "Loading mesh : " << id << "\n";
            std::cerr << shape.mesh.indices.size() << '\n';
            std::vector<rtr::primitives::face> faces;
            
            for (int i = 0; i < shape.mesh.indices.size(); i += 3)
            {
                std::array<rtr::vertex, 3> face_vertices;
                for (int i = 0; i < 3; ++i)
                {
                    tinyobj::index_t idx = shape.mesh.indices[i];
                    tinyobj::real_t vx = attrib.vertices[3*idx.vertex_index+0];
                    tinyobj::real_t vy = attrib.vertices[3*idx.vertex_index+1];
                    tinyobj::real_t vz = attrib.vertices[3*idx.vertex_index+2];
//                    tinyobj::real_t nx = attrib.normals[3*idx.normal_index+0];
//                    tinyobj::real_t ny = attrib.normals[3*idx.normal_index+1];
//                    tinyobj::real_t nz = attrib.normals[3*idx.normal_index+2];
//                    tinyobj::real_t tx = attrib.texcoords[2*idx.texcoord_index+0];
//                    tinyobj::real_t ty = attrib.texcoords[2*idx.texcoord_index+1];

                    face_vertices[i] = rtr::vertex(glm::vec3{vx, vy, vz});//, glm::vec3{0, ny, nz}, nullptr, tx, ty);
                }
                rtr::primitives::face face_new(face_vertices, rtr::primitives::face::normal_types::per_vertex, rtr::primitives::face::material_binding::per_object);
                faces.push_back(face_new);
            }
            
            scene.meshes.emplace_back(faces, "");
            
            auto& m = scene.meshes.back();
            m.id = id++;
            
        
//            for(int i = 0; i < mesh.Vertices.size(); i += 3)
//            {
//                std::array<rtr::vertex, 3> face_vertices;
//                for(int j = 0; j < 3; j++)
//                {
//                    face_vertices[j] = rtr::vertex(to_vec3(mesh.Vertices[i+j].Position), to_vec3(mesh.Vertices[i+j].Normal), nullptr,
//                                                   mesh.Vertices[i+j].TextureCoordinate.X, mesh.Vertices[i+j].TextureCoordinate.Y);
//                }
//
//                rtr::primitives::face face_new(face_vertices, rtr::primitives::face::normal_types::per_vertex, rtr::primitives::face::material_binding::per_object);
//                faces.push_back(face_new);
//            }
//
//            meshes.emplace_back(faces, "");
//            auto& m = meshes.back();
//            m.id = id++;
//            // obj loader allows for only one material per mesh
//            // also, emitted color is 0, meaning that these meshes cannot emit color right now.
//
//            auto& material = mesh.MeshMaterial;
//            if (material)
//                m.materials.emplace_back(to_vec3(material->Kd), to_vec3(material->Ka), to_vec3(material->Ks), glm::vec3{0, 0, 0}, material->Ns, 0);
//            else
//            {
//                m.materials.emplace_back(glm::vec3{0.5, 0.5, 0.5}, glm::vec3{0.2, 0.2, 0.2}, glm::vec3{0, 0, 0}, glm::vec3{0, 0, 0}, 0, 0);
//                std::cerr << "This obj doesn't have any materials, default diffuse material will be used!" << '\n';
//            }
        }
        return scene;
	}
    
    inline glm::vec3 GetElem(tinyxml2::XMLElement* element)
    {
        glm::vec3 color;
        
        std::istringstream ss {element->GetText()};
        ss >> color.r;
        ss >> color.g;
        ss >> color.b;
        
        return color;
    }
    
    std::pair<int, rtr::material> load_material(tinyxml2::XMLElement *child)
    {
        int id;
        child->QueryIntAttribute("id", &id);

        auto  ambient  = GetElem(child->FirstChildElement("AmbientReflectance"));
        auto  diffuse  = GetElem(child->FirstChildElement("DiffuseReflectance"));
        auto  specular = GetElem(child->FirstChildElement("SpecularReflectance"));
        
        glm::vec3 mirror = {0, 0, 0}, transparency = {0, 0, 0}, glass = {0, 0, 0};
        float phongEx = 0;
        
        tinyxml2::XMLElement* tmp;
        if ((tmp = child->FirstChildElement("PhongExponent")))
            phongEx  = tmp->FloatText(1);
        
        rtr::material mat{diffuse, ambient, specular, glm::vec3{0, 0, 0}, phongEx, 0};
        
        if ((tmp = child->FirstChildElement("Transparency"))) // then the material should be a glass (transparent)
        {
            tinyxml2::XMLElement* tmp_m;
            auto transp = GetElem(tmp);
            mat.trans = transp[0];
            float refraction_index = 1.f;
            if ((tmp_m = child->FirstChildElement("RefractionIndex"))) {
                refraction_index = tmp_m->FloatText(1.f);
            }
            
            mat.refr_index = refraction_index;
        }

        return std::make_pair(id, mat);
    }
    
    rtr::light load_point_light(tinyxml2::XMLElement *child)
    {
        assert(child->Name() == std::string("PointLight"));
        int id;
        child->QueryIntAttribute("id", &id);
        
        auto position  = GetElem(child->FirstChildElement("Position"));
        auto intensity = GetElem(child->FirstChildElement("Intensity"));
        
        return rtr::light(position, intensity);
    }
    
    rtr::dir_light load_directional_light(tinyxml2::XMLElement *child)
    {
        int id;
        child->QueryIntAttribute("id", &id);
        
        auto direction  = GetElem(child->FirstChildElement("Direction"));
        auto radiance = GetElem(child->FirstChildElement("Radiance"));
        
        return rtr::dir_light(direction, radiance);
    }
    
    inline rtr::scene load_from_xml(const std::string& filename)
    {
        rtr::scene scene;
        
        tinyxml2::XMLDocument document;
        document.LoadFile(filename.c_str());
        
        if (document.Error()){
            document.PrintError();
            std::abort();
        }
        
        auto docscene = document.FirstChildElement("Scene");
        
        if (!docscene){
            std::cerr << "Not a valid scene file" << '\n';
            std::abort();
        }
        
        if (auto elem = docscene->FirstChildElement("BackgroundColor")){
            auto color = GetElem(elem);
            scene.background_color = color;
        }
        
        if (auto elem = docscene->FirstChildElement("ShadowRayEpsilon")){
            scene.shadow_ray_epsilon = elem->FloatText();
        }
        
        if (auto elem = docscene->FirstChildElement("MaxRecursionDepth")){
            scene.max_recursion_depth = elem->IntText(1);
        }
        
        if (auto elem = docscene->FirstChildElement("IntersectionTestEpsilon")){
            scene.intersection_test_epsilon = elem->FloatText();
        }
        
        if (auto elem = docscene->FirstChildElement("Camera"))
        {
            auto position = GetElem(elem->FirstChildElement("Position"));
            auto view = GetElem(elem->FirstChildElement("Gaze"));
            auto up = GetElem(elem->FirstChildElement("Up"));
            
            int focal_distance = 12;
            tinyxml2::XMLElement* foc;
            if ((foc = elem->FirstChildElement("FocusDistance")))
            {
                focal_distance = foc->IntText(1);
            }
            auto vertical_fov = 0.985398f;
            
            scene.camera = rtr::camera(position, view, up, focal_distance, vertical_fov, focal_distance, false );
        }
        else {
            std::cerr << "Could not read camera information\n";
            std::abort();
        }
        
        if (auto elem = docscene->FirstChildElement("Lights"))
        {
            for (auto child = elem->FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
            {
                if (child->Name() == std::string("PointLight"))
                    scene.lghts.push_back(load_point_light(child));
                else scene.dir_lghts.push_back(load_directional_light(child));
            }
        }

        std::map<int, rtr::material> materials;
        if (auto elem = docscene->FirstChildElement("Materials")){
            for (auto child = elem->FirstChildElement("Material"); child != NULL; child = child->NextSiblingElement())
            {
                materials.insert(load_material(child));
            }
        }
        
        std::vector<rtr::vertex> vertices;
        if (auto elem = docscene->FirstChildElement("VertexData"))
        {
            vertices = LoadVertexData(elem);
        }
//
//        if (auto elem = docscene->FirstChildElement("Transformations")){
//            transformations  = LoadTransformations(elem);
//        }
        
//        if (auto elem = docscene->FirstChildElement("Textures"))
//        {
//            textures = LoadTextures(elem);
//        }
        
        if(auto objects = docscene->FirstChildElement("Objects"))
        {
            scene.spheres = LoadSpheres(objects, vertices, materials);
            scene.meshes = LoadMeshes(objects, vertices, materials);
        }
        
        scene.print();
        
        return scene;
    }
    
    inline rtr::scene load_from_veach(const std::string& filename)
    {
        rtr::scene scene;
        
        auto io = readScene(filename.c_str());
        auto& cam = io->camera;
        scene.camera = rtr::camera(to_vec3(cam->position), to_vec3(cam->viewDirection), to_vec3(cam->orthoUp), cam->focalDistance, cam->verticalFOV);
        //    camera = rtr::camera(to_vec3(cam->position), to_vec3(cam->viewDirection), to_vec3(cam->orthoUp), cam->focalDistance, cam->verticalFOV, 12.f, false);
        
        auto* light = io->lights;
        while(light != nullptr)
        {
            if (light->type == LightType::POINT_LIGHT)
            {
                scene.lghts.emplace_back(to_vec3(light->position), to_vec3(light->color));
            }
            else if (light->type == LightType::DIRECTIONAL_LIGHT)
            {
                scene.dir_lghts.emplace_back(to_vec3(light->direction), to_vec3(light->color));
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
                
                scene.spheres.emplace_back(obj->name ? obj->name : "", to_vec3(data->origin), data->radius,
                                     to_vec3(data->xaxis), data->xlength,
                                     to_vec3(data->yaxis), data->ylength,
                                     to_vec3(data->zaxis), data->zlength);
                
                auto& sph = scene.spheres.back();
                
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
                
                scene.meshes.emplace_back(faces, obj->name ? obj->name : "");
                auto& mesh = scene.meshes.back();
                mesh.id = id++;
                mesh.materials = std::move(materials);
            }
            obj = obj->next;
        }
        return scene;
    }
    
    inline rtr::scene load(const std::string& filename)
    {
        if (hasEnding(filename, ".obj"))
        {
            return load_from_tinyobj(filename);
        }
        else if (hasEnding(filename, ".ascii"))
        {
            return load_from_veach(filename);
        }
        else if (hasEnding(filename, ".xml"))
        {
            return load_from_xml(filename);
        }
        else
        {
            throw std::runtime_error("unknown file type!");
        }
    }
}
}

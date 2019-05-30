#include <iostream>
#include <scene_io.h>
#include "renderer.hpp"
#include "scene.hpp"

//#define BVH_DISABLED
#define THREADS_DISABLED

int main(int argc, const char** argv)
{
    auto begin = std::chrono::system_clock::now();

    std::string scene_path = "../../Scenes/HW3/test_dof.ascii";
    bool pinhole_camera = true;
    float image_plane_distance = 1.f;
    float lens_width = 1.f;
    if (argc > 1)
    {
        scene_path = std::string(argv[1]);
        if (argc == 4)
        {
            pinhole_camera = false;
            image_plane_distance = std::stof(std::string(argv[2]));
            lens_width = std::stof(std::string(argv[3]));
        }
    }
    
    pinhole_camera = false;
    image_plane_distance = 4;//std::stof(std::string(argv[2]));
    lens_width = 0.1;//std::stof(std::string(argv[3]));
    
    rtr::scene scene(scene_path);
    scene.camera.pinhole = pinhole_camera;
    scene.camera.image_plane_dist = image_plane_distance;
    scene.camera.lens_width = lens_width;
    
    std::cout << "Scene loaded!!\n";
    
    auto end = std::chrono::system_clock::now();
    std::cerr << "Scene loading took : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " millisecs.";

    begin = std::chrono::system_clock::now();
    auto width = 400;
    auto height = 400;
    rtr::renderer r(width, height);
    auto output_buffer = r.render(scene);
    end = std::chrono::system_clock::now();

    std::cerr << "Rendering took : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " millisecs.";
    
    cv::Mat image(width, height, CV_32FC3, output_buffer.data());
    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
    cv::imshow("window", image);
    cv::imwrite("image.tif", image * 255);
    cv::waitKey(0);
    
    return 0;
}

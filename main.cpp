#include <iostream>
#include <scene_io.h>
#include "renderer.hpp"
#include "scene.hpp"

//#define BVH_DISABLED
#define THREADS_DISABLED

int main(int argc, const char** argv)
{
    auto begin = std::chrono::system_clock::now();

    std::string scene_path = "../Scenes/HW3/test3.ascii";
    if (argc > 1) scene_path = std::string(argv[1]);

    rtr::scene scene(scene_path);
    
    std::cout << "Scene loaded!!\n";
    
    auto end = std::chrono::system_clock::now();
    std::cerr << "Scene loading took : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " millisecs.";

    begin = std::chrono::system_clock::now();
    auto width = 1500;
    auto height = 1500;
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

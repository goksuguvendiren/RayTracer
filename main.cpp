#include <iostream>
#include <scene_io.h>
#include "renderer.hpp"
#include "scene.hpp"

int main(int argc, const char** argv)
{
    auto begin = std::chrono::system_clock::now();

    std::string scene_path = "../../Scenes/test4.ascii";
    if (argc > 1) scene_path = std::string(argv[1]);

    rtr::scene scene(scene_path);
//    rtr::scene scene(readScene(scene_path.c_str()));

    rtr::renderer r(400, 400);
    r.render(scene);

    auto end = std::chrono::system_clock::now();
    std::cout << "Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " millisecs.";
    return 0;
}

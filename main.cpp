#include <iostream>
#include <scene_io.h>
#include "renderer.hpp"
#include "scene.hpp"

static rtr::scene loadScene(const std::string& name)
{
    /* load the scene into the SceneIO data structure using given parsing code */
//    auto* ptr = readScene(name);
//    auto scene_io = std::make_unique<SceneIO>(ptr);

    /* hint: use the Visual Studio debugger ("watch" feature) to probe the
       scene data structure and learn more about it for each of the given scenes */

    rtr::scene scene(readScene(name.c_str()));

    /* write any code to transfer from the scene data structure to your own here */
    /* */

    return scene;
}

int main(int argc, const char** argv)
{
    auto begin = std::chrono::system_clock::now();
    std::cout << "Hello, World!" << std::endl;

    std::string scene_path = "../Scenes/test1.ascii";
    if (argc > 1) scene_path = std::string(argv[1]);

    auto scene = loadScene(scene_path);

    rtr::renderer r(400, 400);
    r.render(scene);

    auto end = std::chrono::system_clock::now();
    std::cout << "Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " millisecs.";
    return 0;
}
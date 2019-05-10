#include <iostream>
#include <scene_io.h>
#include "renderer.hpp"
#include "scene.hpp"

static rtr::scene loadScene(char *name)
{
    /* load the scene into the SceneIO data structure using given parsing code */
//    auto* ptr = readScene(name);
//    auto scene_io = std::make_unique<SceneIO>(ptr);

    /* hint: use the Visual Studio debugger ("watch" feature) to probe the
       scene data structure and learn more about it for each of the given scenes */

    rtr::scene scene(readScene(name));

    /* write any code to transfer from the scene data structure to your own here */
    /* */

    return scene;
}

int main()
{
    auto begin = std::chrono::system_clock::now();
    std::cout << "Hello, World!" << std::endl;

    auto scene = loadScene("../Scenes/test1.ascii");

    rtr::renderer r(400, 400);
    r.render(scene);

    auto end = std::chrono::system_clock::now();
    std::cout << "Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " millisecs.";
    return 0;
}
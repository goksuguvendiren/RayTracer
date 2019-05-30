#include <payload.hpp>
#include <shaders/shader.hpp>

#include <glm/vec3.hpp>
#include <opencv2/opencv.hpp>

bool rtr::shaders::Checkerboard(const rtr::payload& payload)
{
	bool u_white = (int(payload.texture_coords.x * 20) % 2) == 1;
    bool v_white = (int(payload.texture_coords.y * 10) % 2) == 1;

    auto val = u_white ^ v_white;
    return val;
}

static glm::vec3 to_vec3(const cv::Vec3b& color)
{
    auto x = color[0] / 255.f;
    auto y = color[1] / 255.f;
    auto z = color[2] / 255.f;
    
    return glm::vec3{z, y, x};
}

rtr::material rtr::shaders::EarthTexture(const rtr::payload& payload, const rtr::material* mat)
{
    static auto image = cv::imread("../src/shaders/earth.jpg");
//    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
    rtr::material m(*mat);
    
    auto uv = payload.texture_coords;
    
    auto i = (1 -uv.y) * image.rows;
    auto j = uv.x * image.cols;
    auto diff = to_vec3(image.at<cv::Vec3b>(i, j));
    m.diffuse = diff;
    return m;
}

# pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

const uint32_t WIDTH = 1920;
const uint32_t HEIGHT = 1080;

class VestaWindow {
public:
    VestaWindow(int w, int h, std::string name);
    ~VestaWindow();
private:
};

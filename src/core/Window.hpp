# pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

int WIDTH = 800;
int HEIGHT = 800;

class Window {
public:
    Window(int w, int h, std::string title);
    ~Window();

    Window(const Window&)            = delete;
    Window& operator=(const Window&) = delete;

    bool shouldClose() const;
    void pollEvents() const;
    GLFWwindow* handle() const { return window_; }
private:
    GLFWwindow* window_;
    int width_;
    int height_;
    std::string title_;
};

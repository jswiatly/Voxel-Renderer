# pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

class Window {
public:
    Window(int w, int h, std::string title);
    ~Window();

    Window(const Window&)            = delete;
    Window& operator=(const Window&) = delete;

    bool shouldClose() const { return glfwWindowShouldClose(window_); }
    void pollEvents() const { glfwPollEvents(); }
    GLFWwindow* handle() const { return window_; }

    bool wasResized() const { return framebufferResized_; }
    void resetResizeFlag() { framebufferResized_ = false; }
    void getFramebufferSize(int& w, int& h) const { glfwGetFramebufferSize(window_, &w, &h); }

private:
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow* window_;
    int width_;
    int height_;
    std::string title_;
    bool framebufferResized_ = false;
};

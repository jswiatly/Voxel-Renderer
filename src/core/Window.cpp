#include "core/Window.hpp"

Window::Window(int w, int h, std::string title) : width_(w), height_(h), title_(std::move(title)) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window_ = glfwCreateWindow(width_, height_, title_.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);
}

Window::~Window() {
    glfwDestroyWindow(window_);
    glfwTerminate();
}

void Window::framebufferResizeCallback(GLFWwindow* window, int /*w*/, int /*h*/) {
    auto self = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    self->framebufferResized_ = true;
}
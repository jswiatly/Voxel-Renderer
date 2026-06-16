#pragma once

struct GLFWwindow;
class Camera;

class InputHandler {
  public:
    void init(GLFWwindow* window);
    void process(GLFWwindow* window, Camera& camera, float dt);

  private:
    float m_lastX = 400.0f;
    float m_lastY = 300.0f;
    bool m_firstMouse = true;
    bool m_cursorMode = false;
    bool m_fKeyWasPressed = false;
};
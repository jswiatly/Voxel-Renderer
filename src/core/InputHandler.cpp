#include "core/InputHandler.hpp"
#include "scene/Camera.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>

void InputHandler::init(GLFWwindow* window) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void InputHandler::process(GLFWwindow* window, Camera& camera, float dt) {
    bool fKeyPressed = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
    if (fKeyPressed && !m_fKeyWasPressed) {
        m_cursorMode = !m_cursorMode;
        glfwSetInputMode(window, GLFW_CURSOR, m_cursorMode ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        if (!m_cursorMode)
            m_firstMouse = true;
    }
    m_fKeyWasPressed = fKeyPressed;

    if (!m_cursorMode) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.processKeyboard(0, dt);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.processKeyboard(1, dt);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.processKeyboard(2, dt);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.processKeyboard(3, dt);

        if (!ImGui::GetIO().WantCaptureMouse) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            if (m_firstMouse) {
                m_lastX = static_cast<float>(xpos);
                m_lastY = static_cast<float>(ypos);
                m_firstMouse = false;
            }
            float xoffset = static_cast<float>(xpos) - m_lastX;
            float yoffset = m_lastY - static_cast<float>(ypos);
            m_lastX = static_cast<float>(xpos);
            m_lastY = static_cast<float>(ypos);
            camera.processMouseMovement(xoffset, yoffset);
        }
    }
}
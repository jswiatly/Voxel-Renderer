#include "core/InputHandler.hpp"
#include "scene/Camera.hpp"
#include "scene/PlayerController.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>

void InputHandler::init(GLFWwindow* window) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void InputHandler::process(GLFWwindow* window, Camera& camera, PlayerController& player, float dt) {
    bool fKeyPressed = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
    if (fKeyPressed && !m_fKeyWasPressed) {
        m_cursorMode = !m_cursorMode;
        glfwSetInputMode(window, GLFW_CURSOR, m_cursorMode ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        if (!m_cursorMode)
            m_firstMouse = true;
    }
    m_fKeyWasPressed = fKeyPressed;

    bool f5Pressed = glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS;
    if (f5Pressed && !m_f5WasPressed) {
        camera.thirdPerson = !camera.thirdPerson;
    }
    m_f5WasPressed = f5Pressed;

    glm::vec3 forward = camera.front;
    forward.y = 0.0f;
    forward = glm::normalize(forward);

    glm::vec3 right = glm::normalize(glm::cross(forward, camera.up));

    if (!m_cursorMode) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            player.move(forward, dt);

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            player.move(-forward, dt);

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            player.move(-right, dt);

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            player.move(right, dt);

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
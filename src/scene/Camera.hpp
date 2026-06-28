#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
  public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;

    bool thirdPerson = false;
    float thirdPersonDistance = 5.0f;

    float yaw;
    float pitch;
    float movementSpeed;
    float mouseSensitivity;

    Camera(glm::vec3 startPos = glm::vec3(0.0f, 0.0f, 3.0f)) {
        position = startPos;
        front = glm::vec3(0.0f, 0.0f, -1.0f);
        up = glm::vec3(0.0f, 1.0f, 0.0f);
        yaw = -90.0f;
        pitch = 0.0f;
        movementSpeed = 15.f;
        mouseSensitivity = 0.1f;
    }
    glm::mat4 getViewMatrix() const {
        glm::vec3 eye = thirdPerson ? position - front * thirdPersonDistance : position;
        return glm::lookAt(eye, eye + front, up);
    }

    void processKeyboard(int direction, float deltaTime) {
        float velocity = movementSpeed * deltaTime;
        if (direction == 0)
            position += front * velocity; // W
        if (direction == 1)
            position -= front * velocity; // S
        if (direction == 2)
            position -= glm::normalize(glm::cross(front, up)) * velocity; // A
        if (direction == 3)
            position += glm::normalize(glm::cross(front, up)) * velocity; // D
    }

    void processMouseMovement(float xoffset, float yoffset) {
        xoffset *= mouseSensitivity;
        yoffset *= mouseSensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        updateCameraVectors();
    }

  private:
    void updateCameraVectors() {
        glm::vec3 newFront;
        newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        newFront.y = sin(glm::radians(pitch));
        newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(newFront);
    }
};
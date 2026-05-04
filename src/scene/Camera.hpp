#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
    public:
        glm::vec3 position;
        glm::vec3 front;
        glm::vec3 up;

        float yaw;
        float pitch;
        float movementSpeed;
        float mouseSensitivity;

        Camera(glm::vec3 startPos = glm::vec3(0.0f, 0.0f, 3.0f)){
            position = startPos;
            front = glm::vec3(0.0f, 0.0f, -1.0f);
            up = glm::vec3(0.0f, 1.0f, 0.0f);
            yaw = -90.0f;
            pitch = 0.0f;
            movementSpeed = 2.5f;
            mouseSensitivity = 0.1f;
        }

        void processKeyboard(){

        }

        void processMouse(){

        }
};
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

        }


};
#pragma once

#include <glm/glm.hpp>

class World;

class PlayerController {
  public:
    PlayerController(glm::vec3 position);

    void update(float dt, const World& world);

    void move(glm::vec3 direction, float dt);

    glm::vec3 getPosition() const;

  private:
    glm::vec3 m_position;
    float m_movementSpeed = 15.0f;
    glm::vec3 m_velocity{0.0f};

    glm::vec3 m_halfSize{0.3f, 0.9f, 0.3f};

    bool m_grounded = false;
};
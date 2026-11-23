#pragma once

#include <glm/glm.hpp>

class World;

class Entity {
  public:
    explicit Entity(glm::vec3 position);

    void update(float dt, const World& world);

    glm::vec3 getPosition() const;

  private:
    glm::vec3 m_position;
    glm::vec3 m_velocity{0.0f};
    glm::vec3 m_walkDirection{1.0f, 0.0f, 0.0f};

    glm::vec3 m_halfSize{0.3f, 0.9f, 0.3f};

    float m_speed = 3.0f;
    float m_changeDirectionIn = 0.0f;
    bool m_grounded = false;
};
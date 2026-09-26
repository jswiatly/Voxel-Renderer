#include "scene/PlayerController.hpp"
#include "scene/World.hpp"

#include <cmath>

PlayerController::PlayerController(glm::vec3 position) : m_position(position) {}

glm::vec3 PlayerController::getPosition() const {
    return m_position;
}

void PlayerController::update(float dt, const World& world) {
    if (!m_grounded)
        m_velocity.y -= 9.81f * dt;

    m_position += m_velocity * dt;

    float bottom = m_position.y - m_halfSize.y;

    int blockX = static_cast<int>(std::floor(m_position.x));
    int blockY = static_cast<int>(std::floor(bottom - 0.001f));
    int blockZ = static_cast<int>(std::floor(m_position.z));

    if (m_velocity.y < 0.0f && world.isSolid(blockX, blockY, blockZ)) {
        m_position.y = blockY + 1.0f + m_halfSize.y;
        m_velocity.y = 0.0f;
        m_grounded = true;
    } else {
        m_grounded = false;
    }
}

void PlayerController::move(glm::vec3 direction, float dt) {
    m_position += direction * m_movementSpeed * dt;
}
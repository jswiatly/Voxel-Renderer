#include "scene/PlayerController.hpp"

PlayerController::PlayerController(glm::vec3 position) : m_position(position) {}

glm::vec3 PlayerController::getPosition() const {
    return m_position;
}

void PlayerController::update(float dt, const World& world) {}
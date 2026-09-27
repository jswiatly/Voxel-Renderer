#include "scene/PlayerController.hpp"
#include "scene/World.hpp"

#include <algorithm>
#include <cmath>

namespace {
// Must match MIN_Y in World.hpp / Terrain.cpp: the lowest real voxel layer.
// Below that, World::getBlock() always reports solid "bedrock" (as a safety
// fallback so out-of-range reads don't fall through). If the player ever
// tunnels past it in a single big step, blockY can be an arbitrarily large
// negative number — we must never snap the landing position to that value,
// only ever to the real bedrock surface at worst.
constexpr float WORLD_MIN_Y = -32.0f;
} // namespace

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
        const float landingY = std::max(static_cast<float>(blockY) + 1.0f, WORLD_MIN_Y + 1.0f);
        m_position.y = landingY + m_halfSize.y;
        m_velocity.y = 0.0f;
        m_grounded = true;
    } else {
        m_grounded = false;
    }
}

void PlayerController::move(glm::vec3 direction, float dt) {
    m_position += direction * m_movementSpeed * dt;
}
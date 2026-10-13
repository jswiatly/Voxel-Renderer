#include "scene/PlayerController.hpp"
#include "scene/World.hpp"

#include <algorithm>
#include <cmath>

// Must match MIN_Y in World.hpp / Terrain.cpp: the lowest real voxel layer.
// Below that, World::getBlock() always reports solid "bedrock" (as a safety
// fallback so out-of-range reads don't fall through). If the player ever
// tunnels past it in a single big step, blockY can be an arbitrarily large
// negative number — we must never snap the landing position to that value,
// only ever to the real bedrock surface at worst.

namespace {
constexpr float WORLD_MIN_Y = -32.0f;
bool collidesAt(const World& world, const glm::vec3& position, const glm::vec3& halfSize) {
    const int minX = static_cast<int>(std::floor(position.x - halfSize.x + 0.001f));
    const int maxX = static_cast<int>(std::floor(position.x + halfSize.x - 0.001f));

    const int minY = static_cast<int>(std::floor(position.y - halfSize.y + 0.001f));
    const int maxY = static_cast<int>(std::floor(position.y + halfSize.y - 0.001f));

    const int minZ = static_cast<int>(std::floor(position.z - halfSize.z + 0.001f));
    const int maxZ = static_cast<int>(std::floor(position.z + halfSize.z - 0.001f));

    for (int x = minX; x <= maxX; ++x)
        for (int y = minY; y <= maxY; ++y)
            for (int z = minZ; z <= maxZ; ++z)
                if (world.isSolid(x, y, z))
                    return true;

    return false;
}
} // namespace

PlayerController::PlayerController(glm::vec3 position) : m_position(position) {}

glm::vec3 PlayerController::getPosition() const {
    return m_position;
}
/*
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
*/

void PlayerController::update(float dt, const World& world) {
    for (int axis : {0, 2}) {
        glm::vec3 candidate = m_position;
        candidate[axis] += m_pendingMovement[axis];

        if (!collidesAt(world, candidate, m_halfSize))
            m_position = candidate;
    }

    m_pendingMovement = glm::vec3(0.0f);

    m_velocity.y -= 9.81f * dt;

    glm::vec3 candidate = m_position;
    candidate.y += m_velocity.y * dt;

    if (collidesAt(world, candidate, m_halfSize)) {
        if (m_velocity.y < 0.0f)
            m_grounded = true;

        m_velocity.y = 0.0f;
    } else {
        m_position = candidate;
        m_grounded = false;
    }
}

void PlayerController::move(glm::vec3 direction, float dt) {
    m_pendingMovement += direction * m_movementSpeed * dt;
}

void PlayerController::jump() {
    if (m_grounded) {
        m_velocity.y = 7.0f;
        m_grounded = false;
    }
}
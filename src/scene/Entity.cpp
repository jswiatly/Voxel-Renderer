#include "scene/Entity.hpp"
#include "scene/World.hpp"

#include <cmath>
#include <random>

namespace {
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
Entity::Entity(glm::vec3 position) : m_position(position) {}

void Entity::update(float dt, const World& world) {
    m_changeDirectionIn -= dt;

    if (m_changeDirectionIn <= 0.0f) {
        static std::mt19937 random{std::random_device{}()};
        static std::uniform_real_distribution<float> directionDistribution(-1.0f, 1.0f);
        static std::uniform_real_distribution<float> durationDistribution(1.0f, 3.0f);

        m_walkDirection = {directionDistribution(random), 0.0f, directionDistribution(random)};
        if (glm::length(m_walkDirection) < 0.01f)
            m_walkDirection = {1.0f, 0.0f, 0.0f};
        else
            m_walkDirection = glm::normalize(m_walkDirection);

        m_changeDirectionIn = durationDistribution(random);
    }

    for (int axis : {0, 2}) {
        glm::vec3 candidate = m_position;
        candidate[axis] += m_walkDirection[axis] * m_speed * dt;

        if (collidesAt(world, candidate, m_halfSize)) {
            m_changeDirectionIn = 0.0f;
            continue;
        }

        m_position = candidate;
    }

    m_velocity.y -= 9.81f * dt;
    glm::vec3 candidate = m_position;
    candidate.y += m_velocity.y * dt;

    if (collidesAt(world, candidate, m_halfSize)) {
        m_grounded = m_velocity.y < 0.0f;
        m_velocity.y = 0.0f;
    } else {
        m_position = candidate;
        m_grounded = false;
    }
}

glm::vec3 Entity::getPosition() const {
    return m_position;
}

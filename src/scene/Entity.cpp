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
    }
}

glm::vec3 Entity::getPosition() const {
    return m_position;
}
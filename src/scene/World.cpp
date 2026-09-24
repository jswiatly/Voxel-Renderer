#include "World.hpp"

#include <cassert>

World::World(int size)
    : m_size(size), m_half(size / 2), m_voxelMap(static_cast<size_t>(size) * static_cast<size_t>(size) * RANGE_Y, 0) {}

size_t World::getIndex(int x, int y, int z) const {
    int gx = x + m_half;
    int gz = z + m_half;

    assert(gx >= 0 && gx < m_size);
    assert(gz >= 0 && gz < m_size);
    assert(y >= MIN_Y && y <= MAX_Y);

    return (static_cast<size_t>(gx) * m_size + gz) * RANGE_Y + (y - MIN_Y);
}

uint8_t World::getBlock(int x, int y, int z) const {
    if (y < MIN_Y)
        return 2; // BLOCK_STONE

    if (y > MAX_Y)
        return 0; // BLOCK_AIR

    int gx = x + m_half;
    int gz = z + m_half;

    if (gx < 0 || gx >= m_size || gz < 0 || gz >= m_size) {
        return 0; // BLOCK_AIR
    }

    return m_voxelMap[getIndex(x, y, z)];
}
bool World::isSolid(int x, int y, int z) const {
    uint8_t block = getBlock(x, y, z);

    return block != 0; // BLOCK_AIR
}

void World::setBlock(int x, int y, int z, uint8_t block) {
    if (y < MIN_Y || y > MAX_Y)
        return;

    int gx = x + m_half;
    int gz = z + m_half;

    if (gx < 0 || gx >= m_size || gz < 0 || gz >= m_size) {
        return;
    }

    m_voxelMap[getIndex(x, y, z)] = block;
}
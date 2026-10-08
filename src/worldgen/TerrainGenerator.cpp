#include "worldgen/TerrainGenerator.hpp"
#include "scene/World.hpp"
#include "scene/Block.hpp"

#include <cmath>
#include <algorithm>
#include <array>

#include <glm/glm.hpp>

TerrainGenerator::TerrainGenerator(const WorldGenSettings& settings)
    : m_settings(settings), m_lowNoise(settings.seed + 1000, 4), m_highNoise(settings.seed + 2000, 4) {}

namespace {

float terrainDensity(const OctavePerlin& lowNoise, const OctavePerlin& highNoise, float x, float y, float z) {
    const double low = lowNoise.sample3D(x * 0.008, y * 0.012, z * 0.008);

    const double high = highNoise.sample3D(x * 0.025, y * 0.025, z * 0.025);

    const float terrainHeight = 64.0f + static_cast<float>(low) * 35.0f;

    const float vertical = terrainHeight - y;

    const float detail = static_cast<float>(high) * 8.0f;

    return vertical + detail;
}

} // namespace

TerrainMetaData TerrainGenerator::generateBase(World& world) const {
    TerrainMetaData metadata;

    metadata.size = m_settings.worldSize;
    metadata.half = metadata.size / 2;

    const size_t columnCount = static_cast<size_t>(metadata.size) * metadata.size;

    metadata.columnTopY.resize(columnCount);
    metadata.biome.resize(columnCount);
    metadata.jitter.resize(columnCount);
    metadata.slope.resize(columnCount);
    metadata.shore.resize(columnCount);

    const int size = metadata.size;
    const int half = metadata.half;

    for (int gx = 0; gx < size; ++gx) {
        for (int gz = 0; gz < size; ++gz) {
            const int x = gx - half;
            const int z = gz - half;

            int topY = World::MIN_Y;

            for (int y = World::MIN_Y; y <= World::MAX_Y; ++y) {
                const float density = terrainDensity(m_lowNoise, m_highNoise, static_cast<float>(x),
                                                     static_cast<float>(y), static_cast<float>(z));

                if (density > 0.0f) {
                    world.setBlock(x, y, z, static_cast<uint8_t>(Block::Stone));

                    topY = y;
                }
            }

            metadata.columnTopY[gx * size + gz] = topY;

            // Na razie neutralne wartości.
            metadata.biome[gx * size + gz] = 0.5f;
            metadata.jitter[gx * size + gz] = 0.5f;
            metadata.slope[gx * size + gz] = 0;
            metadata.shore[gx * size + gz] = 0;
        }
    }

    return metadata;
}
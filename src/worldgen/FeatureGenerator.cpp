#include "FeatureGenerator.hpp"
#include "scene/World.hpp"
#include "scene/Block.hpp"
#include <glm/glm.hpp>
#include <cmath>

namespace {
constexpr int CANOPY_BOTTOM = 2;
constexpr int CANOPY_TOP = 1;
} // namespace

FeatureGenerator::FeatureGenerator(const WorldGenSettings& settings) : m_settings(settings) {}

void FeatureGenerator::generateTrees(World& world, const TerrainMetaData& metadata) const {
    auto growsGrass = [&](int gx, int gz) {
        const int idx = gx * metadata.size + gz;
        const int h = metadata.columnTopY[idx];
        if (h <= m_settings.seaLevel || metadata.slope[idx] >= m_settings.cliffSlope)
            return false;
        if (metadata.shore[idx] != 0 && h <= m_settings.seaLevel + m_settings.beachHeight)
            return false;
        if (metadata.biome[idx] > m_settings.desertThreshold)
            return false;
        return metadata.jitter[idx] >= glm::smoothstep(m_settings.alpineStart, m_settings.alpineEnd, float(h));
    };

    auto plant = [&](int gx, int gz, int y, uint8_t block) {
        if (gx < 0 || gx >= metadata.size || gz < 0 || gz >= metadata.size || y < World::MIN_Y || y > World::MAX_Y)
            return;

        if (world.getBlock(gx - metadata.half, y, gz - metadata.half) == static_cast<uint8_t>(Block::Air))
            world.setBlock(gx - metadata.half, y, gz - metadata.half, block);
    };

    const uint32_t seedHash = static_cast<uint32_t>(m_settings.seed) * 0x9E3779B9u;

    auto hash = [seedHash](int32_t a, int32_t b) {
        uint32_t h = static_cast<uint32_t>(a) * 0x8DA6B343u ^ static_cast<uint32_t>(b) * 0xD8163841u ^ seedHash;
        h ^= h >> 15;
        h *= 0x2C1B3C6Du;
        h ^= h >> 12;
        h *= 0x297A2D39u;
        h ^= h >> 15;
        return float(h) * (1.0f / 4294967296.0f);
    };

    auto noise = [&hash](float x, float z) {
        float xi = std::floor(x), zi = std::floor(z);
        int32_t ix = static_cast<int32_t>(xi), iz = static_cast<int32_t>(zi);
        float xf = x - xi, zf = z - zi;
        float u = xf * xf * (3.f - 2.f * xf);
        float v = zf * zf * (3.f - 2.f * zf);
        return glm::mix(glm::mix(hash(ix, iz), hash(ix + 1, iz), u),
                        glm::mix(hash(ix, iz + 1), hash(ix + 1, iz + 1), u), v);
    };

    constexpr float ROT_C = 0.8572f;
    constexpr float ROT_S = 0.5150f;

    auto octaves = [&noise](float x, float z, bool ridged) {
        float t = 0.f, a = 1.f, n = 0.f;
        for (int i = 0; i < 6; ++i) {
            float v = noise(x, z);
            t += (ridged ? 1.f - std::abs(v - 0.5f) * 2.f : v) * a;
            n += a;
            a *= 0.5f;
            float rx = (x * ROT_C - z * ROT_S) * 2.0f;
            z = (x * ROT_S + z * ROT_C) * 2.0f;
            x = rx;
        }
        return t / n;
    };

    auto fbm = [&octaves](float x, float z) { return octaves(x, z, false); };

    for (int cellX = 0; cellX * m_settings.treeCell < metadata.size; ++cellX) {
        for (int cellZ = 0; cellZ * m_settings.treeCell < metadata.size; ++cellZ) {
            const int gx = cellX * m_settings.treeCell + int(hash(cellX + 10007, cellZ + 20011) * m_settings.treeCell);
            const int gz = cellZ * m_settings.treeCell + int(hash(cellX + 30011, cellZ + 40013) * m_settings.treeCell);
            if (gx >= metadata.size || gz >= metadata.size || !growsGrass(gx, gz))
                continue;

            const float forest =
                fbm((float(gx - metadata.half) + 4000.f) * 0.01f, (float(gz - metadata.half) - 4000.f) * 0.01f);
            if (hash(cellX + 50021, cellZ + 60029) > glm::smoothstep(m_settings.forestLo, m_settings.forestHi, forest))
                continue;

            const int base = metadata.columnTopY[gx * metadata.size + gz] + 1;
            const int top =
                base + m_settings.trunkMin + int(hash(cellX + 70039, cellZ + 80051) * m_settings.trunkVar) - 1;
            if (top + CANOPY_TOP > World::MAX_Y)
                continue;

            for (int y = base; y <= top; ++y)
                plant(gx, gz, y, static_cast<uint8_t>(Block::Wood));

            for (int dy = -CANOPY_BOTTOM; dy <= CANOPY_TOP; ++dy) {
                const int radius = (dy < 0) ? 2 : 1;
                for (int dx = -radius; dx <= radius; ++dx) {
                    for (int dz = -radius; dz <= radius; ++dz) {
                        const bool corner = std::abs(dx) == radius && std::abs(dz) == radius;
                        if (corner && (dy == CANOPY_TOP || hash(gx + dx * 31, gz + dz * 17) < 0.55f))
                            continue;
                        plant(gx + dx, gz + dz, top + dy, static_cast<uint8_t>(Block::Leaves));
                    }
                }
            }
        }
    }
}
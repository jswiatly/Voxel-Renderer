#include "CaveGenerator.hpp"

#include "scene/World.hpp"
#include "scene/Block.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>

namespace {

constexpr int CHUNK_SIZE = 16;
constexpr int SAMPLE_STEP = 4;

// Siatka 5 x 41 x 5 obejmuje chunk 16 x 160 x 16.
constexpr int NODES_X = CHUNK_SIZE / SAMPLE_STEP + 1;
constexpr int NODES_Y = (World::MAX_Y - World::MIN_Y) / SAMPLE_STEP + 1;
constexpr int NODES_Z = NODES_X;

constexpr int CELLS_X = NODES_X - 1;
constexpr int CELLS_Y = NODES_Y - 1;
constexpr int CELLS_Z = NODES_Z - 1;

constexpr int BOTTOM_BUFFER = 4;
constexpr int SURFACE_BUFFER = 5;

// Większy próg = więcej i szersze jaskinie.
constexpr float TUNNEL_THRESHOLD = 0.300f;

using NoiseGrid = std::array<float, NODES_X * NODES_Y * NODES_Z>;

int gridIndex(int x, int y, int z) {
    return (y * NODES_Z + z) * NODES_X + x;
}

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// Interpolacja trójliniowa wartości szumu wewnątrz komórki.
float sampleCell(const NoiseGrid& grid, int cx, int cy, int cz, float tx, float ty, float tz) {
    const auto at = [&](int dx, int dy, int dz) { return grid[gridIndex(cx + dx, cy + dy, cz + dz)]; };

    const float x00 = lerp(at(0, 0, 0), at(1, 0, 0), tx);
    const float x10 = lerp(at(0, 1, 0), at(1, 1, 0), tx);
    const float x01 = lerp(at(0, 0, 1), at(1, 0, 1), tx);
    const float x11 = lerp(at(0, 1, 1), at(1, 1, 1), tx);

    const float y0 = lerp(x00, x10, ty);
    const float y1 = lerp(x01, x11, ty);

    return lerp(y0, y1, tz);
}

// Jeżeli przedział wartości w narożnikach nie przecina
// [-threshold, threshold], wnętrze komórki też go nie osiągnie.
// Możemy więc bezpiecznie pominąć tę komórkę.
bool mayContainTunnel(const NoiseGrid& grid, int cx, int cy, int cz) {
    float minValue = std::numeric_limits<float>::max();
    float maxValue = std::numeric_limits<float>::lowest();

    for (int dy = 0; dy <= 1; ++dy) {
        for (int dz = 0; dz <= 1; ++dz) {
            for (int dx = 0; dx <= 1; ++dx) {
                const float value = grid[gridIndex(cx + dx, cy + dy, cz + dz)];

                minValue = std::min(minValue, value);
                maxValue = std::max(maxValue, value);
            }
        }
    }

    return minValue <= TUNNEL_THRESHOLD && maxValue >= -TUNNEL_THRESHOLD;
}

} // namespace

CaveGenerator::CaveGenerator(const WorldGenSettings& settings)
    : m_tunnelA(static_cast<uint64_t>(static_cast<uint32_t>(settings.seed)) + 0xCA7E1234ULL, 2),
      m_tunnelB(static_cast<uint64_t>(static_cast<uint32_t>(settings.seed)) + 0xBADC0FFEEULL, 2) {}

void CaveGenerator::generate(World& world, const TerrainMetaData& metadata) const {
    const int chunksPerAxis = (metadata.size + CHUNK_SIZE - 1) / CHUNK_SIZE;

    for (int chunkX = 0; chunkX < chunksPerAxis; ++chunkX) {
        for (int chunkZ = 0; chunkZ < chunksPerAxis; ++chunkZ) {
            const int baseGX = chunkX * CHUNK_SIZE;
            const int baseGZ = chunkZ * CHUNK_SIZE;

            NoiseGrid noiseA{};
            NoiseGrid noiseB{};

            // 1. Próbkujemy szum tylko na rzadkiej siatce.
            for (int ny = 0; ny < NODES_Y; ++ny) {
                const int worldY = World::MIN_Y + ny * SAMPLE_STEP;

                for (int nz = 0; nz < NODES_Z; ++nz) {
                    const int gz = baseGZ + nz * SAMPLE_STEP;
                    const float z = static_cast<float>(gz - metadata.half);

                    for (int nx = 0; nx < NODES_X; ++nx) {
                        const int gx = baseGX + nx * SAMPLE_STEP;
                        const float x = static_cast<float>(gx - metadata.half);

                        const int index = gridIndex(nx, ny, nz);
                        const float y = static_cast<float>(worldY);

                        noiseA[index] = static_cast<float>(m_tunnelA.sample3D(x * 0.035, y * 0.050, z * 0.035));

                        noiseB[index] = static_cast<float>(m_tunnelB.sample3D(x * 0.045, y * 0.040, z * 0.045));
                    }
                }
            }

            // 2. Sprawdzamy poszczególne komórki siatki.
            for (int cellZ = 0; cellZ < CELLS_Z; ++cellZ) {
                for (int cellX = 0; cellX < CELLS_X; ++cellX) {
                    // Cache wysokości powierzchni dla 4 x 4 kolumn.
                    std::array<std::array<int, SAMPLE_STEP>, SAMPLE_STEP> topY{};

                    std::array<std::array<bool, SAMPLE_STEP>, SAMPLE_STEP> valid{};

                    for (int dx = 0; dx < SAMPLE_STEP; ++dx) {
                        for (int dz = 0; dz < SAMPLE_STEP; ++dz) {
                            const int gx = baseGX + cellX * SAMPLE_STEP + dx;
                            const int gz = baseGZ + cellZ * SAMPLE_STEP + dz;

                            valid[dx][dz] = gx < metadata.size && gz < metadata.size;

                            if (valid[dx][dz]) {
                                topY[dx][dz] = metadata.columnTopY[metadata.index(gx, gz)];
                            }
                        }
                    }

                    // 3. Pomijamy komórki, które nie mogą
                    // zawierać tunelu.
                    for (int cellY = 0; cellY < CELLS_Y; ++cellY) {
                        if (!mayContainTunnel(noiseA, cellX, cellY, cellZ) ||
                            !mayContainTunnel(noiseB, cellX, cellY, cellZ)) {
                            continue;
                        }

                        // 4. Sprawdzamy bloki tylko w kandydujących
                        // komórkach.
                        for (int dy = 0; dy < SAMPLE_STEP; ++dy) {
                            const int worldY = World::MIN_Y + cellY * SAMPLE_STEP + dy;

                            if (worldY < World::MIN_Y + BOTTOM_BUFFER) {
                                continue;
                            }

                            for (int dx = 0; dx < SAMPLE_STEP; ++dx) {
                                for (int dz = 0; dz < SAMPLE_STEP; ++dz) {
                                    if (!valid[dx][dz])
                                        continue;

                                    const int gx = baseGX + cellX * SAMPLE_STEP + dx;
                                    const int gz = baseGZ + cellZ * SAMPLE_STEP + dz;

                                    // Zostawiamy warstwę skały
                                    // chroniącą powierzchnię.
                                    if (worldY > topY[dx][dz] - SURFACE_BUFFER) {
                                        continue;
                                    }

                                    const float tx = static_cast<float>(dx) / SAMPLE_STEP;
                                    const float ty = static_cast<float>(dy) / SAMPLE_STEP;
                                    const float tz = static_cast<float>(dz) / SAMPLE_STEP;

                                    const float a = sampleCell(noiseA, cellX, cellY, cellZ, tx, ty, tz);

                                    const float b = sampleCell(noiseB, cellX, cellY, cellZ, tx, ty, tz);

                                    // Przecięcie dwóch pól 3D
                                    // tworzy wijące się tunele.
                                    if (std::abs(a) >= TUNNEL_THRESHOLD || std::abs(b) >= TUNNEL_THRESHOLD) {
                                        continue;
                                    }

                                    const int worldX = gx - metadata.half;
                                    const int worldZ = gz - metadata.half;

                                    // Usuwamy tylko skałę.
                                    if (world.getBlock(worldX, worldY, worldZ) == static_cast<uint8_t>(Block::Stone)) {
                                        world.setBlock(worldX, worldY, worldZ, static_cast<uint8_t>(Block::Air));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
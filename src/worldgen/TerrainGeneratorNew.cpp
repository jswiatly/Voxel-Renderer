#include "worldgen/TerrainGenerator.hpp"
#include "scene/World.hpp"
#include "scene/Block.hpp"

#include <array>
#include <algorithm>
#include <cmath>

constexpr int GENERATION_CHUNK_SIZE_X = 16;
constexpr int GENERATION_CHUNK_SIZE_Z = 16;

namespace {

constexpr int DENSITY_STEP_X = 4;
constexpr int DENSITY_STEP_Y = 16;
constexpr int DENSITY_STEP_Z = 4;

constexpr int NODES_X = 5;
constexpr int NODES_Y = (World::MAX_Y - World::MIN_Y) / DENSITY_STEP_Y + 1;
constexpr int NODES_Z = 5;

constexpr int CELLS_X = NODES_X - 1;
constexpr int CELLS_Y = NODES_Y - 1;
constexpr int CELLS_Z = NODES_Z - 1;

using DensityGrid = std::array<float, NODES_X * NODES_Y * NODES_Z>;

inline int densityIndex(int x, int y, int z) {
    return (y * NODES_Z + z) * NODES_X + x;
}

float terrainDensity(const OctavePerlin& lowNoise, const OctavePerlin& highNoise, float x, float y, float z) {
    const double low = lowNoise.sample2D(x * 0.008, z * 0.008);

    const double high = highNoise.sample2D(x * 0.025, z * 0.025);

    const float terrainHeight = 64.0f + static_cast<float>(low) * 50.0f + static_cast<float>(high) * 10.0f;

    return terrainHeight - y;
}

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

float sampleInterpolatedDensity(const DensityGrid& grid, int localX, int localY, int localZ) {
    /*
        localX: 0..15
        localY: 0..160
        localZ: 0..15

        Znajdujemy komórkę 4×16×4,
        w której znajduje się voxel.
    */

    int cellX = localX / DENSITY_STEP_X;
    int cellY = localY / DENSITY_STEP_Y;
    int cellZ = localZ / DENSITY_STEP_Z;

    /*
        Dla górnej granicy Y = 160
        wychodziłoby cellY == 10.

        Ostatnia komórka ma jednak indeks 9,
        a punkt na jej końcu ma t = 1.
    */
    cellX = std::min(cellX, CELLS_X - 1);
    cellY = std::min(cellY, CELLS_Y - 1);
    cellZ = std::min(cellZ, CELLS_Z - 1);

    const float tx = float(localX - cellX * DENSITY_STEP_X) / float(DENSITY_STEP_X);

    const float ty = float(localY - cellY * DENSITY_STEP_Y) / float(DENSITY_STEP_Y);

    const float tz = float(localZ - cellZ * DENSITY_STEP_Z) / float(DENSITY_STEP_Z);

    /*
        8 narożników naszej komórki:

             c010 -------- c110
              /              /
             /              /
         c000 -------- c100

         oraz identyczna ściana z = 1.
    */

    const float c000 = grid[densityIndex(cellX, cellY, cellZ)];

    const float c100 = grid[densityIndex(cellX + 1, cellY, cellZ)];

    const float c010 = grid[densityIndex(cellX, cellY + 1, cellZ)];

    const float c110 = grid[densityIndex(cellX + 1, cellY + 1, cellZ)];

    const float c001 = grid[densityIndex(cellX, cellY, cellZ + 1)];

    const float c101 = grid[densityIndex(cellX + 1, cellY, cellZ + 1)];

    const float c011 = grid[densityIndex(cellX, cellY + 1, cellZ + 1)];

    const float c111 = grid[densityIndex(cellX + 1, cellY + 1, cellZ + 1)];

    /*
        Najpierw interpolacja X.
    */

    const float x00 = lerp(c000, c100, tx);
    const float x10 = lerp(c010, c110, tx);
    const float x01 = lerp(c001, c101, tx);
    const float x11 = lerp(c011, c111, tx);

    /*
        Potem Y.
    */

    const float y0 = lerp(x00, x10, ty);
    const float y1 = lerp(x01, x11, ty);

    /*
        Na końcu Z.
    */

    return lerp(y0, y1, tz);
}

DensityGrid generateDensityGrid(const OctavePerlin& lowNoise, const OctavePerlin& highNoise, int chunkX, int chunkZ,
                                int worldHalf) {
    DensityGrid grid{};

    /*
        chunkX/chunkZ są indeksami chunka.

        Chunk ma 16×16 bloków.
        Node'y poziomo są co 4 bloki.

        Dlatego potrzebujemy 0,4,8,12,16.
    */

    const int baseX = chunkX * GENERATION_CHUNK_SIZE_X - worldHalf;

    const int baseZ = chunkZ * GENERATION_CHUNK_SIZE_Z - worldHalf;

    for (int ny = 0; ny < NODES_Y; ++ny) {
        const int worldY = World::MIN_Y + ny * DENSITY_STEP_Y;

        for (int nz = 0; nz < NODES_Z; ++nz) {
            const int worldZ = baseZ + nz * DENSITY_STEP_Z;

            for (int nx = 0; nx < NODES_X; ++nx) {
                const int worldX = baseX + nx * DENSITY_STEP_X;

                grid[densityIndex(nx, ny, nz)] = terrainDensity(lowNoise, highNoise, static_cast<float>(worldX),
                                                                static_cast<float>(worldY), static_cast<float>(worldZ));
            }
        }
    }

    return grid;
}

} // namespace

TerrainGenerator::TerrainGenerator(const WorldGenSettings& settings)
    : m_settings(settings), m_lowNoise(settings.seed + 1000, 4), m_highNoise(settings.seed + 2000, 4) {}

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

    const int chunksPerAxis = (size + GENERATION_CHUNK_SIZE_X - 1) / GENERATION_CHUNK_SIZE_X;

    /*
        Generujemy teraz świat chunkami.

        To jeszcze NIE jest streaming świata.
        Po prostu zmieniamy organizację generowania,
        żeby density grid był liczony osobno dla każdego chunka.
    */

    for (int chunkX = 0; chunkX < chunksPerAxis; ++chunkX) {
        for (int chunkZ = 0; chunkZ < chunksPerAxis; ++chunkZ) {
            const DensityGrid density = generateDensityGrid(m_lowNoise, m_highNoise, chunkX, chunkZ, half);

            const int baseGX = chunkX * GENERATION_CHUNK_SIZE_X;

            const int baseGZ = chunkZ * GENERATION_CHUNK_SIZE_Z;

            /*
                Teraz odtwarzamy normalne voxele.

                Noise już NIE jest tutaj wywoływany.

                Robimy tylko interpolację.
            */

            for (int localX = 0; localX < GENERATION_CHUNK_SIZE_X; ++localX) {
                const int gx = baseGX + localX;

                if (gx >= size)
                    continue;

                const int worldX = gx - half;

                for (int localZ = 0; localZ < GENERATION_CHUNK_SIZE_Z; ++localZ) {
                    const int gz = baseGZ + localZ;

                    if (gz >= size)
                        continue;

                    const int worldZ = gz - half;

                    int topY = World::MIN_Y;

                    for (int localY = 0; localY <= World::MAX_Y - World::MIN_Y; ++localY) {
                        const int worldY = World::MIN_Y + localY;

                        const float densityValue = sampleInterpolatedDensity(density, localX, localY, localZ);

                        if (densityValue > 0.0f) {
                            world.setBlock(worldX, worldY, worldZ, static_cast<uint8_t>(Block::Stone));

                            topY = worldY;
                        }
                    }

                    const size_t index = static_cast<size_t>(gx) * size + gz;

                    metadata.columnTopY[index] = topY;

                    // Na razie neutralne wartości.
                    metadata.biome[index] = 0.5f;
                    metadata.jitter[index] = 0.5f;
                    metadata.slope[index] = 0;
                    metadata.shore[index] = 0;
                }
            }
        }
    }

    return metadata;
}
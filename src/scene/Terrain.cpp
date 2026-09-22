#include "scene/Terrain.hpp"

#include <cmath>
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>

#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>

#include "World.hpp"

namespace {

constexpr glm::vec3 FACE_VERTS[6][4] = {
    // FRONT (+Z)
    {{-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}},
    // BACK (-Z)
    {{0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}},
    // LEFT (-X)
    {{-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, -0.5f}},
    // RIGHT (+X)
    {{0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}},
    // BOTTOM (-Y)
    {{-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, 0.5f}},
    // TOP (+Y)
    {{-0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}},
};

constexpr glm::ivec3 FACE_DIR[6] = {{0, 0, 1}, {0, 0, -1}, {-1, 0, 0}, {1, 0, 0}, {0, -1, 0}, {0, 1, 0}};

constexpr float AO_CURVE[4] = {0.45f, 0.65f, 0.82f, 1.0f};

int aoLevel(bool s1, bool s2, bool corner) {
    if (s1 && s2)
        return 0;
    return 3 - (int(s1) + int(s2) + int(corner));
}

void aoSamples(int face, int i, glm::ivec3& s1, glm::ivec3& s2, glm::ivec3& corner) {
    glm::ivec3 n = FACE_DIR[face];
    corner = glm::ivec3(glm::round(FACE_VERTS[face][i] * 2.0f));
    s1 = n;
    s2 = n;
    bool firstTangent = true;
    for (int a = 0; a < 3; ++a) {
        if (n[a] != 0)
            continue;
        if (firstTangent) {
            s1[a] = corner[a];
            firstTangent = false;
        } else {
            s2[a] = corner[a];
        }
    }
}

void addFace(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, glm::vec3 offset, int face, glm::vec3 color,
             glm::vec4 ao, float layer) {
    static constexpr glm::vec2 FACE_UV[4] = {
        {0.f, 1.f},
        {1.f, 1.f},
        {1.f, 0.f},
        {0.f, 0.f},
    };

    glm::vec3 normal = glm::vec3(FACE_DIR[face]);

    uint32_t start = static_cast<uint32_t>(vertices.size());
    for (int i = 0; i < 4; ++i) {
        vertices.push_back({FACE_VERTS[face][i] + offset, color * ao[i], glm::vec3(FACE_UV[i], layer), normal});
    }
    indices.push_back(start + 0);
    indices.push_back(start + 1);
    indices.push_back(start + 2);
    indices.push_back(start + 2);
    indices.push_back(start + 3);
    indices.push_back(start + 0);
}

void addWaterQuad(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, int x, int z, int w, int d) {
    const float y = float(WATER_LEVEL) + 0.5f;
    const float x0 = float(x) - 0.5f, x1 = x0 + float(w);
    const float z0 = float(z) - 0.5f, z1 = z0 + float(d);
    const glm::vec3 corners[4] = {{x0, y, z1}, {x1, y, z1}, {x1, y, z0}, {x0, y, z0}};

    uint32_t start = static_cast<uint32_t>(vertices.size());
    for (const glm::vec3& c : corners) {
        vertices.push_back({c, glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)});
    }
    indices.push_back(start + 0);
    indices.push_back(start + 1);
    indices.push_back(start + 2);
    indices.push_back(start + 2);
    indices.push_back(start + 3);
    indices.push_back(start + 0);
}

constexpr uint8_t BLOCK_AIR = 0;
constexpr uint8_t BLOCK_DIRT = 1;
constexpr uint8_t BLOCK_STONE = 2;
constexpr uint8_t BLOCK_WOOD = 3;
constexpr uint8_t BLOCK_LEAVES = 4;

constexpr int MIN_Y = -32;
constexpr int MAX_Y = 128;
constexpr int RANGE_Y = MAX_Y - MIN_Y + 1;
constexpr int SEA = 0;

constexpr int FACE_TOP = 5;

constexpr float LAYER_GRASS = 0.0f;
constexpr float LAYER_ROCK = 1.0f;

constexpr int CANOPY_BOTTOM = 2;
constexpr int CANOPY_TOP = 1;

const glm::vec3 TINT_GRASS_LUSH{0.85f, 1.05f, 0.80f};
const glm::vec3 TINT_GRASS_DRY{1.55f, 1.30f, 0.60f};
const glm::vec3 TINT_ROCK{1.00f, 0.98f, 0.95f};
const glm::vec3 TINT_SAND{1.80f, 1.52f, 0.95f};
const glm::vec3 TINT_DIRT{1.15f, 0.80f, 0.52f};
const glm::vec3 TINT_STONE{0.85f, 0.84f, 0.86f};
const glm::vec3 TINT_SEABED{1.25f, 1.15f, 0.85f};
const glm::vec3 TINT_WOOD{0.72f, 0.48f, 0.30f};
const glm::vec3 TINT_LEAVES_LUSH{0.58f, 0.82f, 0.46f};
const glm::vec3 TINT_LEAVES_DRY{0.95f, 0.85f, 0.40f};

struct SurfaceMaterial {
    glm::vec3 tint;
    float layer;
};

SurfaceMaterial pickMaterial(int y, int depth, int slope, float biome, float jitter, bool shore, bool topFace,
                             const TerrainParams& params) {
    if (depth >= params.soilDepth)
        return {TINT_STONE, LAYER_ROCK};

    if (y < WATER_LEVEL)
        return {TINT_SEABED, LAYER_ROCK};

    if (slope >= params.cliffSlope)
        return {TINT_ROCK, LAYER_ROCK};

    if ((shore && y <= WATER_LEVEL + params.beachHeight) || biome > params.desertThreshold)
        return {TINT_SAND, LAYER_ROCK};

    if (!topFace || depth > 0)
        return {TINT_DIRT, LAYER_ROCK};

    float alpine = glm::smoothstep(params.alpineStart, params.alpineEnd, float(y));
    if (jitter < alpine)
        return {TINT_ROCK, LAYER_ROCK};

    return {glm::mix(TINT_GRASS_LUSH, TINT_GRASS_DRY, glm::smoothstep(0.45f, 0.62f, biome)), LAYER_GRASS};
}

bool isTreeBlock(uint8_t block) {
    return block == BLOCK_WOOD || block == BLOCK_LEAVES;
}

SurfaceMaterial treeMaterial(uint8_t block, float biome, float jitter) {
    if (block == BLOCK_WOOD)
        return {TINT_WOOD * (0.90f + 0.20f * jitter), LAYER_ROCK};

    glm::vec3 leaf = glm::mix(TINT_LEAVES_LUSH, TINT_LEAVES_DRY, glm::smoothstep(0.45f, 0.62f, biome));
    return {leaf * (0.85f + 0.30f * jitter), LAYER_GRASS};
}

} // namespace

std::vector<Chunk> generateChunkedTerrain(World& world, const TerrainParams& params) {
    ZoneScoped;
    const int SIZE = params.worldSize;
    const int HALF = SIZE / 2;
    const int chunksPerAxis = (SIZE + CHUNK_SIZE_X - 1) / CHUNK_SIZE_X;
    ZoneTextF("world %dx%d, %d chunks, voxelMap %.1f MB", SIZE, SIZE, chunksPerAxis * chunksPerAxis,
              double(SIZE) * double(SIZE) * double(RANGE_Y) / (1024.0 * 1024.0));

    TracyCZoneN(zoneInit, "Init chunks", true);
    std::vector<Chunk> chunks(chunksPerAxis * chunksPerAxis);

    for (int cx = 0; cx < chunksPerAxis; ++cx) {
        for (int cz = 0; cz < chunksPerAxis; ++cz) {
            float wx = (cx * CHUNK_SIZE_X) - HALF + CHUNK_SIZE_X * 0.5f;
            float wz = (cz * CHUNK_SIZE_Z) - HALF + CHUNK_SIZE_Z * 0.5f;
            chunks[cx * chunksPerAxis + cz].center = glm::vec3(wx, 0.0f, wz);
        }
    }

    TracyCZoneEnd(zoneInit);

    TracyCZoneN(zoneAlloc, "Alloc voxelMap + column arrays", true);
    std::vector<uint8_t> voxelMap(SIZE * SIZE * RANGE_Y, BLOCK_AIR);
    std::vector<int32_t> columnHeight(SIZE * SIZE, 0);
    std::vector<float> columnBiome(SIZE * SIZE, 0.0f);
    std::vector<float> columnJitter(SIZE * SIZE, 0.0f);
    std::vector<uint8_t> columnSlope(SIZE * SIZE, 0);
    TracyCZoneEnd(zoneAlloc);

    const uint32_t seedHash = static_cast<uint32_t>(params.seed) * 0x9E3779B9u;

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
    auto fbmRidged = [&octaves](float x, float z) { return octaves(x, z, true); };

    TracyCZoneN(zoneHeight, "Heightmap + noise + column fill", true);
    for (int gx = 0; gx < SIZE; ++gx) {
        for (int gz = 0; gz < SIZE; ++gz) {
            int x = gx - HALF;
            int z = gz - HALF;
            float fx = float(x), fz = float(z);

            float wx =
                (fbm((fx + 1000.f) * params.warpFreq, (fz + 1000.f) * params.warpFreq) - 0.5f) * params.warpStrength;
            float wz =
                (fbm((fx - 1000.f) * params.warpFreq, (fz - 1000.f) * params.warpFreq) - 0.5f) * params.warpStrength;
            fx += wx;
            fz += wz;

            float plains = fbm(fx * params.plainsFreq, fz * params.plainsFreq);
            float sel = noise(fx * 0.008f, fz * 0.008f);
            sel = glm::clamp((sel - 0.5f) / 0.5f, 0.0f, 1.0f);
            sel = sel * sel;
            float mountains = fbmRidged(fx * params.mountainFreq, fz * params.mountainFreq);
            float cont = fbm(fx * params.continentFreq - 2000.f, fz * params.continentFreq + 2000.f);
            float land = glm::smoothstep(params.continentLo, params.continenHi, cont);
            float seabed = params.oceanFloor + plains * params.oceanRelief;
            float lowland = plains * 16.0f - 5.0f;

            float hf = glm::mix(seabed, lowland, land) + sel * mountains * 110.0f * land;
            int h = SEA + static_cast<int>(std::floor(hf));

            columnHeight[gx * SIZE + gz] = h;
            columnBiome[gx * SIZE + gz] =
                0.75f * noise(fx * 0.006f + 300.f, fz * 0.006f - 300.f) + 0.25f * noise(fx * 0.02f, fz * 0.02f);
            columnJitter[gx * SIZE + gz] = noise(fx * 0.05f - 700.f, fz * 0.05f + 700.f);

            for (int y = MIN_Y; y <= h; ++y) {
                uint8_t block = (h - y < 4) ? BLOCK_DIRT : BLOCK_STONE;
                world.setBlock(gx - HALF, y, gz - HALF, block);
            }
        }
    }

    TracyCZoneEnd(zoneHeight);

    TracyCZoneN(zoneSlope, "Column slope", true);
    for (int gx = 0; gx < SIZE; ++gx) {
        for (int gz = 0; gz < SIZE; ++gz) {
            int h = columnHeight[gx * SIZE + gz];
            int maxDelta = 0;
            constexpr int NEIGHBOR[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
            for (const auto& n : NEIGHBOR) {
                int nx = glm::clamp(gx + n[0], 0, SIZE - 1);
                int nz = glm::clamp(gz + n[1], 0, SIZE - 1);
                maxDelta = std::max(maxDelta, std::abs(h - columnHeight[nx * SIZE + nz]));
            }
            columnSlope[gx * SIZE + gz] = static_cast<uint8_t>(std::min(maxDelta, 255));
        }
    }

    TracyCZoneEnd(zoneSlope);

    TracyCZoneN(zoneShore, "Shore dilation", true);
    std::vector<uint8_t> columnShore(SIZE * SIZE, 0);
    {
        std::vector<uint8_t> rowDilate(SIZE * SIZE, 0);
        for (int gx = 0; gx < SIZE; ++gx) {
            for (int gz = 0; gz < SIZE; ++gz) {
                int lo = std::max(0, gz - params.beachRadius);
                int hi = std::min(SIZE - 1, gz + params.beachRadius);
                bool near = false;
                for (int k = lo; k <= hi && !near; ++k)
                    near = columnHeight[gx * SIZE + k] < WATER_LEVEL;
                rowDilate[gx * SIZE + gz] = near ? 1 : 0;
            }
        }
        for (int gx = 0; gx < SIZE; ++gx) {
            for (int gz = 0; gz < SIZE; ++gz) {
                int lo = std::max(0, gx - params.beachRadius);
                int hi = std::min(SIZE - 1, gx + params.beachRadius);
                bool near = false;
                for (int k = lo; k <= hi && !near; ++k)
                    near = rowDilate[k * SIZE + gz] != 0;
                columnShore[gx * SIZE + gz] = near ? 1 : 0;
            }
        }
    }

    TracyCZoneEnd(zoneShore);

    auto growsGrass = [&](int gx, int gz) {
        const int idx = gx * SIZE + gz;
        const int h = columnHeight[idx];
        if (h <= WATER_LEVEL || columnSlope[idx] >= params.cliffSlope)
            return false;
        if (columnShore[idx] != 0 && h <= WATER_LEVEL + params.beachHeight)
            return false;
        if (columnBiome[idx] > params.desertThreshold)
            return false;
        return columnJitter[idx] >= glm::smoothstep(params.alpineStart, params.alpineEnd, float(h));
    };

    auto plant = [&](int gx, int gz, int y, uint8_t block) {
        if (gx < 0 || gx >= SIZE || gz < 0 || gz >= SIZE || y < MIN_Y || y > MAX_Y)
            return;

        if (world.getBlock(gx - HALF, y, gz - HALF) == BLOCK_AIR)
            world.setBlock(gx - HALF, y, gz - HALF, block);
    };

    TracyCZoneN(zoneTrees, "Trees", true);
    for (int cellX = 0; cellX * params.treeCell < SIZE; ++cellX) {
        for (int cellZ = 0; cellZ * params.treeCell < SIZE; ++cellZ) {
            const int gx = cellX * params.treeCell + int(hash(cellX + 10007, cellZ + 20011) * params.treeCell);
            const int gz = cellZ * params.treeCell + int(hash(cellX + 30011, cellZ + 40013) * params.treeCell);
            if (gx >= SIZE || gz >= SIZE || !growsGrass(gx, gz))
                continue;

            const float forest = fbm((float(gx - HALF) + 4000.f) * 0.01f, (float(gz - HALF) - 4000.f) * 0.01f);
            if (hash(cellX + 50021, cellZ + 60029) > glm::smoothstep(params.forestLo, params.forestHi, forest))
                continue;

            const int base = columnHeight[gx * SIZE + gz] + 1;
            const int top = base + params.trunkMin + int(hash(cellX + 70039, cellZ + 80051) * params.trunkVar) - 1;
            if (top + CANOPY_TOP > MAX_Y)
                continue;

            for (int y = base; y <= top; ++y)
                plant(gx, gz, y, BLOCK_WOOD);

            for (int dy = -CANOPY_BOTTOM; dy <= CANOPY_TOP; ++dy) {
                const int radius = (dy < 0) ? 2 : 1;
                for (int dx = -radius; dx <= radius; ++dx) {
                    for (int dz = -radius; dz <= radius; ++dz) {
                        const bool corner = std::abs(dx) == radius && std::abs(dz) == radius;
                        if (corner && (dy == CANOPY_TOP || hash(gx + dx * 31, gz + dz * 17) < 0.55f))
                            continue;
                        plant(gx + dx, gz + dz, top + dy, BLOCK_LEAVES);
                    }
                }
            }
        }
    }

    TracyCZoneEnd(zoneTrees);

    auto getVoxel = [&](int x, int y, int z) -> uint8_t { return world.getBlock(x, y, z); };

    auto isSolidAO = [&](int x, int y, int z) { return getVoxel(x, y, z) != BLOCK_AIR; };

    auto faceAO = [&](int x, int y, int z, int face) {
        glm::vec4 ao;
        for (int i = 0; i < 4; ++i) {
            glm::ivec3 s1, s2, c;
            aoSamples(face, i, s1, s2, c);
            bool b1 = isSolidAO(x + s1.x, y + s1.y, z + s1.z);
            bool b2 = isSolidAO(x + s2.x, y + s2.y, z + s2.z);
            bool bc = isSolidAO(x + c.x, y + c.y, z + c.z);
            ao[i] = AO_CURVE[aoLevel(b1, b2, bc)];
        }
        return ao;
    };

    TracyCZoneN(zoneMesh, "Meshing (faces + AO)", true);
    for (int gx = 0; gx < SIZE; ++gx) {
        for (int gz = 0; gz < SIZE; ++gz) {
            int x = gx - HALF;
            int z = gz - HALF;

            int cx = gx / CHUNK_SIZE_X;
            int cz = gz / CHUNK_SIZE_Z;
            Chunk& chunk = chunks[cx * chunksPerAxis + cz];

            const int surfaceY = columnHeight[gx * SIZE + gz];
            const float biome = columnBiome[gx * SIZE + gz];
            const float jitter = columnJitter[gx * SIZE + gz];
            const int slope = columnSlope[gx * SIZE + gz];
            const bool shore = columnShore[gx * SIZE + gz] != 0;

            for (int y = MIN_Y; y <= MAX_Y; ++y) {
                uint8_t block = getVoxel(x, y, z);
                if (block == BLOCK_AIR)
                    continue;

                const int depth = surfaceY - y;

                for (int f = 0; f < 6; ++f) {
                    glm::ivec3 d = FACE_DIR[f];
                    uint8_t neighbor = getVoxel(x + d.x, y + d.y, z + d.z);

                    if (neighbor == BLOCK_AIR) {
                        SurfaceMaterial mat = isTreeBlock(block) ? treeMaterial(block, biome, jitter)
                                                                 : pickMaterial(y, depth, slope, biome, jitter, shore,
                                                                                f == FACE_TOP, params);
                        glm::vec4 ao = faceAO(x, y, z, f);
                        addFace(chunk.vertices, chunk.indices, glm::vec3(x, y, z), f, mat.tint, ao, mat.layer);
                    }
                }
            }
        }
    }

#ifdef TRACY_ENABLE
    {
        size_t totalVerts = 0, totalIdx = 0;
        for (const Chunk& c : chunks) {
            totalVerts += c.vertices.size();
            totalIdx += c.indices.size();
        }
        TracyCZoneValue(zoneMesh, totalVerts);
        TracyPlot("Terrain vertices", static_cast<int64_t>(totalVerts));
        TracyPlot("Terrain indices", static_cast<int64_t>(totalIdx));
    }
#endif
    TracyCZoneEnd(zoneMesh);

    TracyCZoneN(zoneWater, "Water quads (greedy)", true);
    for (int cx = 0; cx < chunksPerAxis; ++cx) {
        for (int cz = 0; cz < chunksPerAxis; ++cz) {
            Chunk& chunk = chunks[cx * chunksPerAxis + cz];
            const int baseX = cx * CHUNK_SIZE_X;
            const int baseZ = cz * CHUNK_SIZE_Z;

            bool used[CHUNK_SIZE_X][CHUNK_SIZE_Z] = {};
            auto flooded = [&](int lx, int lz) {
                int gx = baseX + lx, gz = baseZ + lz;
                if (gx >= SIZE || gz >= SIZE)
                    return false;
                return columnHeight[gx * SIZE + gz] < WATER_LEVEL;
            };

            for (int lx = 0; lx < CHUNK_SIZE_X; ++lx) {
                for (int lz = 0; lz < CHUNK_SIZE_Z; ++lz) {
                    if (used[lx][lz] || !flooded(lx, lz))
                        continue;

                    int w = 1;
                    while (lx + w < CHUNK_SIZE_X && !used[lx + w][lz] && flooded(lx + w, lz))
                        ++w;

                    int d = 1;
                    for (bool grow = true; grow && lz + d < CHUNK_SIZE_Z; ++d) {
                        for (int i = 0; i < w; ++i) {
                            if (used[lx + i][lz + d] || !flooded(lx + i, lz + d)) {
                                grow = false;
                                break;
                            }
                        }
                        if (!grow)
                            break;
                    }

                    for (int i = 0; i < w; ++i)
                        for (int j = 0; j < d; ++j)
                            used[lx + i][lz + j] = true;

                    addWaterQuad(chunk.waterVertices, chunk.waterIndices, baseX + lx - HALF, baseZ + lz - HALF, w, d);
                }
            }
        }
    }

#ifdef TRACY_ENABLE
    {
        size_t quads = 0;
        for (const Chunk& c : chunks)
            quads += c.waterIndices.size() / 6;
        TracyCZoneValue(zoneWater, quads);
    }
#endif
    TracyCZoneEnd(zoneWater);

    return chunks;
}
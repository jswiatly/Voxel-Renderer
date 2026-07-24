#include "scene/Terrain.hpp"

#include <cmath>
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>

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

constexpr uint8_t BLOCK_AIR = 0;
constexpr uint8_t BLOCK_GRASS = 1;
constexpr uint8_t BLOCK_DIRT = 2;
constexpr uint8_t BLOCK_STONE = 3;
constexpr uint8_t BLOCK_SAND = 4;
constexpr uint8_t BLOCK_WATER = 5;

constexpr int MIN_Y = -32;
constexpr int MAX_Y = 128;
constexpr int RANGE_Y = MAX_Y - MIN_Y + 1;
constexpr int SEA = 0;

}

std::vector<Chunk> generateChunkedTerrain(int worldSize, int seed) {
    const int SIZE = worldSize;
    const int HALF = SIZE / 2;
    const int chunksPerAxis = (SIZE + CHUNK_SIZE_X - 1) / CHUNK_SIZE_X;
    std::vector<Chunk> chunks(chunksPerAxis * chunksPerAxis);

    for (int cx = 0; cx < chunksPerAxis; ++cx) {
        for (int cz = 0; cz < chunksPerAxis; ++cz) {
            float wx = (cx * CHUNK_SIZE_X) - HALF + CHUNK_SIZE_X * 0.5f;
            float wz = (cz * CHUNK_SIZE_Z) - HALF + CHUNK_SIZE_Z * 0.5f;
            chunks[cx * chunksPerAxis + cz].center = glm::vec3(wx, 0.0f, wz);
        }
    }

    std::vector<uint8_t> voxelMap(SIZE * SIZE * RANGE_Y, BLOCK_AIR);

    float seedPhase = float(seed) * 78.233f;

    auto noise = [seedPhase](float x, float z) {
        float xi = std::floor(x), zi = std::floor(z);
        float xf = x - xi, zf = z - zi;
        auto h = [seedPhase](float a, float b) {
            float v = std::sin(a * 127.1f + b * 311.7f + seedPhase) * 43758.5453f;
            return v - std::floor(v);
        };
        float u = xf * xf * (3.f - 2.f * xf);
        float v = zf * zf * (3.f - 2.f * zf);
        return glm::mix(glm::mix(h(xi, zi), h(xi + 1, zi), u), glm::mix(h(xi, zi + 1), h(xi + 1, zi + 1), u), v);
    };

    auto fbm = [&](float x, float z) {
        float t = 0.f, a = 1.f, f = 1.f, n = 0.f;
        for (int i = 0; i < 6; ++i) {
            float v = noise(x * f, z * f);
            t += (1.f - std::abs(v - 0.5f) * 2.f) * a;
            n += a;
            a *= 0.5f;
            f *= 2.0f;
        }
        return t / n;
    };

    auto hash3 = [seedPhase](float x, float y, float z) {
        float v = std::sin(x * 127.1f + y * 311.7f + z * 74.7f + seedPhase) * 43758.5453f;
        return v - std::floor(v);
    };

    auto noise3D = [&](float x, float y, float z) {
        float xi = std::floor(x), yi = std::floor(y), zi = std::floor(z);
        float xf = x - xi, yf = y - yi, zf = z - zi;

        float u = xf * xf * (3.f - 2.f * xf);
        float v = yf * yf * (3.f - 2.f * yf);
        float w = zf * zf * (3.f - 2.f * zf);

        auto lerp = [](float a, float b, float t) { return a + t * (b - a); };

        float c00 = lerp(hash3(xi, yi, zi), hash3(xi + 1, yi, zi), u);
        float c10 = lerp(hash3(xi, yi + 1, zi), hash3(xi + 1, yi + 1, zi), u);
        float c01 = lerp(hash3(xi, yi, zi + 1), hash3(xi + 1, yi, zi + 1), u);
        float c11 = lerp(hash3(xi, yi + 1, zi + 1), hash3(xi + 1, yi + 1, zi + 1), u);

        float c0 = lerp(c00, c10, v);
        float c1 = lerp(c01, c11, v);

        return lerp(c0, c1, w);
    };

    auto fbm3D = [&](float x, float y, float z) {
        float t = 0.f, a = 1.f, f = 1.f, n = 0.f;
        for (int i = 0; i < 4; ++i) {
            t += noise3D(x * f, y * f, z * f) * a;
            n += a;
            a *= 0.5f;
            f *= 2.0f;
        }
        return t / n;
    };

    for (int gx = 0; gx < SIZE; ++gx) {
        for (int gz = 0; gz < SIZE; ++gz) {
            int x = gx - HALF;
            int z = gz - HALF;
            float fx = float(x), fz = float(z);

            float wx = (fbm((fx + 1000.f) * 0.005f, (fz + 1000.f) * 0.005f) - 0.5f) * 80.f;
            float wz = (fbm((fx - 1000.f) * 0.005f, (fz - 1000.f) * 0.005f) - 0.5f) * 80.f;
            fx += wx;
            fz += wz;

            float plains = fbm(fx * 0.01f, fz * 0.01f);
            float sel = noise(fx * 0.008f, fz * 0.008f);
            sel = glm::clamp((sel - 0.5f) / 0.5f, 0.0f, 1.0f);
            sel = sel * sel;
            float mountains = fbm(fx * 0.03f, fz * 0.03f);

            int h = SEA + static_cast<int>(plains * 6.0f) + static_cast<int>(sel * mountains * 110.0f);

            float moisture = fbm(float(gx) * 0.005f + 500.f, float(gz) * 0.005f + 500.f);

            for (int y = MIN_Y; y <= MAX_Y; ++y) {
                uint8_t block = BLOCK_AIR;

                if (y <= h) {
                    float density = float(h - y);

                    if (y < h - 3) {
                        float caveNoise = fbm3D(float(x) * 0.04f, float(y) * 0.04f, float(z) * 0.04f);
                        if (caveNoise > 0.6f) {
                            density = -1.0f;
                        }
                    }

                    if (density >= 0.0f) {
                        int depth = h - y;
                        bool isBeachOrDesert = (h <= SEA + 2 && moisture < 0.5f);

                        if (depth == 0) {
                            block = isBeachOrDesert ? BLOCK_SAND : BLOCK_GRASS;
                        } else if (depth < 4) {
                            block = isBeachOrDesert ? BLOCK_SAND : BLOCK_DIRT;
                        } else {
                            block = BLOCK_STONE;
                        }
                    }
                }

                if (block == BLOCK_AIR && y <= SEA) {
                    block = BLOCK_WATER;
                }

                if (block != BLOCK_AIR) {
                    voxelMap[(gx * SIZE + gz) * RANGE_Y + (y - MIN_Y)] = block;
                }
            }
        }
    }

    auto getVoxel = [&](int x, int y, int z) -> uint8_t {
        if (y < MIN_Y)
            return BLOCK_STONE;
        int gx = x + HALF, gz = z + HALF;
        if (gx < 0 || gx >= SIZE || gz < 0 || gz >= SIZE || y > MAX_Y)
            return BLOCK_AIR;
        return voxelMap[(gx * SIZE + gz) * RANGE_Y + (y - MIN_Y)];
    };

    auto isSolidAO = [&](int x, int y, int z) {
        uint8_t b = getVoxel(x, y, z);
        return b != BLOCK_AIR && b != BLOCK_WATER;
    };

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

    for (int gx = 0; gx < SIZE; ++gx) {
        for (int gz = 0; gz < SIZE; ++gz) {
            int x = gx - HALF;
            int z = gz - HALF;

            int cx = gx / CHUNK_SIZE_X;
            int cz = gz / CHUNK_SIZE_Z;
            Chunk& chunk = chunks[cx * chunksPerAxis + cz];

            for (int y = MIN_Y; y <= MAX_Y; ++y) {
                uint8_t block = getVoxel(x, y, z);
                if (block == BLOCK_AIR)
                    continue;

                float layer = 0.0f;
                glm::vec3 col(1.0f);

                if (block == BLOCK_GRASS)
                    layer = 0.0f;
                else if (block == BLOCK_DIRT)
                    layer = 1.0f;
                else if (block == BLOCK_STONE)
                    layer = 2.0f;
                else if (block == BLOCK_SAND)
                    layer = 3.0f;
                else if (block == BLOCK_WATER) {
                    layer = 4.0f;
                    col = glm::vec3(0.3f, 0.6f, 0.9f);
                }

                for (int f = 0; f < 6; ++f) {
                    glm::ivec3 d = FACE_DIR[f];
                    uint8_t neighbor = getVoxel(x + d.x, y + d.y, z + d.z);

                    bool drawFace = false;
                    if (block == BLOCK_WATER) {
                        drawFace = (neighbor == BLOCK_AIR);
                    } else {
                        drawFace = (neighbor == BLOCK_AIR || neighbor == BLOCK_WATER);
                    }

                    if (drawFace) {
                        glm::vec4 ao = faceAO(x, y, z, f);
                        addFace(chunk.vertices, chunk.indices, glm::vec3(x, y, z), f, col, ao, layer);
                    }
                }
            }
        }
    }
    return chunks;
}
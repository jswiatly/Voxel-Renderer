#include "scene/Terrain.hpp"

#include <cmath>
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

void addFace(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, glm::vec3 offset, int face,
             glm::vec3 color) {
    static constexpr glm::vec2 FACE_UV[4] = {
        {0.f, 1.f},
        {1.f, 1.f},
        {1.f, 0.f},
        {0.f, 0.f},
    };

    glm::vec3 normal = glm::vec3(FACE_DIR[face]);

    uint32_t start = static_cast<uint32_t>(vertices.size());
    for (int i = 0; i < 4; ++i) {
        vertices.push_back({FACE_VERTS[face][i] + offset, color, FACE_UV[i], normal});
    }
    indices.push_back(start + 0);
    indices.push_back(start + 1);
    indices.push_back(start + 2);
    indices.push_back(start + 2);
    indices.push_back(start + 3);
    indices.push_back(start + 0);
}

}

std::vector<Chunk> generateChunkedTerrain(int worldSize) {
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

    std::vector<int> heightMap(SIZE * SIZE);

        auto noise = [](float x, float z) {
        float xi = std::floor(x), zi = std::floor(z);
        float xf = x - xi, zf = z - zi;
        auto h = [](float a, float b) {
            float v = std::sin(a * 127.1f + b * 311.7f) * 43758.5453f;
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

    for (int gx = 0; gx < SIZE; ++gx) {
        for (int gz = 0; gz < SIZE; ++gz) {
            int x = gx - HALF;
            int z = gz - HALF;
            int h = static_cast<int>(fbm(x * 0.025f, z * 0.025f) * 255.f) - 8;
            heightMap[gx * SIZE + gz] = h;
        }
    }

    auto isSolid = [&](int x, int y, int z) {
        int gx = x + HALF, gz = z + HALF;
        if (gx < 0 || gx >= SIZE || gz < 0 || gz >= SIZE)
            return false;
        if (y < -6)
            return true;
        return y <= heightMap[gx * SIZE + gz];
    };

    for (int gx = 0; gx < SIZE; ++gx) {
        for (int gz = 0; gz < SIZE; ++gz) {
            int x = gx - HALF;
            int z = gz - HALF;
            int h = heightMap[gx * SIZE + gz];

            int cx = gx / CHUNK_SIZE_X;
            int cz = gz / CHUNK_SIZE_Z;
            Chunk& chunk = chunks[cx * chunksPerAxis + cz];

            for (int y = -6; y <= h; ++y) {
                glm::vec3 col(0.9f);
                for (int f = 0; f < 6; ++f) {
                    glm::ivec3 d = FACE_DIR[f];
                    if (!isSolid(x + d.x, y + d.y, z + d.z)) {
                        addFace(chunk.vertices, chunk.indices, glm::vec3(x, y, z), f, col);
                    }
                }
            }
        }
    }
    return chunks;
}
#include "scene/PlayerBlock.hpp"

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

constexpr glm::vec3 FACE_NORMAL[6] = {
    {0, 0, 1}, {0, 0, -1}, {-1, 0, 0}, {1, 0, 0}, {0, -1, 0}, {0, 1, 0},
};

constexpr glm::vec2 FACE_UV[4] = {{0.f, 1.f}, {1.f, 1.f}, {1.f, 0.f}, {0.f, 0.f}};

} // namespace

BlockMeshData buildBlockMesh(glm::vec3 size, glm::vec3 color, float textureLayer) {
    BlockMeshData out;
    out.vertices.reserve(24);
    out.indices.reserve(36);

    for (int face = 0; face < 6; ++face) {
        const uint32_t start = static_cast<uint32_t>(out.vertices.size());
        for (int i = 0; i < 4; ++i) {
            out.vertices.push_back(
                {FACE_VERTS[face][i] * size, color, glm::vec3(FACE_UV[i], textureLayer), FACE_NORMAL[face]});
        }
        out.indices.insert(out.indices.end(), {start + 0, start + 1, start + 2, start + 2, start + 3, start + 0});
    }
    return out;
}
#pragma once

#include "renderer/Vertex.hpp"
#include <cstdint>
#include <vector>

struct Chunk {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    glm::vec3 center{0.0f};
};

inline constexpr int CHUNK_SIZE_X = 16;
inline constexpr int CHUNK_SIZE_Z = 16;
inline constexpr int CHUNK_SIZE_Y = 256;
inline constexpr int WORLD_SIZE = 1024;

std::vector<Chunk> generateChunkedTerrain(int worldSize = WORLD_SIZE, int seed = 0);
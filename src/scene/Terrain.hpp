#pragma once

#include "renderer/Vertex.hpp"
#include "scene/TerrainParams.hpp"
#include <cstdint>
#include <vector>

struct Chunk {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    // Water surface lives in its own buffers: it is drawn by a separate,
    // alpha-blended pipeline after the opaque terrain.
    std::vector<Vertex> waterVertices;
    std::vector<uint32_t> waterIndices;
    glm::vec3 center{0.0f};
};

inline constexpr int WATER_LEVEL = 0;

inline constexpr int CHUNK_SIZE_X = 16;
inline constexpr int CHUNK_SIZE_Z = 16;
inline constexpr int CHUNK_SIZE_Y = 256;
inline constexpr int WORLD_SIZE = 1024;

std::vector<Chunk> generateChunkedTerrain(const TerrainParams& params);
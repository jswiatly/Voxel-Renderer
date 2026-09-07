#pragma once

#include "renderer/Vertex.hpp"

#include <cstdint>
#include <vector>

struct BlockMeshData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

BlockMeshData buildBlockMesh(glm::vec3 size, glm::vec3 color, float textureLayer = 1.0f);
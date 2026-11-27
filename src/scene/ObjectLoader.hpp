#pragma once

#include "renderer/Vertex.hpp"

#include <cstdint>
#include <string>
#include <vector>

struct ObjMeshData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

ObjMeshData loadObj(const std::string& path, float textureLayer = 1.0f);
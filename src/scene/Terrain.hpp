#pragma once

#include "renderer/Vertex.hpp"
#include <cstdint>
#include <vector>

void generateTerrain(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, int size = 220);
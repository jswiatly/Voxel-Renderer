#pragma once

#include "renderer/Vertex.hpp"
#include <vector>
#include <cstdint>

void generateTerrain(std::vector<Vertex>& vertices, 
                    std::vector<uint32_t>& indices, 
                    int size = 220);
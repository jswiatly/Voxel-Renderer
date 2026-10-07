#pragma once

#include <cstdint>
#include <vector>

struct TerrainMetaData {
    int size = 0;
    int half = 0;

    std::vector<int32_t> columnTopY;
    std::vector<float> biome;
    std::vector<float> jitter;
    std::vector<uint8_t> slope;
    std::vector<uint8_t> shore;

    int index(int gx, int gz) const {
        return gx * size + gz;
    }
};
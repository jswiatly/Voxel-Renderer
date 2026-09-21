#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

class World{
    public:
        static constexpr int MIN_Y = -32;
        static constexpr int MAX_Y = 128;
        static constexpr int RANGE_Y = MAX_Y - MIN_Y + 1;

        World(int size);

        uint8_t getBlock(int x, int y, int z) const;
        bool isSolid(int x, int y, int z) const;

        void setBlock(int x, int y, int z, uint8_t block);

    private:
        int m_size;
        int m_half;

        std::vector<uint8_t> m_voxelMap;

        size_t getIndex(int x, int y, int z) const;
};
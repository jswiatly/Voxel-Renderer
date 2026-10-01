#pragma once
#include <cstdint>

enum class Block : uint8_t {
    Air = 0,
    Grass,
    Dirt,
    Stone,
    Water,
    Sand,
    Gravel,
    Bedrock,
    Wood,
    Leaves,
    Ice,
};

constexpr bool isSolid(Block block) {
    return block == Block::Grass || block == Block::Dirt || block == Block::Stone || block == Block::Sand ||
           block == Block::Gravel || block == Block::Bedrock || block == Block::Wood || block == Block::Leaves ||
           block == Block::Ice;
}

constexpr bool isOpaque(Block block) {
    return block != Block::Air && block != Block::Water && block != Block::Leaves;
}
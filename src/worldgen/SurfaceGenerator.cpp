#include "worldgen/SurfaceGenerator.hpp"

#include "scene/Block.hpp"
#include "scene/World.hpp"

#include "scene/Block.hpp"
#include "scene/World.hpp"

void SurfaceGenerator::generate(World& world, const TerrainMetaData& metadata) const {
    for (int gx = 0; gx < metadata.size; ++gx) {
        for (int gz = 0; gz < metadata.size; ++gz) {

            const int topY = metadata.columnTopY[metadata.index(gx, gz)];

            if (topY < World::MIN_Y)
                continue;

            const int worldX = gx - metadata.half;
            const int worldZ = gz - metadata.half;

            // Top block
            world.setBlock(worldX, topY, worldZ, static_cast<uint8_t>(Block::Grass));

            // Two blocks underneath
            if (topY - 1 >= World::MIN_Y) {
                world.setBlock(worldX, topY - 1, worldZ, static_cast<uint8_t>(Block::Dirt));
            }

            if (topY - 2 >= World::MIN_Y) {
                world.setBlock(worldX, topY - 2, worldZ, static_cast<uint8_t>(Block::Dirt));
            }
        }
    }
}
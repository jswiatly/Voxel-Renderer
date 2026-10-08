#pragma once

#include "TerrainMetadata.hpp"

class World;

class SurfaceGenerator {
  public:
    void generate(World& world, const TerrainMetaData& metadata) const;
};
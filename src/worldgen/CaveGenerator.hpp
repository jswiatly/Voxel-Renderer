#pragma once

#include "TerrainMetadata.hpp"
#include "WorldGenSettings.hpp"
#include "OctavePerlin.hpp"

class World;

class CaveGenerator {
  public:
    explicit CaveGenerator(const WorldGenSettings& settings);

    void generate(World& world, const TerrainMetaData& metadata) const;

  private:
    OctavePerlin m_tunnelA;
    OctavePerlin m_tunnelB;
};
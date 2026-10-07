#pragma once

#include "TerrainMetaData.hpp"
#include "WorldGenSettings.hpp"
#include "worldgen/OctavePerlin.hpp"

class World;

class TerrainGenerator {
  public:
    explicit TerrainGenerator(const WorldGenSettings& settings);

    TerrainMetaData generateBase(World& world) const;

  private:
    WorldGenSettings m_settings;

    OctavePerlin m_lowNoise;
    OctavePerlin m_highNoise;
};
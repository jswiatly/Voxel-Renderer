#pragma once

#include "TerrainMetadata.hpp"
#include "WorldGenSettings.hpp"

class World;

class FeatureGenerator {
  public:
    explicit FeatureGenerator(const WorldGenSettings& settings);

    void generateTrees(World& world, const TerrainMetaData& metadata) const;

  private:
    WorldGenSettings m_settings;
};
#pragma once

#include <cstdint>
#include <vector>
#include "ImprovedPerlin.hpp"

class OctavePerlin {
  public:
    OctavePerlin(uint64_t seed, int octaveCount);

    double sample2D(double x, double z) const;
    double sample3D(double x, double y, double z) const;

  private:
    std::vector<ImprovedPerlin> m_octaves;
};
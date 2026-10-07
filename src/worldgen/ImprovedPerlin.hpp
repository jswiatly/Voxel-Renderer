#pragma once

#include <array>
#include <cstdint>

class ImprovedPerlin {
  public:
    explicit ImprovedPerlin(uint64_t seed);

    double sample(double x, double y, double z) const;
    double sample2D(double x, double z) const;

  private:
    std::array<uint8_t, 512> m_permutation{};

    static double fade(double t);
    static double gradientDot(uint8_t hash, double x, double y, double z);
};
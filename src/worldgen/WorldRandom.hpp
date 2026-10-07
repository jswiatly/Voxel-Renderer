#pragma once

#include <cstdint>
#include <random>

class WorldRandom {
  public:
    explicit WorldRandom(uint64_t seed);

    uint64_t nextU64();

  private:
    std::mt19937_64 m_engine;
};
#include "WorldRandom.hpp"

WorldRandom::WorldRandom(uint64_t seed) : m_engine(seed) {}

uint64_t WorldRandom::nextU64() {
    return m_engine();
}
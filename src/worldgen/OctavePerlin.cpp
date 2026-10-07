#include "OctavePerlin.hpp"

OctavePerlin::OctavePerlin(uint64_t seed, int octaveCount) {
    m_octaves.reserve(octaveCount);

    for (int i = 0; i < octaveCount; ++i) {
        m_octaves.emplace_back(seed + i * 0x9E3779B97F4A7C15ULL);
    }
}

double OctavePerlin::sample2D(double x, double z) const {
    double result = 0.0;
    double amplitude = 1.0;
    double frequency = 1.0;
    double amplitudeSum = 0.0;

    for (const auto& octave : m_octaves) {
        result += octave.sample2D(x * frequency, z * frequency) * amplitude;

        amplitudeSum += amplitude;

        frequency *= 2.0;
        amplitude *= 0.5;
    }

    return result / amplitudeSum;
}

double OctavePerlin::sample3D(double x, double y, double z) const {
    double result = 0.0;
    double amplitude = 1.0;
    double frequency = 1.0;
    double amplitudeSum = 0.0;

    for (const auto& octave : m_octaves) {
        result += octave.sample(x * frequency, y * frequency, z * frequency) * amplitude;

        amplitudeSum += amplitude;

        frequency *= 2.0;
        amplitude *= 0.5;
    }

    return result / amplitudeSum;
}
#include "ImprovedPerlin.hpp"

#include <algorithm>
#include <random>
#include <cmath>

ImprovedPerlin::ImprovedPerlin(uint64_t seed) {
    std::mt19937_64 random(seed);

    for (int i{0}; i < 256; i++) {
        m_permutation[i] = static_cast<uint8_t>(i);
    }

    std::shuffle(m_permutation.begin(), m_permutation.begin() + 256, random);

    for (int i{0}; i < 256; i++) {
        m_permutation[i + 256] = m_permutation[i];
    }
}

double ImprovedPerlin::fade(double t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

double ImprovedPerlin::gradientDot(uint8_t hash, double x, double y, double z) {
    const int h = hash & 15;

    const double u = h < 8 ? x : y;
    const double v = h < 4 ? y : (h == 12 || h == 14 ? x : z);

    const double first = (h & 1) == 0 ? u : -u;
    const double second = (h & 2) == 0 ? v : -v;

    return first + second;
}

double ImprovedPerlin::sample(double x, double y, double z) const {
    const double floorX = std::floor(x);
    const double floorY = std::floor(y);
    const double floorZ = std::floor(z);

    const int x0 = static_cast<int>(floorX) & 255;
    const int y0 = static_cast<int>(floorY) & 255;
    const int z0 = static_cast<int>(floorZ) & 255;

    const double localX = x - floorX;
    const double localY = y - floorY;
    const double localZ = z - floorZ;

    const double u = fade(localX);
    const double v = fade(localY);
    const double w = fade(localZ);

    const int a = m_permutation[x0] + y0;
    const int aa = m_permutation[a] + z0;
    const int ab = m_permutation[a + 1] + z0;

    const int b = m_permutation[x0 + 1] + y0;
    const int ba = m_permutation[b] + z0;
    const int bb = m_permutation[b + 1] + z0;

    const double x1 = std::lerp(gradientDot(m_permutation[aa], localX, localY, localZ),
                                gradientDot(m_permutation[ba], localX - 1.0, localY, localZ), u);

    const double x2 = std::lerp(gradientDot(m_permutation[ab], localX, localY - 1.0, localZ),
                                gradientDot(m_permutation[bb], localX - 1.0, localY - 1.0, localZ), u);

    const double y1 = std::lerp(x1, x2, v);

    const double x3 = std::lerp(gradientDot(m_permutation[aa + 1], localX, localY, localZ - 1.0),
                                gradientDot(m_permutation[ba + 1], localX - 1.0, localY, localZ - 1.0), u);

    const double x4 = std::lerp(gradientDot(m_permutation[ab + 1], localX, localY - 1.0, localZ - 1.0),
                                gradientDot(m_permutation[bb + 1], localX - 1.0, localY - 1.0, localZ - 1.0), u);

    const double y2 = std::lerp(x3, x4, v);

    return std::lerp(y1, y2, w);
}

double ImprovedPerlin::sample2D(double x, double z) const {
    return sample(x, 0.0, z);
}
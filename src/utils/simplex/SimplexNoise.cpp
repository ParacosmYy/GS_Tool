/**
 * @file SimplexNoise.cpp
 * @brief Simplex噪声生成器实现
 */

#include "utils/simplex/SimplexNoise.h"

#include <QElapsedTimer>
#include <algorithm>

SimplexNoise::SimplexNoise(int seed, QObject* parent)
    : QObject(parent)
{
    setSeed(seed);
}

void SimplexNoise::setSeed(int seed)
{
    int p[256];
    for (int i = 0; i < 256; ++i) p[i] = i;

    /* Fisher-Yates shuffle */
    for (int i = 255; i > 0; --i) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        int j = seed % (i + 1);
        std::swap(p[i], p[j]);
    }

    for (int i = 0; i < 512; ++i) m_perm[i] = p[i & 255];
}

double SimplexNoise::noise2D(double x, double y) const
{
    /* 2D Simplex噪声实现 */
    const double F2 = 0.5 * (qSqrt(3.0) - 1.0);
    const double G2 = (3.0 - qSqrt(3.0)) / 6.0;

    double s = (x + y) * F2;
    int i = qFloor(x + s);
    int j = qFloor(y + s);

    double t = (i + j) * G2;
    double x0 = x - (i - t);
    double y0 = y - (j - t);

    int i1 = (x0 > y0) ? 1 : 0;
    int j1 = (x0 > y0) ? 0 : 1;

    double x1 = x0 - i1 + G2;
    double y1 = y0 - j1 + G2;
    double x2 = x0 - 1.0 + 2.0 * G2;
    double y2 = y0 - 1.0 + 2.0 * G2;

    int ii = i & 255;
    int jj = j & 255;

    double n0 = 0.0, n1 = 0.0, n2 = 0.0;

    double t0 = 0.5 - x0 * x0 - y0 * y0;
    if (t0 >= 0) {
        t0 *= t0;
        n0 = t0 * t0 * grad2(m_perm[ii + m_perm[jj]], x0, y0);
    }

    double t1 = 0.5 - x1 * x1 - y1 * y1;
    if (t1 >= 0) {
        t1 *= t1;
        n1 = t1 * t1 * grad2(m_perm[ii + i1 + m_perm[jj + j1]], x1, y1);
    }

    double t2 = 0.5 - x2 * x2 - y2 * y2;
    if (t2 >= 0) {
        t2 *= t2;
        n2 = t2 * t2 * grad2(m_perm[ii + 1 + m_perm[jj + 1]], x2, y2);
    }

    m_stats.totalSamples++;
    return 70.0 * (n0 + n1 + n2);
}

double SimplexNoise::noise3D(double x, double y, double z) const
{
    /* 简化3D噪声：使用两层2D噪声近似 */
    double n = noise2D(x, y) * 0.5 + noise2D(y + 31.7, z + 17.3) * 0.3 +
               noise2D(x + 47.1, z + 23.9) * 0.2;
    m_stats.totalSamples++;
    return n;
}

double SimplexNoise::fractal2D(double x, double y, int octaves,
                                double persistence) const
{
    double total = 0.0;
    double amplitude = 1.0;
    double frequency = 1.0;
    double maxVal = 0.0;

    for (int i = 0; i < octaves; ++i) {
        total += noise2D(x * frequency, y * frequency) * amplitude;
        maxVal += amplitude;
        amplitude *= persistence;
        frequency *= 2.0;
        m_stats.totalOctaveSamples++;
    }

    return (maxVal > 0) ? total / maxVal : 0.0;
}

QVector<double> SimplexNoise::generate2DMap(int width, int height,
                                              double scale)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> map;
    map.reserve(width * height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            map.append(fractal2D(x * scale, y * scale));
        }
    }

    m_stats.totalSamples += width * height;
    emit mapGenerated(width, height);
    return map;
}

double SimplexNoise::grad2(int hash, double x, double y) const
{
    int h = hash & 7;
    double u = (h < 4) ? x : y;
    double v = (h < 4) ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

double SimplexNoise::grad3(int hash, double x, double y, double z) const
{
    int h = hash & 15;
    double u = (h < 8) ? x : y;
    double v = (h < 4) ? y : ((h == 12 || h == 14) ? x : z);
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

void SimplexNoise::resetStatistics()
{
    m_stats = Stats{};
}

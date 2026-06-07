/**
 * @file SplitRadixFFT5.cpp
 * @brief SplitRadixFFT5 实现
 *
 * 实现分裂基FFT：共轭对旋转因子优化、缓存友好迭代蝶形网络、位反转置换。
 */

#include "utils/fft205/SplitRadixFFT5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SplitRadixFFT5::SplitRadixFFT5(QObject *parent) : QObject(parent)
{
    computeTwiddles(m_size);
    buildBitRevTable(m_size);
}

SplitRadixFFT5::~SplitRadixFFT5() = default;

/* ---- Configuration ---- */

void SplitRadixFFT5::setTransformSize(int n)
{
    // Round to power of 2, minimum 4
    int p = 4;
    while (p < n) p <<= 1;
    m_size = p;
    computeTwiddles(m_size);
    buildBitRevTable(m_size);
}

/* ---- Bit reverse ---- */

int SplitRadixFFT5::bitReverse(int x, int log2n)
{
    int result = 0;
    for (int i = 0; i < log2n; ++i) {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}

/* ---- Build bit-reversal table ---- */

void SplitRadixFFT5::buildBitRevTable(int n)
{
    int log2n = 0;
    int temp = n;
    while (temp > 1) { log2n++; temp >>= 1; }

    m_bitRevTable.resize(n);
    for (int i = 0; i < n; ++i)
        m_bitRevTable[i] = bitReverse(i, log2n);
}

/* ---- Compute conjugate-pair twiddles ---- */

void SplitRadixFFT5::computeTwiddles(int n)
{
    int n4 = n / 4;
    m_cosTwiddle.resize(n4);
    m_sinTwiddle.resize(n4);

    for (int k = 0; k < n4; ++k) {
        double angle = 2.0 * M_PI * k / n;
        m_cosTwiddle[k] = qCos(angle);
        m_sinTwiddle[k] = qSin(angle);
    }
}

/* ---- Bit-reversal permutation ---- */

void SplitRadixFFT5::bitReversePermute(QVector<double>& data) const
{
    int n = data.size() / 2; // complex pairs
    for (int i = 0; i < n; ++i) {
        int j = m_bitRevTable[i % m_bitRevTable.size()];
        if (j > i) {
            std::swap(data[2 * i], data[2 * j]);
            std::swap(data[2 * i + 1], data[2 * j + 1]);
        }
    }
}

/* ---- Iterative split-radix butterfly ---- */

void SplitRadixFFT5::butterflyIterative(QVector<double>& data, bool inverse) const
{
    int n = data.size() / 2; // number of complex points
    double sign = inverse ? 1.0 : -1.0;

    // Split-radix: process in increasing block sizes
    // Block sizes: 4, 16, 64, ... (powers of 4) for L-shaped butterflies
    for (int m = 4; m <= n; m <<= 2) {
        int m4 = m / 4;
        int stride = n / m;

        for (int j = 0; j < m4; ++j) {
            // Twiddle index
            int twIdx = j * stride;
            double c1 = (twIdx < m_cosTwiddle.size()) ? m_cosTwiddle[twIdx] : 0.0;
            double s1 = (twIdx < m_sinTwiddle.size()) ? m_sinTwiddle[twIdx] * sign : 0.0;

            // 3x twiddle for split-radix
            int twIdx3 = 3 * twIdx;
            double c3 = qCos(2.0 * M_PI * twIdx3 / n);
            double s3 = qSin(2.0 * M_PI * twIdx3 / n) * sign;

            for (int k = j; k < n; k += m) {
                int k1 = k + m4;
                int k2 = k1 + m4;
                int k3 = k2 + m4;

                // Load complex values
                double x0r = data[2*k],   x0i = data[2*k+1];
                double x1r = data[2*k1],  x1i = data[2*k1+1];
                double x2r = data[2*k2],  x2i = data[2*k2+1];
                double x3r = data[2*k3],  x3i = data[2*k3+1];

                // Apply twiddle factors (conjugate pair optimization)
                double t1r = x1r*c1 - x1i*s1, t1i = x1r*s1 + x1i*c1;
                double t2r = x2r*c1 - x2i*s1, t2i = x2r*s1 + x2i*c1;
                double t3r = x3r*c3 - x3i*s3, t3i = x3r*s3 + x3i*c3;

                // L-shaped butterfly
                double sum12r = t1r + t2r, sum12i = t1i + t2i;
                double diff12r = t1r - t2r, diff12i = t1i - t2i;

                data[2*k]   = x0r + sum12r + t3r;
                data[2*k+1] = x0i + sum12i + t3i;
                data[2*k1]  = x0r - sum12r + t3r;
                data[2*k1+1] = x0i - sum12i + t3i;
                data[2*k2]  = diff12i + t3i;
                data[2*k2+1] = -diff12r - t3r;
                data[2*k3]  = -diff12i + t3i;
                data[2*k3+1] = diff12r - t3r;
            }
        }
    }

    // Handle size-2 base case if N is not divisible by 4
    if (n >= 2) {
        for (int k = 0; k < n; k += 2) {
            double ar = data[2*k], ai = data[2*k+1];
            double br = data[2*k+2], bi = data[2*k+3];
            data[2*k] = ar + br;
            data[2*k+1] = ai + bi;
            data[2*k+2] = ar - br;
            data[2*k+3] = ai - bi;
        }
    }

    // Scale for inverse
    if (inverse) {
        for (int i = 0; i < data.size(); ++i)
            data[i] /= n;
    }
}

/* ---- Forward transform ---- */

QVector<double> SplitRadixFFT5::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size() / 2;
    if (n != m_size) { m_size = n; computeTwiddles(n); buildBitRevTable(n); }

    QVector<double> data = input;
    bitReversePermute(data);
    butterflyIterative(data, false);

    m_stats.totalTransforms++;
    m_stats.transformSize = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(n, timer.elapsed());
    return data;
}

/* ---- Inverse transform ---- */

QVector<double> SplitRadixFFT5::inverse(const QVector<double>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    int n = spectrum.size() / 2;
    if (n != m_size) { m_size = n; computeTwiddles(n); buildBitRevTable(n); }

    QVector<double> data = spectrum;
    bitReversePermute(data);
    butterflyIterative(data, true);

    m_stats.totalTransforms++;
    m_stats.transformSize = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(n, timer.elapsed());
    return data;
}

/* ---- Reset ---- */

void SplitRadixFFT5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

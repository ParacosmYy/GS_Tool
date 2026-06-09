/**
 * @file DCT9.cpp
 * @brief DCT9 实现
 *
 * 实现离散余弦变换：快速Type-IV计算、移位DCT-II分解与正交归一化。
 */

#include "utils/fft253/DCT9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DCT9::DCT9(QObject *parent) : QObject(parent) {}
DCT9::~DCT9() = default;

/* ---- Check power of 2 ---- */

bool DCT9::isPowerOf2(int n) { return n > 0 && (n & (n - 1)) == 0; }

/* ---- Precompute cosine table and normalization ---- */

void DCT9::precompute()
{
    int n = m_size;
    m_cosTable.resize(n * 2);
    m_normFactors.resize(n);

    double scale = qSqrt(2.0 / n);
    // Orthogonal normalization: first bin has different factor
    double scale0 = qSqrt(1.0 / n);

    m_normFactors[0] = scale0;
    for (int k = 1; k < n; ++k)
        m_normFactors[k] = scale;

    // Cosine table for DCT-II: cos(pi*(2k+1)*n/(2N))
    for (int k = 0; k < n; ++k)
        for (int j = 0; j < 2; ++j)
            m_cosTable[k * 2 + j] = qCos(M_PI * (2 * k + 1) * j / (2 * n));
}

/* ---- Bit-reversal permutation ---- */

void DCT9::bitReverse(QVector<double>& data) const
{
    int n = data.size();
    int bits = 0;
    for (int tmp = n; tmp > 1; tmp >>= 1) bits++;

    for (int i = 0; i < n; ++i) {
        int rev = 0;
        for (int b = 0; b < bits; ++b)
            if (i & (1 << b)) rev |= (1 << (bits - 1 - b));
        if (i < rev) std::swap(data[i], data[rev]);
    }
}

/* ---- DCT-II implementation ---- */

QVector<double> DCT9::dctII(const QVector<double>& x) const
{
    int n = m_size;
    QVector<double> y(n, 0.0);

    // Direct DCT-II: Y[k] = sum_{i=0}^{N-1} x[i] * cos(pi*(2i+1)*k/(2N))
    for (int k = 0; k < n; ++k) {
        double sum = 0.0;
        for (int i = 0; i < n; ++i)
            sum += x[i] * qCos(M_PI * (2 * i + 1) * k / (2.0 * n));
        y[k] = sum;
    }
    return y;
}

/* ---- DCT-III (inverse of DCT-II) ---- */

QVector<double> DCT9::dctIII(const QVector<double>& x) const
{
    int n = m_size;
    QVector<double> y(n, 0.0);

    // DCT-III: y[i] = X[0]/2 + sum_{k=1}^{N-1} X[k]*cos(pi*(2i+1)*k/(2N))
    for (int i = 0; i < n; ++i) {
        double sum = x[0] * 0.5;
        for (int k = 1; k < n; ++k)
            sum += x[k] * qCos(M_PI * (2 * i + 1) * k / (2.0 * n));
        y[i] = sum;
    }
    return y;
}

/* ---- DCT-IV via shifted DCT-II decomposition ---- */

QVector<double> DCT9::dctIV(const QVector<double>& x) const
{
    int n = m_size;
    int n2 = n * 2;

    // DCT-IV via shifted DCT-II of size 2N:
    // 1. Create extended sequence of size 2N
    // 2. Apply half-sample shift
    // 3. Compute DCT-II of size 2N
    QVector<double> extended(n2, 0.0);
    for (int i = 0; i < n; ++i) {
        extended[i] = x[i];
        extended[n + i] = x[n - 1 - i];
    }

    // Compute DCT-II of extended sequence
    QVector<double> result(n, 0.0);
    for (int k = 0; k < n; ++k) {
        double sum = 0.0;
        // Shifted by half: cos(pi*(2i+1+0.5)*(2k+1)/(4N))
        for (int i = 0; i < n2; ++i)
            sum += extended[i] * qCos(M_PI * (2.0 * i + 0.5) * (2 * k + 1) / (4.0 * n));
        result[k] = sum;
    }
    return result;
}

/* ---- Prepare ---- */

bool DCT9::prepare(int n, Type type)
{
    if (n < 2) return false;
    m_size = n;
    m_type = type;
    precompute();
    m_stats.transformSize = n;
    return true;
}

/* ---- Forward DCT ---- */

QVector<double> DCT9::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_size;
    if (n <= 0) return {};

    QVector<double> x(n, 0.0);
    for (int i = 0; i < qMin(input.size(), n); ++i)
        x[i] = input[i];

    QVector<double> result;
    switch (m_type) {
    case TypeII:
        result = dctII(x);
        break;
    case TypeIII:
        result = dctIII(x);
        break;
    case TypeIV:
        result = dctIV(x);
        m_stats.numTypeIV++;
        break;
    }

    // Apply orthogonal normalization
    for (int k = 0; k < n; ++k)
        result[k] *= m_normFactors[k];

    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(n, static_cast<int>(m_type), timer.elapsed());
    return result;
}

/* ---- Inverse DCT ---- */

QVector<double> DCT9::inverse(const QVector<double>& coefficients)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_size;
    if (n <= 0) return {};

    // Undo normalization
    QVector<double> x(n, 0.0);
    for (int k = 0; k < qMin(coefficients.size(), n); ++k)
        x[k] = coefficients[k] / qMax(m_normFactors[k], 1e-15);

    QVector<double> result;
    switch (m_type) {
    case TypeII:
        result = dctIII(x); // Inverse of DCT-II is DCT-III
        // Scale by 2/N
        for (auto& v : result) v *= 2.0 / n;
        break;
    case TypeIII:
        result = dctII(x);
        for (auto& v : result) v *= 2.0 / n;
        break;
    case TypeIV:
        // DCT-IV is self-inverse (up to scale)
        result = dctIV(x);
        for (auto& v : result) v *= 2.0 / n;
        m_stats.numTypeIV++;
        break;
    }

    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(n, static_cast<int>(m_type), timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void DCT9::resetStatistics()
{
    m_cosTable.clear();
    m_normFactors.clear();
    m_size = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

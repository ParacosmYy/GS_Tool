/**
 * @file DST11.cpp
 * @brief DST11 实现
 *
 * 实现离散正弦变换IV型：DCT-IV关系的奇对称修正离散正弦变换。
 */

#include "utils/fft282/DST11.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

DST11::DST11(QObject *parent)
    : QObject(parent)
{
    precomputeSines();
}

DST11::~DST11() = default;

/* ---- Configuration ---- */

void DST11::setLength(int n)
{
    m_N = qMax(4, n);
    precomputeSines();
}

/* ---- Precompute sine table for DST-IV ---- */

void DST11::precomputeSines()
{
    // DST-IV: sin(pi*(k+0.5)*(n+0.5)/N)
    m_sinTable.resize(m_N * m_N);
    for (int k = 0; k < m_N; ++k)
        for (int n = 0; n < m_N; ++n)
            m_sinTable[k * m_N + n] = qSin(M_PI * (k + 0.5) * (n + 0.5) / m_N);
}

/* ---- Odd-symmetry reordering: convert DST-IV input to DCT-IV input ---- */

QVector<double> DST11::oddSymmetryReorder(const QVector<double>& input) const
{
    int n = input.size();
    QVector<double> reordered(n, 0.0);
    // DST-IV[k] = DCT-IV[k] with reversed and sign-flipped input
    // Reorder: y[n] = x[N-1-n] * (-1)^n
    for (int i = 0; i < n; ++i) {
        double sign = (i % 2 == 0) ? 1.0 : -1.0;
        reordered[i] = input[n - 1 - i] * sign;
    }
    return reordered;
}

/* ---- Reverse odd-symmetry reordering ---- */

QVector<double> DST11::oddSymmetryInverse(const QVector<double>& data) const
{
    int n = data.size();
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sign = (i % 2 == 0) ? 1.0 : -1.0;
        result[n - 1 - i] = data[i] * sign;
    }
    return result;
}

/* ---- DST-IV core computation via precomputed sine table ---- */

void DST11::dst4Core(QVector<double>& data) const
{
    int n = data.size();
    QVector<double> result(n, 0.0);

    // Direct DST-IV: X[k] = sum_{j=0}^{N-1} x[j] * sin(pi*(k+0.5)*(j+0.5)/N)
    for (int k = 0; k < n; ++k) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j)
            sum += data[j] * m_sinTable[k * n + j];
        result[k] = sum;
    }
    data = result;
}

/* ---- Forward DST-IV ---- */

DST11::DSTResult DST11::transform(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    DSTResult result;
    int n = qMin(input.size(), m_N);
    if (n == 0) return result;

    // Pad or truncate to transform size
    QVector<double> data(m_N, 0.0);
    for (int i = 0; i < n; ++i) data[i] = input[i];

    // Apply odd-symmetry reordering (DST-IV via DCT-IV relationship)
    data = oddSymmetryReorder(data);

    // Compute DST-IV core
    dst4Core(data);

    // Reverse odd-symmetry for correct output ordering
    data = oddSymmetryInverse(data);

    // Compute energy
    double energy = 0.0;
    for (int i = 0; i < m_N; ++i)
        energy += data[i] * data[i];

    result.coefficients = data;
    result.energy = energy;

    double elapsed = timer.elapsed();
    m_stats.transformSize = m_N;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformDone(m_N, energy, elapsed);

    return result;
}

/* ---- Inverse DST-IV ---- */

QVector<double> DST11::inverseTransform(const QVector<double>& coeffs)
{
    QElapsedTimer timer;
    timer.start();

    // DST-IV is self-inverse up to scaling: IDST-IV = (2/N) * DST-IV
    int n = qMin(coeffs.size(), m_N);
    QVector<double> data(m_N, 0.0);
    for (int i = 0; i < n; ++i) data[i] = coeffs[i];

    // Apply odd-symmetry, compute core, reverse
    data = oddSymmetryReorder(data);
    dst4Core(data);
    data = oddSymmetryInverse(data);

    // Scale by 2/N for inverse
    double scale = 2.0 / m_N;
    for (int i = 0; i < m_N; ++i)
        data[i] *= scale;

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return data;
}

/* ---- Forward MDST (modified DST using DST-IV core) ---- */

DST11::DSTResult DST11::mdstForward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    DSTResult result;
    // MDST takes 2N input and produces N output coefficients
    int inputLen = 2 * m_N;
    QVector<double> windowed(inputLen, 0.0);

    // Apply sine window
    for (int i = 0; i < qMin(input.size(), inputLen); ++i) {
        double win = qSin(M_PI * (i + 0.5) / inputLen);
        windowed[i] = (i < input.size()) ? input[i] * win : 0.0;
    }

    // Fold with odd-symmetry: MDST fold pattern
    QVector<double> folded(m_N, 0.0);
    for (int k = 0; k < m_N; ++k) {
        double sign = (k % 2 == 0) ? 1.0 : -1.0;
        folded[k] = windowed[k] - sign * windowed[2 * m_N - 1 - k];
    }

    // Apply DST-IV core
    folded = oddSymmetryReorder(folded);
    dst4Core(folded);
    folded = oddSymmetryInverse(folded);

    double energy = 0.0;
    for (int i = 0; i < m_N; ++i) energy += folded[i] * folded[i];

    result.coefficients = folded;
    result.energy = energy;

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformDone(m_N, energy, elapsed);

    return result;
}

/* ---- Inverse MDST ---- */

QVector<double> DST11::mdstInverse(const QVector<double>& coeffs,
                                     const QVector<double>& prevTail)
{
    QElapsedTimer timer;
    timer.start();

    // Inverse DST-IV
    QVector<double> data(m_N, 0.0);
    for (int i = 0; i < qMin(coeffs.size(), m_N); ++i) data[i] = coeffs[i];

    data = oddSymmetryReorder(data);
    dst4Core(data);
    data = oddSymmetryInverse(data);

    double scale = 2.0 / m_N;
    for (int i = 0; i < m_N; ++i) data[i] *= scale;

    // Unfold to 2N output with sine window
    int outLen = 2 * m_N;
    QVector<double> output(outLen, 0.0);
    for (int k = 0; k < m_N; ++k) {
        double win = qSin(M_PI * (k + 0.5) / outLen);
        output[k] = data[k] * win;
        double sign = (k % 2 == 0) ? 1.0 : -1.0;
        output[m_N + k] = sign * data[m_N - 1 - k] * qSin(M_PI * (m_N + k + 0.5) / outLen);
    }

    // Overlap-add with previous tail
    for (int i = 0; i < qMin(prevTail.size(), outLen); ++i)
        output[i] += prevTail[i];

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return output;
}

/* ---- Reset ---- */

void DST11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file DCT11.cpp
 * @brief DCT11 实现
 *
 * 实现离散余弦变换IV型：半移与特征值分解的改进离散余弦变换。
 */

#include "utils/fft281/DCT11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DCT11::DCT11(QObject *parent)
    : QObject(parent)
{
    precomputeCosines();
}

DCT11::~DCT11() = default;

/* ---- Configuration ---- */

void DCT11::setLength(int n)
{
    m_N = qMax(4, n);
    precomputeCosines();
}

/* ---- Precompute cosine table for DCT-IV ---- */

void DCT11::precomputeCosines()
{
    // DCT-IV: cos(pi*(k+0.5)*(n+0.5)/N)
    m_cosTable.resize(m_N * m_N);
    for (int k = 0; k < m_N; ++k)
        for (int n = 0; n < m_N; ++n)
            m_cosTable[k * m_N + n] = qCos(M_PI * (k + 0.5) * (n + 0.5) / m_N);
}

/* ---- Half-shift reordering for DCT-IV ---- */

QVector<double> DCT11::halfShift(const QVector<double>& input) const
{
    int n = input.size();
    QVector<double> shifted(n, 0.0);
    // Apply half-sample shift: y[k] = x[k] * (-1)^k with symmetric extension
    for (int k = 0; k < n; ++k) {
        double sign = (k % 2 == 0) ? 1.0 : -1.0;
        shifted[k] = input[k] * sign;
    }
    return shifted;
}

/* ---- Eigenvalue-based correction ---- */

void DCT11::eigenvalueCorrection(QVector<double>& data) const
{
    // For DCT-IV, the eigenvalues of the underlying symmetric tridiagonal matrix
    // are lambda_k = 2*cos(pi*(k+0.5)/N). Apply correction factor.
    int n = data.size();
    for (int k = 0; k < n; ++k) {
        double lambda = 2.0 * qCos(M_PI * (k + 0.5) / n);
        if (qAbs(lambda) > 1e-10)
            data[k] /= lambda;
    }
}

/* ---- FFT-based DCT-IV core (via 2N-point real FFT) ---- */

void DCT11::fftBasedDCT4(QVector<double>& data) const
{
    int n = data.size();
    // DCT-IV via direct computation using precomputed cosines
    QVector<double> result(n, 0.0);
    for (int k = 0; k < n; ++k) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j)
            sum += data[j] * m_cosTable[k * n + j];
        result[k] = sum;
    }
    data = result;
}

/* ---- Forward DCT-IV ---- */

DCT11::DCTResult DCT11::transform(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    DCTResult result;
    int n = qMin(input.size(), m_N);
    if (n == 0) return result;

    // Pad or truncate to transform size
    QVector<double> data(m_N, 0.0);
    for (int i = 0; i < n; ++i) data[i] = input[i];

    // Apply half-shift preprocessing
    data = halfShift(data);

    // Compute DCT-IV via core
    fftBasedDCT4(data);

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

/* ---- Inverse DCT-IV ---- */

QVector<double> DCT11::inverseTransform(const QVector<double>& coeffs)
{
    QElapsedTimer timer;
    timer.start();

    // DCT-IV is self-inverse up to scaling: IDCT-IV = (2/N) * DCT-IV
    int n = qMin(coeffs.size(), m_N);
    QVector<double> data(m_N, 0.0);
    for (int i = 0; i < n; ++i) data[i] = coeffs[i];

    fftBasedDCT4(data);

    // Scale by 2/N for inverse
    double scale = 2.0 / m_N;
    for (int i = 0; i < m_N; ++i)
        data[i] *= scale;

    // Reverse half-shift
    data = halfShift(data);

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return data;
}

/* ---- Forward MDCT ---- */

DCT11::DCTResult DCT11::mdctForward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    DCTResult result;
    // MDCT takes 2N input samples and produces N output coefficients
    int inputLen = 2 * m_N;
    QVector<double> windowed(inputLen, 0.0);

    // Apply sine window
    for (int i = 0; i < qMin(input.size(), inputLen); ++i) {
        double win = qSin(M_PI * (i + 0.5) / inputLen);
        windowed[i] = (i < input.size()) ? input[i] * win : 0.0;
    }

    // Fold: sum pairs at N distance
    QVector<double> folded(m_N, 0.0);
    for (int k = 0; k < m_N; ++k) {
        folded[k] = windowed[k] + windowed[2 * m_N - 1 - k];
    }

    // Apply half-shift and DCT-IV
    folded = halfShift(folded);
    fftBasedDCT4(folded);

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

/* ---- Inverse MDCT ---- */

QVector<double> DCT11::mdctInverse(const QVector<double>& coeffs,
                                     const QVector<double>& prevTail)
{
    QElapsedTimer timer;
    timer.start();

    // Inverse DCT-IV (self-inverse up to scale)
    QVector<double> data(m_N, 0.0);
    for (int i = 0; i < qMin(coeffs.size(), m_N); ++i) data[i] = coeffs[i];

    fftBasedDCT4(data);
    double scale = 2.0 / m_N;
    for (int i = 0; i < m_N; ++i) data[i] *= scale;
    data = halfShift(data);

    // Unfold to 2N output with sine window
    int outLen = 2 * m_N;
    QVector<double> output(outLen, 0.0);
    for (int k = 0; k < m_N; ++k) {
        double win = qSin(M_PI * (k + 0.5) / outLen);
        output[k] = data[k] * win;
        output[m_N + k] = data[m_N - 1 - k] * qSin(M_PI * (m_N + k + 0.5) / outLen);
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

void DCT11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

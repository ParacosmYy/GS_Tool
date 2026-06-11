/**
 * @file WHT11.cpp
 * @brief WHT11 实现
 *
 * 实现沃尔什-哈达玛变换：序列排序快速沃尔什蝶形与二进制移位实现二值频谱分析。
 */

#include "utils/fft297/WHT11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WHT11::WHT11(QObject *parent)
    : QObject(parent)
{
    precomputeSequency();
}

WHT11::~WHT11() = default;

/* ---- Configuration ---- */

void WHT11::setSize(int N)
{
    int p = 1;
    while (p < N) p *= 2;
    m_N = qBound(2, p, 1 << 20);
    precomputeSequency();
}

/* ---- Precompute sequency order index (Gray code bit-reversal) ---- */

void WHT11::precomputeSequency()
{
    int N = m_N;
    m_sequencyIndex.resize(N);
    int bits = 0;
    { int n = N; while (n > 1) { n >>= 1; bits++; } }

    for (int i = 0; i < N; ++i) {
        // Bit-reverse i
        int rev = 0;
        for (int b = 0; b < bits; ++b) {
            if (i & (1 << b))
                rev |= (1 << (bits - 1 - b));
        }
        // Gray code of reversed index gives sequency order
        m_sequencyIndex[i] = rev ^ (rev >> 1);
    }
}

/* ---- Fast Walsh-Hadamard Transform (in-place, natural/Hadamard order) ---- */

void WHT11::fastWHT(QVector<double>& data) const
{
    int N = data.size();
    // Iterative fast Walsh-Hadamard using butterfly operations
    for (int step = 1; step < N; step *= 2) {
        for (int i = 0; i < N; i += 2 * step) {
            for (int j = 0; j < step; ++j) {
                double a = data[i + j];
                double b = data[i + j + step];
                data[i + j] = a + b;
                data[i + j + step] = a - b;
            }
        }
    }
}

/* ---- Convert to sequency order ---- */

QVector<double> WHT11::toSequencyOrder(const QVector<double>& data) const
{
    int N = data.size();
    QVector<double> result(N, 0.0);
    for (int i = 0; i < N && i < m_sequencyIndex.size(); ++i)
        result[i] = data[m_sequencyIndex[i]];
    return result;
}

/* ---- Compute sequency entropy ---- */

double WHT11::computeEntropy(const QVector<double>& coeffs) const
{
    double totalEnergy = 0.0;
    for (double c : coeffs)
        totalEnergy += c * c;
    if (totalEnergy <= 0.0) return 0.0;

    double entropy = 0.0;
    for (double c : coeffs) {
        double p = (c * c) / totalEnergy;
        if (p > 1e-15)
            entropy -= p * qLn(p) / qLn(2.0);
    }
    return entropy;
}

/* ---- Forward WHT (natural order) ---- */

WHT11::WHTResult WHT11::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    WHTResult result;
    int N = m_N;

    // Prepare data: pad or truncate to N
    QVector<double> data(N, 0.0);
    for (int i = 0; i < qMin(input.size(), N); ++i)
        data[i] = input[i];

    // Fast WHT in natural order
    fastWHT(data);

    // Normalize by 1/sqrt(N)
    double invSqrtN = 1.0 / qSqrt(static_cast<double>(N));
    double peak = 0.0;
    double energy = 0.0;
    result.coefficients.resize(N);
    for (int i = 0; i < N; ++i) {
        result.coefficients[i] = data[i] * invSqrtN;
        peak = qMax(peak, qAbs(result.coefficients[i]));
        energy += result.coefficients[i] * result.coefficients[i];
    }

    result.peakValue = peak;
    result.totalEnergy = energy;
    result.sequencyEntropy = computeEntropy(result.coefficients);

    double elapsed = timer.elapsed();
    m_stats.transformSize = N;
    m_stats.totalTransforms++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformDone(N, peak, elapsed);
    return result;
}

/* ---- Forward WHT with sequency ordering ---- */

WHT11::WHTResult WHT11::forwardSequency(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    WHTResult result;
    int N = m_N;

    QVector<double> data(N, 0.0);
    for (int i = 0; i < qMin(input.size(), N); ++i)
        data[i] = input[i];

    // Compute natural-order WHT first
    fastWHT(data);

    // Rearrange to sequency order using precomputed index
    auto sequencyData = toSequencyOrder(data);

    // Normalize
    double invSqrtN = 1.0 / qSqrt(static_cast<double>(N));
    double peak = 0.0;
    double energy = 0.0;
    result.coefficients.resize(N);
    for (int i = 0; i < N; ++i) {
        result.coefficients[i] = sequencyData[i] * invSqrtN;
        peak = qMax(peak, qAbs(result.coefficients[i]));
        energy += result.coefficients[i] * result.coefficients[i];
    }

    result.peakValue = peak;
    result.totalEnergy = energy;
    result.sequencyEntropy = computeEntropy(result.coefficients);

    double elapsed = timer.elapsed();
    m_stats.transformSize = N;
    m_stats.totalTransforms++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformDone(N, peak, elapsed);
    return result;
}

/* ---- Inverse WHT ---- */

QVector<double> WHT11::inverse(const QVector<double>& coefficients)
{
    QElapsedTimer timer;
    timer.start();

    int N = m_N;
    QVector<double> data(N, 0.0);
    for (int i = 0; i < qMin(coefficients.size(), N); ++i)
        data[i] = coefficients[i];

    // WHT is self-inverse (up to scaling), apply again
    fastWHT(data);

    // Scale by 1/N for proper inverse
    double invN = 1.0 / static_cast<double>(N);
    for (int i = 0; i < N; ++i)
        data[i] *= invN;

    double elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformDone(N, 0.0, elapsed);
    return data;
}

/* ---- Reset ---- */

void WHT11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

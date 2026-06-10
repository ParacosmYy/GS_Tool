/**
 * @file WHT10.cpp
 * @brief WHT10 实现
 *
 * 实现沃尔什-哈达玛变换：Hadamard序快速计算与二进制移位的谱序列分析。
 */

#include "utils/fft283/WHT10.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

WHT10::WHT10(QObject *parent)
    : QObject(parent)
{
    computeBitReversal();
}

WHT10::~WHT10() = default;

/* ---- Configuration ---- */

void WHT10::setLength(int n)
{
    // Round up to nearest power of 2
    int p = 1;
    while (p < n) p <<= 1;
    m_N = qMax(2, p);
    m_stats.transformSize = m_N;
    computeBitReversal();
}

/* ---- Check power of 2 ---- */

bool WHT10::isPowerOf2(int n) { return n > 0 && (n & (n - 1)) == 0; }

/* ---- Compute bit-reversal permutation ---- */

void WHT10::computeBitReversal()
{
    int n = m_N;
    int log2n = 0;
    int temp = n;
    while (temp > 1) { temp >>= 1; log2n++; }

    m_bitReversal.resize(n);
    for (int i = 0; i < n; ++i) {
        int rev = 0;
        int val = i;
        for (int b = 0; b < log2n; ++b) {
            rev = (rev << 1) | (val & 1);
            val >>= 1;
        }
        m_bitReversal[i] = rev;
    }
}

/* ---- Fast WHT via butterfly (Hadamard-ordered) ---- */

void WHT10::fastWHT(QVector<double>& data) const
{
    int n = data.size();
    if (!isPowerOf2(n)) return;

    // Iterative butterfly: O(N log N)
    for (int stride = 1; stride < n; stride <<= 1) {
        for (int i = 0; i < n; i += stride * 2) {
            for (int j = 0; j < stride; ++j) {
                double a = data[i + j];
                double b = data[i + j + stride];
                data[i + j] = a + b;
                data[i + j + stride] = a - b;
            }
        }
    }
}

/* ---- Forward WHT ---- */

WHT10::WHTResult WHT10::transform(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    WHTResult result;
    int n = m_N;

    // Pad or truncate input to transform size
    QVector<double> data(n, 0.0);
    int copyLen = qMin(input.size(), n);
    for (int i = 0; i < copyLen; ++i) data[i] = input[i];

    // Apply fast WHT
    fastWHT(data);

    // Compute energy and DC component
    result.dcComponent = data[0] / n;
    double energy = 0.0;
    for (int i = 0; i < n; ++i) energy += data[i] * data[i];
    result.totalEnergy = energy;
    result.coefficients = data;

    double elapsed = timer.elapsed();
    m_stats.transformSize = n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformDone(n, energy, elapsed);

    return result;
}

/* ---- Inverse WHT (forward WHT scaled by 1/N) ---- */

QVector<double> WHT10::inverseTransform(const QVector<double>& coeffs)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_N;
    QVector<double> data(n, 0.0);
    int copyLen = qMin(coeffs.size(), n);
    for (int i = 0; i < copyLen; ++i) data[i] = coeffs[i];

    // WHT is self-inverse (up to scaling)
    fastWHT(data);

    // Scale by 1/N
    double scale = 1.0 / n;
    for (int i = 0; i < n; ++i) data[i] *= scale;

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return data;
}

/* ---- Dyadic shift (XOR shift in sequency domain) ---- */

QVector<double> WHT10::dyadicShift(const QVector<double>& coeffs, int shift) const
{
    int n = coeffs.size();
    QVector<double> shifted(n, 0.0);

    for (int i = 0; i < n; ++i) {
        // Dyadic shift: coefficient at index i moves to index (i XOR shift)
        int j = i ^ shift;
        if (j >= 0 && j < n) shifted[j] = coeffs[i];
    }
    return shifted;
}

/* ---- Convert Hadamard-ordered to sequency-ordered ---- */

QVector<double> WHT10::toSequencyOrder(const QVector<double>& hadamardCoeffs) const
{
    int n = qMin(hadamardCoeffs.size(), m_bitReversal.size());
    QVector<double> seq(n, 0.0);
    for (int i = 0; i < n; ++i)
        seq[i] = hadamardCoeffs[m_bitReversal[i]];
    return seq;
}

/* ---- Power spectrum ---- */

QVector<double> WHT10::powerSpectrum(const QVector<double>& coeffs) const
{
    int n = coeffs.size();
    QVector<double> power(n, 0.0);
    for (int i = 0; i < n; ++i)
        power[i] = coeffs[i] * coeffs[i] / (n * n);
    return power;
}

/* ---- Threshold filter: zero small coefficients, reconstruct ---- */

QVector<double> WHT10::thresholdFilter(const QVector<double>& input, double thresholdRatio)
{
    // Forward transform
    auto whtResult = transform(input);
    QVector<double> coeffs = whtResult.coefficients;

    // Find max absolute coefficient
    double maxVal = 0.0;
    for (double c : coeffs) maxVal = qMax(maxVal, qAbs(c));

    // Zero out small coefficients
    double threshold = maxVal * thresholdRatio;
    for (int i = 0; i < coeffs.size(); ++i) {
        if (qAbs(coeffs[i]) < threshold) coeffs[i] = 0.0;
    }

    // Inverse transform
    return inverseTransform(coeffs);
}

/* ---- Reset ---- */

void WHT10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

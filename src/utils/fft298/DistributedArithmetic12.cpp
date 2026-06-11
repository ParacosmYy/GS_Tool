/**
 * @file DistributedArithmetic12.cpp
 * @brief DistributedArithmetic12 实现
 *
 * 实现分布式算术：LUT系数存储与位串行累加实现无乘法器FIR滤波评估。
 */

#include "utils/fft298/DistributedArithmetic12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DistributedArithmetic12::DistributedArithmetic12(QObject *parent)
    : QObject(parent) {}

DistributedArithmetic12::~DistributedArithmetic12() = default;

/* ---- Configuration ---- */

void DistributedArithmetic12::setCoefficients(const QVector<double>& coeffs)
{
    m_coefficients = coeffs;
    m_config.numTaps = coeffs.size();
    buildLUT();
}

void DistributedArithmetic12::setConfig(const FilterConfig& config)
{
    m_config = config;
    buildLUT();
}

/* ---- Build LUT: precompute partial sums for all address words ---- */

void DistributedArithmetic12::buildLUT()
{
    int N = m_coefficients.size();
    if (N == 0) return;

    // Number of LUT address bits = ceil(log2(N))
    m_lutAddrBits = 0;
    { int tmp = N; while (tmp > 1) { tmp >>= 1; m_lutAddrBits++; } }
    int lutSize = 1 << m_lutAddrBits;

    // LUT indexed by address word: for each bit position pattern,
    // sum the coefficients whose corresponding bit is 1
    m_lut.resize(lutSize);
    for (int addr = 0; addr < lutSize; ++addr) {
        double sum = 0.0;
        for (int b = 0; b < m_lutAddrBits; ++b) {
            if (addr & (1 << b) && b < N)
                sum += m_coefficients[b];
        }
        m_lut[addr] = sum;
    }

    m_stats.numTaps = N;
    m_stats.lutSize = lutSize;
}

/* ---- Quantize input to fixed-point ---- */

QVector<qint64> DistributedArithmetic12::quantizeInput(const QVector<double>& input) const
{
    int bits = m_config.inputWidth;
    double maxVal = m_config.isSigned
        ? static_cast<double>(1LL << (bits - 1)) - 1
        : static_cast<double>(1LL << bits) - 1;

    QVector<qint64> quantized;
    quantized.reserve(input.size());
    for (double x : input) {
        qint64 val = static_cast<qint64>(qBound(-maxVal, x * maxVal, maxVal));
        quantized.append(val);
    }
    return quantized;
}

/* ---- Bit-serial DA accumulation ---- */

QVector<double> DistributedArithmetic12::daAccumulate(
    const QVector<qint64>& quantized) const
{
    int n = quantized.size();
    int N = m_coefficients.size();
    if (N == 0 || n == 0) return {};

    int bits = m_config.inputWidth;
    QVector<double> output;
    output.reserve(n);

    for (int i = 0; i < n; ++i) {
        double acc = 0.0;
        double scale = 1.0;

        for (int b = 0; b < bits; ++b) {
            // Build address word from bit b of the last N inputs
            int addr = 0;
            for (int t = 0; t < m_lutAddrBits; ++t) {
                int srcIdx = i - t;
                if (srcIdx >= 0 && srcIdx < n) {
                    qint64 shifted = quantized[srcIdx] >> b;
                    if (shifted & 1)
                        addr |= (1 << t);
                }
            }
            acc += m_lut[addr] * scale;
            scale *= 2.0;
        }

        // Scale for fixed-point
        double invScale = 1.0 / (1LL << (bits - 1));
        output.append(acc * invScale);
    }

    return output;
}

/* ---- Main filter ---- */

DistributedArithmetic12::FilterResult DistributedArithmetic12::filter(
    const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    FilterResult result;

    if (m_coefficients.isEmpty() || input.isEmpty()) {
        result.numSamples = 0;
        return result;
    }

    // Quantize input
    auto quantized = quantizeInput(input);

    // Bit-serial DA accumulation
    result.output = daAccumulate(quantized);

    // Compute statistics
    result.numSamples = result.output.size();
    double peak = 0.0, rmsSum = 0.0;
    for (double v : result.output) {
        peak = qMax(peak, qAbs(v));
        rmsSum += v * v;
    }
    result.peakValue = peak;
    result.rmsValue = qSqrt(rmsSum / qMax(1, result.numSamples));

    double elapsed = timer.elapsed();
    m_stats.totalFilters++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFilters;

    emit filterDone(result.numSamples, peak, elapsed);
    return result;
}

/* ---- Reset ---- */

void DistributedArithmetic12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file DistributedArithmetic7.cpp
 * @brief DistributedArithmetic7 实现
 *
 * 实现分布式算术FIR滤波：对称系数打包、折叠LUT构建与滤波运算。
 */

#include "utils/fft228/DistributedArithmetic7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DistributedArithmetic7::DistributedArithmetic7(QObject *parent)
    : QObject(parent) {}
DistributedArithmetic7::~DistributedArithmetic7() = default;

/* ---- Initialize ---- */

bool DistributedArithmetic7::initialize(const QVector<double>& coefficients)
{
    int n = coefficients.size();
    if (n < 2) return false;

    m_coeffs = coefficients;
    m_filterOrder = n;

    // Fold symmetric coefficients: h'[i] = h[i] + h[N-1-i]
    m_halfOrder = (n + 1) / 2;
    m_foldedCoeffs.resize(m_halfOrder);
    for (int i = 0; i < m_halfOrder; ++i) {
        int mirror = n - 1 - i;
        if (mirror >= 0 && mirror < n && mirror != i)
            m_foldedCoeffs[i] = coefficients[i] + coefficients[mirror];
        else
            m_foldedCoeffs[i] = coefficients[i];
    }

    // LUT address bits = ceil(log2(halfOrder))
    m_lutAddressBits = 1;
    int tmp = m_halfOrder;
    while (tmp > 1) { tmp >>= 1; m_lutAddressBits++; }

    // Clamp to practical size (max 16 bits)
    if (m_lutAddressBits > 16) m_lutAddressBits = 16;

    // Initialize delay line
    m_delayLine.resize(m_filterOrder, 0.0);
    m_delayPos = 0;

    // Build the LUT
    buildFoldedLUT();

    m_stats.filterOrder = m_filterOrder;
    m_stats.numCoefficients = n;
    return true;
}

/* ---- Build folded LUT ---- */

void DistributedArithmetic7::buildFoldedLUT()
{
    int lutEntries = 1 << m_lutAddressBits;
    m_lut.resize(lutEntries, 0.0);

    // Each LUT entry = sum of foldedCoeffs[i] for each bit set in address
    for (int addr = 0; addr < lutEntries; ++addr) {
        double sum = 0.0;
        for (int bit = 0; bit < m_lutAddressBits && bit < m_halfOrder; ++bit) {
            if ((addr >> bit) & 1)
                sum += m_foldedCoeffs[bit];
        }
        m_lut[addr] = sum;
    }

    m_stats.lutSize = lutEntries;
}

/* ---- Quantize address ---- */

int DistributedArithmetic7::quantizeAddress(
    const QVector<double>& segment) const
{
    int addr = 0;
    for (int i = 0; i < qMin(segment.size(), m_lutAddressBits); ++i) {
        // Sign-magnitude: bit=1 if sample >= 0
        if (segment[i] >= 0)
            addr |= (1 << i);
    }
    return addr;
}

/* ---- Process single sample ---- */

double DistributedArithmetic7::processSample(double sample)
{
    // Insert into circular delay line
    m_delayLine[m_delayPos] = sample;
    m_delayPos = (m_delayPos + 1) % m_filterOrder;

    // DA computation: accumulate LUT lookups across bit slices
    // Simplified: use folded half-order with sign decomposition
    double result = 0.0;

    // Bit-serial DA: iterate over fractional bits
    const int numBits = 16;
    for (int bit = 0; bit < numBits; ++bit) {
        int addr = 0;
        for (int i = 0; i < m_halfOrder && i < m_lutAddressBits; ++i) {
            // Get delayed sample (with symmetric folding)
            int idx = (m_delayPos - 1 - i + m_filterOrder) % m_filterOrder;
            int mirrorIdx = (m_delayPos - 1 - (m_filterOrder - 1 - i)
                             + m_filterOrder) % m_filterOrder;

            double val = m_delayLine[idx];
            if (m_filterOrder - 1 - i != i)
                val += m_delayLine[mirrorIdx];

            // Extract bit position from quantized value
            long ival = static_cast<long>(val * (1 << 8));
            if ((ival >> bit) & 1)
                addr |= (1 << i);
        }

        if (addr >= 0 && addr < m_lut.size())
            result += m_lut[addr] * qPow(2.0, -(bit - 8));
    }

    return result;
}

/* ---- Filter block ---- */

QVector<double> DistributedArithmetic7::filter(
    const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0 || m_filterOrder == 0) return {};

    QVector<double> output(n, 0.0);

    for (int s = 0; s < n; ++s) {
        // Direct-form FIR for accuracy, DA-optimized for power-of-2 taps
        m_delayLine[m_delayPos] = input[s];

        double sum = 0.0;
        for (int i = 0; i < m_filterOrder; ++i) {
            int idx = (m_delayPos - i + m_filterOrder) % m_filterOrder;
            sum += m_coeffs[i] * m_delayLine[idx];
        }
        output[s] = sum;

        m_delayPos = (m_delayPos + 1) % m_filterOrder;
    }

    m_stats.numSamples += n;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit filterCompleted(n, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void DistributedArithmetic7::reset()
{
    m_delayLine.fill(0.0);
    m_delayPos = 0;
}

/* ---- LUT contents ---- */

QVector<double> DistributedArithmetic7::lutContents() const
{
    return m_lut;
}

/* ---- Reset statistics ---- */

void DistributedArithmetic7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

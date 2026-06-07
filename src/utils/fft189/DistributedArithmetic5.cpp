/**
 * @file DistributedArithmetic5.cpp
 * @brief DistributedArithmetic5 实现
 *
 * 实现分布式算术FIR滤波器：LUT构建、位串行求和、定点Q格式、浮点验证。
 */

#include "utils/fft189/DistributedArithmetic5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DistributedArithmetic5::DistributedArithmetic5(QObject *parent) : QObject(parent) {}
DistributedArithmetic5::~DistributedArithmetic5() = default;

/* ---- Configuration ---- */

void DistributedArithmetic5::setFractionalBits(int bits) { m_fracBits = qMax(1, bits); }

/* ---- Fixed-point helpers ---- */

qint32 DistributedArithmetic5::toFixedPoint(double val) const
{
    double scaled = val * (1 << m_fracBits);
    return static_cast<qint32>(qRound(scaled));
}

double DistributedArithmetic5::fromFixedPoint(qint32 val) const
{
    return static_cast<double>(val) / (1 << m_fracBits);
}

/* ---- Build LUT from coefficients ---- */

void DistributedArithmetic5::buildLut()
{
    // LUT: 2^N entries, where N = number of coefficients
    // Each entry is precomputed sum of selected coefficients
    int N = m_coeffs.size();
    int lutSize = 1 << N;
    m_lut.resize(lutSize);

    for (int addr = 0; addr < lutSize; ++addr) {
        qint32 sum = 0;
        for (int b = 0; b < N; ++b) {
            if (addr & (1 << b))
                sum += m_coeffs[b];
        }
        m_lut[addr] = sum;
    }
}

/* ---- Set coefficients ---- */

void DistributedArithmetic5::setCoefficients(const QVector<double>& coeffs)
{
    m_fCoeffs = coeffs;
    m_order = coeffs.size();

    // Convert to fixed-point
    m_coeffs.resize(m_order);
    for (int i = 0; i < m_order; ++i)
        m_coeffs[i] = toFixedPoint(coeffs[i]);

    // Initialize shift register
    m_shiftReg.resize(m_order, 0);
    m_regIndex = 0;

    // Build LUT
    buildLut();

    m_stats.filterOrder = m_order;
    m_stats.lutEntries = m_lut.size();
    m_stats.fracBits = m_fracBits;
}

/* ---- Process single sample ---- */

qint32 DistributedArithmetic5::processSample(qint32 sample)
{
    if (m_order == 0) return 0;

    // Insert into circular shift register
    m_shiftReg[m_regIndex] = sample;
    m_regIndex = (m_regIndex + 1) % m_order;

    // Build LUT address from MSBs of each register entry
    // Extract sign bit of each sample for bit-serial DA
    qint32 accumulator = 0;
    int numBits = m_fracBits + 1; // Include sign bit

    for (int bit = 0; bit < numBits; ++bit) {
        int addr = 0;
        for (int n = 0; n < m_order; ++n) {
            int idx = (m_regIndex - 1 - n + m_order) % m_order;
            if (m_shiftReg[idx] & (1 << bit))
                addr |= (1 << n);
        }
        qint32 lutVal = m_lut[addr];
        // Scale by bit position
        accumulator += lutVal << bit;
    }

    // Account for fractional scaling
    accumulator >>= m_fracBits;

    m_stats.totalFilterOps++;
    return accumulator;
}

/* ---- Batch processing ---- */

QVector<qint32> DistributedArithmetic5::processBlock(const QVector<qint32>& samples)
{
    QElapsedTimer timer;
    timer.start();

    QVector<qint32> output(samples.size());
    for (int i = 0; i < samples.size(); ++i)
        output[i] = processSample(samples[i]);

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalFilterOps > 0)
        ? m_timeSum / m_stats.totalFilterOps : 0.0;

    emit filterCompleted(samples.size(), timer.elapsed());
    return output;
}

/* ---- Float filtering for verification ---- */

QVector<double> DistributedArithmetic5::filterFloat(
    const QVector<double>& input) const
{
    if (m_fCoeffs.isEmpty()) return input;

    int n = input.size();
    int order = m_fCoeffs.size();
    QVector<double> output(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < order; ++j) {
            int idx = i - j;
            if (idx >= 0)
                sum += m_fCoeffs[j] * input[idx];
        }
        output[i] = sum;
    }
    return output;
}

/* ---- Reset ---- */

void DistributedArithmetic5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_shiftReg.clear();
    m_regIndex = 0;
}

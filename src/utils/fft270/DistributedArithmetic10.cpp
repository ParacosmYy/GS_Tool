/**
 * @file DistributedArithmetic10.cpp
 * @brief DistributedArithmetic10 实现
 *
 * 实现分布式算术：查找表分解与位串行累加无乘法器FIR滤波。
 */

#include "utils/fft270/DistributedArithmetic10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

DistributedArithmetic10::DistributedArithmetic10(QObject *parent)
    : QObject(parent) {}

DistributedArithmetic10::~DistributedArithmetic10() = default;

/* ---- Configuration ---- */

void DistributedArithmetic10::setCoefficients(const QVector<double>& coeffs)
{
    m_floatCoeffs = coeffs;
    m_order = coeffs.size();

    // Quantize to fixed-point
    m_quantCoeffs.resize(m_order);
    for (int i = 0; i < m_order; ++i)
        m_quantCoeffs[i] = quantize(coeffs[i]);

    buildLUT();
}

void DistributedArithmetic10::setInputWordLength(int bits)
{
    m_wordLength = qBound(4, bits, 32);
    if (!m_quantCoeffs.isEmpty()) buildLUT();
}

void DistributedArithmetic10::setQuantMode(QuantMode mode)
{
    m_quantMode = mode;
    if (!m_floatCoeffs.isEmpty()) {
        for (int i = 0; i < m_order; ++i)
            m_quantCoeffs[i] = quantize(m_floatCoeffs[i]);
        buildLUT();
    }
}

/* ---- Quantize coefficient ---- */

qint32 DistributedArithmetic10::quantize(double value) const
{
    double scale = static_cast<double>(1 << (m_wordLength - 1));
    double scaled = value * scale;
    switch (m_quantMode) {
    case Truncate:
        return static_cast<qint32>(scaled);
    case Round:
        return static_cast<qint32>(scaled + (scaled >= 0 ? 0.5 : -0.5));
    case Convergent: {
        qint32 truncated = static_cast<qint32>(scaled);
        double frac = scaled - truncated;
        if (qAbs(frac - 0.5) < 1e-12)
            return (truncated % 2 == 0) ? truncated : truncated + 1;
        return static_cast<qint32>(scaled + (scaled >= 0 ? 0.5 : -0.5));
    }
    }
    return static_cast<qint32>(scaled);
}

/* ---- Build lookup table ---- */

void DistributedArithmetic10::buildLUT()
{
    // LUT maps N-bit address (one bit from each tap) to partial sum
    // Address width = number of taps (up to word length)
    m_lutAddrBits = qMin(m_order, m_wordLength);
    int lutSize = 1 << m_lutAddrBits;
    m_lut.resize(lutSize);
    m_lut.fill(0.0);

    double scale = 1.0 / static_cast<double>(1 << (m_wordLength - 1));

    for (int addr = 0; addr < lutSize; ++addr) {
        double sum = 0.0;
        for (int tap = 0; tap < m_lutAddrBits; ++tap) {
            if (addr & (1 << tap))
                sum += m_quantCoeffs[tap] * scale;
        }
        m_lut[addr] = sum;
    }
}

/* ---- DA FIR filter ---- */

QVector<double> DistributedArithmetic10::filter(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0 || m_order == 0) return input;

    QVector<double> output(n, 0.0);
    double scale = 1.0 / static_cast<double>(1 << (m_wordLength - 1));

    // Input shift register
    QVector<double> shiftReg(m_order, 0.0);

    for (int i = 0; i < n; ++i) {
        // Shift in new sample
        for (int t = m_order - 1; t > 0; --t)
            shiftReg[t] = shiftReg[t - 1];
        shiftReg[0] = input[i];

        // Bit-serial DA: accumulate across all bit positions
        double acc = 0.0;
        for (int bit = 0; bit < m_wordLength; ++bit) {
            // Form LUT address from bit 'bit' of each tap
            int addr = 0;
            for (int tap = 0; tap < m_lutAddrBits; ++tap) {
                // Extract sign-magnitude bit from shifted input
                double val = shiftReg[tap] / scale;
                qint32 fixed = static_cast<qint32>(val);
                if (fixed & (1 << bit))
                    addr |= (1 << tap);
            }
            acc += m_lut[addr] * (1 << bit) * scale;
        }
        output[i] = acc;
    }

    // Normalize output amplitude
    double maxOut = 0.0;
    for (int i = 0; i < n; ++i)
        if (qAbs(output[i]) > maxOut) maxOut = qAbs(output[i]);
    if (maxOut > 1e-10) {
        for (int i = 0; i < n; ++i)
            output[i] /= maxOut;
    }

    double elapsed = timer.elapsed();
    m_stats.filterOrder = m_order;
    m_stats.lutSize = m_lut.size();
    m_stats.inputWordLength = m_wordLength;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit filterCompleted(n, m_order, elapsed);

    return output;
}

/* ---- Accessors ---- */

QVector<double> DistributedArithmetic10::lookupTable() const { return m_lut; }
QVector<qint32> DistributedArithmetic10::quantizedCoeffs() const { return m_quantCoeffs; }

/* ---- Reset ---- */

void DistributedArithmetic10::resetStatistics()
{
    m_floatCoeffs.clear();
    m_quantCoeffs.clear();
    m_lut.clear();
    m_order = 0;
    m_lutAddrBits = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

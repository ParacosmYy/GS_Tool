/**
 * @file DistributedArithmetic11.cpp
 * @brief DistributedArithmetic11 实现
 *
 * 实现分布式算术：ROM分区无乘累加FIR与位串行累加器的硬件友好滤波。
 */

#include "utils/fft284/DistributedArithmetic11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DistributedArithmetic11::DistributedArithmetic11(QObject *parent)
    : QObject(parent) {}

DistributedArithmetic11::~DistributedArithmetic11() = default;

/* ---- Configuration ---- */

void DistributedArithmetic11::setCoefficients(const QVector<double>& coeffs)
{
    m_config.coefficients = coeffs;
    m_numTaps = coeffs.size();
    m_shiftReg.resize(m_numTaps, 0);
}

void DistributedArithmetic11::setInputBits(int bits)
{
    m_config.inputBits = qBound(4, bits, 32);
}

void DistributedArithmetic11::setLUTSegments(int segments)
{
    m_config.lutSegments = qBound(1, segments, 16);
}

/* ---- Quantize to fixed-point ---- */

qint32 DistributedArithmetic11::quantize(double value, int bits) const
{
    double maxVal = (1 << (bits - 1)) - 1;
    double scaled = value * maxVal;
    return static_cast<qint32>(qBound(-maxVal - 1, qRound(scaled), maxVal));
}

/* ---- Build LUT for one segment of coefficients ---- */

QVector<qint32> DistributedArithmetic11::buildSegmentLUT(
    const QVector<double>& segCoeffs, int addrBits) const
{
    int entries = 1 << addrBits;
    QVector<qint32> lut(entries, 0);

    // LUT address is formed by one bit from each tap in this segment
    // For each possible address, sum the corresponding coefficient subset
    int segSize = segCoeffs.size();

    for (int addr = 0; addr < entries; ++addr) {
        qint32 sum = 0;
        for (int b = 0; b < addrBits && b < segSize; ++b) {
            if (addr & (1 << b)) {
                sum += quantize(segCoeffs[b], m_config.coeffBits);
            }
        }
        lut[addr] = sum;
    }
    return lut;
}

/* ---- Build all LUT tables ---- */

void DistributedArithmetic11::buildLUT()
{
    if (m_numTaps == 0) return;

    int segments = qMin(m_config.lutSegments, m_numTaps);
    m_tapsPerSegment = (m_numTaps + segments - 1) / segments;

    m_lutTables.clear();
    m_lutTables.reserve(segments);

    for (int s = 0; s < segments; ++s) {
        int start = s * m_tapsPerSegment;
        int end = qMin(start + m_tapsPerSegment, m_numTaps);
        QVector<double> segCoeffs;
        for (int i = start; i < end; ++i)
            segCoeffs.append(m_config.coefficients[i]);

        int addrBits = segCoeffs.size();
        // Limit address bits for practical LUT size
        addrBits = qMin(addrBits, 12);
        m_lutTables.append(buildSegmentLUT(segCoeffs, addrBits));
    }
}

/* ---- Bit-serial accumulate ---- */

qint32 DistributedArithmetic11::bitSerialAccumulate(
    const QVector<qint32>& regSlice) const
{
    // Address = bits from each tap register in this segment
    int addrBits = regSlice.size();
    qint32 result = 0;

    // Process each input bit position
    int inputBits = m_config.inputBits;
    for (int bit = 0; bit < inputBits; ++bit) {
        int addr = 0;
        for (int t = 0; t < addrBits; ++t) {
            if (regSlice[t] & (1 << bit))
                addr |= (1 << t);
        }

        // Look up precomputed partial sum
        int segIdx = 0;  // Only used when called from the main filter loop
        Q_UNUSED(segIdx)
        result += addr;  // Simplified: in real HW, this is a LUT lookup
    }
    return result;
}

/* ---- Saturate ---- */

double DistributedArithmetic11::saturate(qint64 value) const
{
    const qint64 maxOut = (1LL << 30) - 1;
    const qint64 minOut = -(1LL << 30);
    value = qBound(minOut, value, maxOut);
    return static_cast<double>(value) / (1LL << (m_config.coeffBits - 1));
}

/* ---- Main filter ---- */

DistributedArithmetic11::DAResult DistributedArithmetic11::filter(
    const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    DAResult result;
    int n = input.size();
    if (n == 0 || m_numTaps == 0) return result;

    result.output.resize(n);
    double peak = 0.0;

    // Scale factor for output normalization
    double scaleFactor = 1.0 / ((1 << (m_config.inputBits - 1)));

    for (int i = 0; i < n; ++i) {
        // Shift register: push new quantized sample
        qint32 sample = quantize(input[i], m_config.inputBits);
        for (int t = m_numTaps - 1; t > 0; --t)
            m_shiftReg[t] = m_shiftReg[t - 1];
        m_shiftReg[0] = sample;

        // Distributed arithmetic: bit-serial accumulation across LUT segments
        qint64 accum = 0;
        int segments = m_lutTables.size();

        for (int s = 0; s < segments; ++s) {
            int start = s * m_tapsPerSegment;
            int end = qMin(start + m_tapsPerSegment, m_numTaps);

            // Process bit by bit for this segment
            for (int bit = 0; bit < m_config.inputBits; ++bit) {
                int addr = 0;
                int addrBit = 0;
                for (int t = start; t < end; ++t) {
                    if (m_shiftReg[t] & (1 << bit))
                        addr |= (1 << addrBit);
                    ++addrBit;
                }
                // LUT lookup
                if (addr < m_lutTables[s].size())
                    accum += static_cast<qint64>(m_lutTables[s][addr]) << bit;
            }
        }

        // Scale output back to floating point
        double output = saturate(accum) * scaleFactor;
        result.output[i] = output;

        double absOut = qAbs(output);
        if (absOut > peak) peak = absOut;
    }

    result.peakAmplitude = peak;
    result.processingGain = (peak > 0.0) ? 20.0 * qLog10(peak + 1e-30) : 0.0;

    double elapsed = timer.elapsed();
    m_stats.filterOrder = m_numTaps;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit filterDone(n, m_numTaps, peak, elapsed);

    return result;
}

/* ---- Reset ---- */

void DistributedArithmetic11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_shiftReg.fill(0);
}

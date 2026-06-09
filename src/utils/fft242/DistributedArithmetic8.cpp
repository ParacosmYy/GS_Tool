/**
 * @file DistributedArithmetic8.cpp
 * @brief DistributedArithmetic8 实现
 *
 * 实现分布式算术：查找表分区与流水线累加定点FIR求值。
 */

#include "utils/fft242/DistributedArithmetic8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DistributedArithmetic8::DistributedArithmetic8(QObject *parent) : QObject(parent) {}
DistributedArithmetic8::~DistributedArithmetic8() = default;

/* ---- Configuration ---- */

void DistributedArithmetic8::setFractionalBits(int bits) { m_fracBits = qMax(1, bits); }

/* ---- Fixed-point helpers ---- */

qint32 DistributedArithmetic8::toFixed(double val) const
{
    return static_cast<qint32>(val * (1 << m_fracBits));
}

double DistributedArithmetic8::fromFixed(qint32 val) const
{
    return static_cast<double>(val) / (1 << m_fracBits);
}

/* ---- Build LUT ---- */

void DistributedArithmetic8::buildLUT()
{
    if (m_taps == 0) return;

    m_partitions = (m_taps + kPartitionSize - 1) / kPartitionSize;
    m_lut.resize(m_partitions);

    for (int p = 0; p < m_partitions; ++p) {
        int start = p * kPartitionSize;
        int pSize = qMin(kPartitionSize, m_taps - start);
        int entries = 1 << pSize;
        m_lut[p].resize(entries);

        for (int addr = 0; addr < entries; ++addr) {
            qint32 sum = 0;
            for (int b = 0; b < pSize; ++b) {
                if (addr & (1 << b))
                    sum += toFixed(m_coeffs[start + b]);
            }
            m_lut[p][addr] = sum;
        }
    }

    m_stats.filterLength = m_taps;
    m_stats.lutEntries = (m_partitions > 0) ? (1 << kPartitionSize) : 0;
    m_stats.numPartitions = m_partitions;

    emit filterConfigured(m_taps, m_stats.lutEntries);
}

/* ---- Set coefficients ---- */

bool DistributedArithmetic8::setCoefficients(const QVector<double>& coeffs)
{
    if (coeffs.isEmpty()) return false;
    m_coeffs = coeffs;
    m_taps = coeffs.size();
    m_shiftReg.resize(m_taps, 0);
    buildLUT();
    return true;
}

/* ---- Process single sample ---- */

double DistributedArithmetic8::processOne(double sample)
{
    if (m_taps == 0) return 0.0;

    // Shift register: push new sample
    m_shiftReg.push_front(toFixed(sample));
    if (m_shiftReg.size() > m_taps)
        m_shiftReg.resize(m_taps);

    // DA evaluation: iterate over fractional bits
    qint32 result = 0;
    int bits = m_fracBits + 1; // extra bit for sign handling

    for (int bit = 0; bit < bits; ++bit) {
        qint32 accum = 0;
        for (int p = 0; p < m_partitions; ++p) {
            int start = p * kPartitionSize;
            int pSize = qMin(kPartitionSize, m_taps - start);
            int addr = 0;
            for (int b = 0; b < pSize; ++b) {
                int si = start + b;
                if (si < m_shiftReg.size()) {
                    qint32 val = m_shiftReg[si];
                    if (val & (1 << bit))
                        addr |= (1 << b);
                }
            }
            if (addr < m_lut[p].size())
                accum += m_lut[p][addr];
        }
        // Shift-accumulate (bit-serial DA)
        result += accum << bit;
    }

    // Scale back: divide by 2^fracBits to normalize
    result >>= m_fracBits;
    return fromFixed(result);
}

/* ---- Process buffer ---- */

QVector<double> DistributedArithmetic8::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    output.resize(input.size());
    for (int i = 0; i < input.size(); ++i)
        output[i] = processOne(input[i]);

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processingCompleted(input.size(), timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void DistributedArithmetic8::reset()
{
    m_shiftReg.fill(0);
}

void DistributedArithmetic8::resetStatistics()
{
    reset();
    m_stats = Stats{}; m_timeSum = 0.0;
}

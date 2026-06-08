/**
 * @file DistributedArithmetic6.cpp
 * @brief DistributedArithmetic6 实现
 *
 * 实现分布式算术：LUT分区构建、位串行查找、流水线累加。
 */

#include "utils/fft214/DistributedArithmetic6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

DistributedArithmetic6::DistributedArithmetic6(QObject *parent)
    : QObject(parent) {}
DistributedArithmetic6::~DistributedArithmetic6() = default;

/* ---- Configuration ---- */

void DistributedArithmetic6::setCoefficients(const QVector<double>& coeffs,
                                              int bitWidth)
{
    m_coeffs = coeffs;
    m_order = coeffs.size();
    m_bitWidth = qMax(4, qMin(32, bitWidth));

    // Determine number of partitions: each LUT covers a subset of coefficients
    // LUT address space = 2^(coeffsPerPartition), keep under 2^12 = 4096
    m_coeffsPerPart = qMax(1, qMin(12, m_bitWidth));
    m_partitions = qMax(1, (m_order + m_coeffsPerPart - 1) / m_coeffsPerPart);
    m_lutSize = 1 << m_coeffsPerPart;

    // Shift register for input history
    m_shiftReg.resize(m_order, 0.0);
    m_regPos = 0;

    buildLUTs();

    m_stats.filterOrder = m_order;
    m_stats.lutEntries = m_lutSize;
    m_stats.partitions = m_partitions;
    m_stats.bitWidth = m_bitWidth;
}

/* ---- Build partitioned LUTs ---- */

void DistributedArithmetic6::buildLUTs()
{
    m_luts.resize(m_partitions);
    for (int p = 0; p < m_partitions; ++p) {
        m_luts[p].resize(m_lutSize, 0.0);
        int startCoeff = p * m_coeffsPerPart;
        int endCoeff = qMin(startCoeff + m_coeffsPerPart, m_order);

        for (int addr = 0; addr < m_lutSize; ++addr) {
            double sum = 0.0;
            for (int b = 0; b < m_coeffsPerPart; ++b) {
                int cIdx = startCoeff + b;
                if (cIdx >= m_order) break;
                // Bit b of address: 1 means coefficient contributes
                if (addr & (1 << b))
                    sum += m_coeffs[cIdx];
            }
            m_luts[p][addr] = sum;
        }
    }
}

/* ---- Fixed-point conversion ---- */

qint32 DistributedArithmetic6::toFixed(double val) const
{
    double scaled = val * (1 << (m_bitWidth - 1));
    return static_cast<qint32>(qBound(
        static_cast<double>(std::numeric_limits<qint32>::min()),
        scaled,
        static_cast<double>(std::numeric_limits<qint32>::max())));
}

double DistributedArithmetic6::fromFixed(qint64 val) const
{
    return static_cast<double>(val) / (1LL << (m_bitWidth - 1));
}

/* ---- Query LUT ---- */

double DistributedArithmetic6::queryLUT(int partition, int address) const
{
    if (partition < 0 || partition >= m_luts.size()) return 0.0;
    if (address < 0 || address >= m_lutSize) return 0.0;
    return m_luts[partition][address];
}

/* ---- Process single sample ---- */

double DistributedArithmetic6::processOne(double input)
{
    if (m_order == 0) return 0.0;

    // Update shift register
    m_shiftReg[m_regPos] = input;

    // Distributed arithmetic: bit-serial accumulation
    // For each bit position, look up all partitions and accumulate with shift
    double output = 0.0;

    for (int bit = 0; bit < m_bitWidth; ++bit) {
        double bitSum = 0.0;
        double signBit = 1.0;
        if (bit == m_bitWidth - 1) signBit = -1.0;  // Two's complement sign

        for (int p = 0; p < m_partitions; ++p) {
            int addr = 0;
            int startIdx = p * m_coeffsPerPart;
            for (int b = 0; b < m_coeffsPerPart; ++b) {
                int cIdx = startIdx + b;
                if (cIdx >= m_order) break;
                // Get the bit-th bit of the (m_order - 1 - cIdx)-th history sample
                int histIdx = (m_regPos - cIdx + m_order) % m_order;
                qint32 fixed = toFixed(m_shiftReg[histIdx]);
                if (fixed & (1 << bit))
                    addr |= (1 << b);
            }
            bitSum += m_luts[p][addr] * signBit;
        }
        output += bitSum * qPow(2.0, -(bit + 1)) * (1 << (m_bitWidth - 1));
    }

    m_regPos = (m_regPos + 1) % m_order;
    return output;
}

/* ---- Process buffer ---- */

QVector<double> DistributedArithmetic6::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n, 0.0);
    for (int i = 0; i < n; ++i)
        output[i] = processOne(input[i]);

    m_stats.totalSamples += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSamples;
    emit processingCompleted(n, timer.elapsed());

    return output;
}

/* ---- Get coefficients ---- */

QVector<double> DistributedArithmetic6::coefficients() const
{
    return m_coeffs;
}

/* ---- Reset ---- */

void DistributedArithmetic6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_shiftReg.fill(0.0);
    m_regPos = 0;
}

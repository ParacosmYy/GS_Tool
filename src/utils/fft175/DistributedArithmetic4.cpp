/**
 * @file DistributedArithmetic4.cpp
 * @brief DistributedArithmetic4 实现
 *
 * 实现分布式算术FIR滤波：LUT预计算、串行/并行位处理。
 */

#include "utils/fft175/DistributedArithmetic4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DistributedArithmetic4::DistributedArithmetic4(QObject *parent)
    : QObject(parent)
{
}

DistributedArithmetic4::~DistributedArithmetic4() = default;

/* ---- Configuration ---- */

void DistributedArithmetic4::setCoefficients(const QVector<double>& coeffs)
{
    m_coeffs = coeffs;
    m_order = coeffs.size();
    m_shiftReg.resize(m_order, 0);
    m_writePos = 0;
    buildLUT();
    m_stats.filterOrder = m_order;
    m_stats.lutSize = m_lut.size();
    emit filterInitialized(m_order, m_lut.size());
}

void DistributedArithmetic4::setMode(Mode mode) { m_mode = mode; }
void DistributedArithmetic4::setInputBitWidth(int bits) { m_bitWidth = qMax(1, bits); }

/* ---- Build LUT ---- */

void DistributedArithmetic4::buildLUT()
{
    /* For N taps, LUT has 2^N entries (for serial bit mode)
       For practical use with large N, use grouped LUT (2^B per group) */
    int N = m_order;
    if (N == 0) { m_lut.clear(); return; }

    /* Use grouped LUT: split taps into groups of B bits */
    const int B = qMin(N, 8); /* Max 256 entries per group */
    int numGroups = (N + B - 1) / B;

    /* Build single unified LUT: 2^B entries per group */
    int groupSize = 1 << B;
    m_lut.resize(numGroups * groupSize, 0.0);

    for (int g = 0; g < numGroups; ++g) {
        int startTap = g * B;
        int endTap = qMin(startTap + B, N);
        int tapsInGroup = endTap - startTap;

        for (int addr = 0; addr < groupSize; ++addr) {
            double sum = 0.0;
            for (int b = 0; b < tapsInGroup; ++b) {
                if (addr & (1 << b))
                    sum += m_coeffs[startTap + b];
            }
            m_lut[g * groupSize + addr] = sum;
        }
    }

    /* Scale factor for quantization */
    m_scaleFactor = static_cast<double>((1 << (m_bitWidth - 1)) - 1);
}

/* ---- Quantize ---- */

qint32 DistributedArithmetic4::quantize(double sample) const
{
    double scaled = sample * m_scaleFactor;
    return static_cast<qint32>(qBound(-(1 << (m_bitWidth - 1)),
                                       static_cast<int>(qRound(scaled)),
                                       (1 << (m_bitWidth - 1)) - 1));
}

/* ---- Serial bit DA ---- */

double DistributedArithmetic4::serialDA(qint32* shiftReg) const
{
    int N = m_order;
    const int B = qMin(N, 8);
    int numGroups = (N + B - 1) / B;
    int groupSize = 1 << B;
    double acc = 0.0;

    for (int bit = 0; bit < m_bitWidth; ++bit) {
        double bitSum = 0.0;

        for (int g = 0; g < numGroups; ++g) {
            int startTap = g * B;
            int endTap = qMin(startTap + B, N);
            int addr = 0;
            for (int b = 0; b < endTap - startTap; ++b) {
                if (shiftReg[startTap + b] & (1 << bit))
                    addr |= (1 << b);
            }
            bitSum += m_lut[g * groupSize + addr];
        }

        /* Accumulate with binary point shift */
        if (bit == m_bitWidth - 1)
            acc -= bitSum; /* Sign bit */
        else
            acc += bitSum;
    }

    return acc / m_scaleFactor;
}

/* ---- Parallel bit DA ---- */

double DistributedArithmetic4::parallelDA(qint32* shiftReg) const
{
    /* For parallel mode, compute one LUT lookup per group */
    int N = m_order;
    const int B = qMin(N, 8);
    int numGroups = (N + B - 1) / B;
    int groupSize = 1 << B;
    double sum = 0.0;

    for (int g = 0; g < numGroups; ++g) {
        int startTap = g * B;
        int endTap = qMin(startTap + B, N);
        int addr = 0;
        for (int b = 0; b < endTap - startTap; ++b) {
            /* Extract bits from quantized shift register */
            qint32 val = shiftReg[startTap + b];
            if (val < 0) val = ~val + 1; /* Use magnitude for address */
            if (val & 1) addr |= (1 << b);
        }
        sum += m_lut[g * groupSize + addr];
    }

    return sum / m_scaleFactor;
}

/* ---- Process one sample ---- */

double DistributedArithmetic4::processOne(double sample)
{
    if (m_order == 0 || m_lut.isEmpty()) return 0.0;

    /* Shift new sample into register */
    m_shiftReg[m_writePos] = quantize(sample);

    /* Build ordered shift register for DA computation */
    QVector<qint32> ordered(m_order, 0);
    for (int i = 0; i < m_order; ++i) {
        int idx = (m_writePos - i + m_order) % m_order;
        ordered[i] = m_shiftReg[idx];
    }

    m_writePos = (m_writePos + 1) % m_order;

    if (m_mode == SerialBit)
        return serialDA(ordered.data());
    return parallelDA(ordered.data());
}

/* ---- Process buffer ---- */

QVector<double> DistributedArithmetic4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n, 0.0);

    for (int i = 0; i < n; ++i)
        output[i] = processOne(input[i]);

    m_stats.totalProcesses++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcesses;

    emit processingCompleted(n);
    return output;
}

/* ---- Accessors ---- */

QVector<double> DistributedArithmetic4::lutTable() const { return m_lut; }
QVector<double> DistributedArithmetic4::coefficients() const { return m_coeffs; }

void DistributedArithmetic4::reset()
{
    m_shiftReg.fill(0);
    m_writePos = 0;
}

void DistributedArithmetic4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

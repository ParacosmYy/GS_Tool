/**
 * @file DistributedArithmetic9.cpp
 * @brief DistributedArithmetic9 实现
 *
 * 实现分布式算术：可重构LUT与串并混合累加多速率FIR滤波。
 */

#include "utils/fft256/DistributedArithmetic9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DistributedArithmetic9::DistributedArithmetic9(QObject *parent)
    : QObject(parent) {}
DistributedArithmetic9::~DistributedArithmetic9() = default;

/* ---- Configuration ---- */

void DistributedArithmetic9::setMode(Mode mode) { m_mode = mode; }
void DistributedArithmetic9::setOversamplingRatio(int osr) { m_osr = qMax(1, osr); }

/* ---- Set coefficients and build LUT ---- */

bool DistributedArithmetic9::setCoefficients(const QVector<double>& coeffs,
                                              int inputBits)
{
    if (coeffs.isEmpty()) return false;
    m_coeffs = coeffs;
    m_taps = coeffs.size();
    m_inputBits = qBound(4, inputBits, 16);
    m_shiftReg.resize(m_taps, 0.0);
    m_decimCounter = 0;

    buildLUT();

    m_stats.filterOrder = m_taps;
    m_stats.lutSize = m_lut.size();
    m_stats.numFilters++;
    return true;
}

/* ---- Build DA LUT ---- */

void DistributedArithmetic9::buildLUT()
{
    // For N taps with B input bits, LUT has 2^B entries
    // Each entry = sum of coefficients where the bit is 1
    int lutEntries = 1 << m_inputBits;
    m_lut.resize(lutEntries, 0.0);

    for (int addr = 0; addr < lutEntries; ++addr) {
        double sum = 0.0;
        // Each bit of addr corresponds to selecting coefficient
        for (int tap = 0; tap < qMin(m_taps, m_inputBits); ++tap) {
            if (addr & (1 << tap)) {
                sum += m_coeffs[tap];
            }
        }
        m_lut[addr] = sum;
    }
}

/* ---- Quantize double to integer ---- */

int DistributedArithmetic9::quantize(double sample) const
{
    int maxVal = (1 << (m_inputBits - 1)) - 1;
    int val = static_cast<int>(qBound(-maxVal - 1.0,
                                       sample * maxVal, static_cast<double>(maxVal)));
    return val & ((1 << m_inputBits) - 1);
}

/* ---- Extract bit ---- */

int DistributedArithmetic9::bitAt(int value, int b)
{
    return (value >> b) & 1;
}

/* ---- Process single sample: serial bit-serial ---- */

double DistributedArithmetic9::processSerial(double sample)
{
    // Shift new sample into register
    for (int i = m_taps - 1; i > 0; --i)
        m_shiftReg[i] = m_shiftReg[i - 1];
    m_shiftReg[0] = sample;

    // Bit-serial accumulation
    double acc = 0.0;
    for (int b = 0; b < m_inputBits; ++b) {
        double partialSum = 0.0;
        for (int t = 0; t < m_taps; ++t) {
            int q = quantize(m_shiftReg[t]);
            if (bitAt(q, b))
                partialSum += m_coeffs[t];
        }
        acc += partialSum * (1 << b);
    }
    return acc / ((1 << m_inputBits) - 1);
}

/* ---- Process single sample: parallel LUT lookup ---- */

double DistributedArithmetic9::processParallel(double sample)
{
    // Shift register update
    for (int i = m_taps - 1; i > 0; --i)
        m_shiftReg[i] = m_shiftReg[i - 1];
    m_shiftReg[0] = sample;

    // Parallel: direct convolution (LUT-accelerated)
    double acc = 0.0;
    for (int t = 0; t < m_taps; ++t) {
        int q = quantize(m_shiftReg[t]);
        if (q >= 0 && q < m_lut.size())
            acc += m_lut[q] * m_coeffs[t];
        else
            acc += m_shiftReg[t] * m_coeffs[t];
    }
    return acc / m_taps;
}

/* ---- Process block ---- */

QVector<double> DistributedArithmetic9::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output;
    output.reserve(n / m_osr + 1);

    for (int i = 0; i < n; ++i) {
        double y;
        if (m_mode == Serial) {
            y = processSerial(input[i]);
        } else if (m_mode == Parallel) {
            y = processParallel(input[i]);
        } else {
            // Hybrid: serial for upper bits, parallel for lower bits
            for (int t = m_taps - 1; t > 0; --t)
                m_shiftReg[t] = m_shiftReg[t - 1];
            m_shiftReg[0] = input[i];

            double acc = 0.0;
            int halfBits = m_inputBits / 2;
            // Lower bits: parallel LUT
            for (int t = 0; t < m_taps; ++t) {
                int q = quantize(m_shiftReg[t]);
                int lowAddr = q & ((1 << halfBits) - 1);
                if (lowAddr < m_lut.size())
                    acc += m_lut[lowAddr];
            }
            // Upper bits: serial accumulation
            for (int b = halfBits; b < m_inputBits; ++b) {
                double partial = 0.0;
                for (int t = 0; t < m_taps; ++t) {
                    int q = quantize(m_shiftReg[t]);
                    if (bitAt(q, b)) partial += m_coeffs[t];
                }
                acc += partial * (1 << (b - halfBits));
            }
            y = acc / ((1 << m_inputBits) - 1);
        }

        // Multi-rate: decimate output
        m_decimCounter++;
        if (m_decimCounter >= m_osr) {
            output.append(y);
            m_decimCounter = 0;
        }
    }

    m_stats.totalOps++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit filteringCompleted(n, output.size(), elapsed);
    return output;
}

/* ---- Get coefficients ---- */

QVector<double> DistributedArithmetic9::coefficients() const
{
    return m_coeffs;
}

/* ---- Reset ---- */

void DistributedArithmetic9::resetStatistics()
{
    m_coeffs.clear();
    m_lut.clear();
    m_shiftReg.clear();
    m_taps = 0;
    m_decimCounter = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

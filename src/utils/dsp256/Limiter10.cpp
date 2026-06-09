/**
 * @file Limiter10.cpp
 * @brief Limiter10 实现
 *
 * 实现砖墙限幅器：过采样插值真峰值检测与ISP/TPB合规计量。
 */

#include "utils/dsp256/Limiter10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Limiter10::Limiter10(QObject *parent)
    : QObject(parent)
{
    m_ceilingLinear = qPow(10.0, m_ceilingDb / 20.0);
    updateCoefficients();
    buildSincTable();
}
Limiter10::~Limiter10() = default;

/* ---- Configuration ---- */

void Limiter10::setCeiling(double ceilingDb)
{
    m_ceilingDb = ceilingDb;
    m_ceilingLinear = qPow(10.0, m_ceilingDb / 20.0);
}

void Limiter10::setRelease(double releaseMs) { m_releaseMs = qMax(0.1, releaseMs); }
void Limiter10::setOversampling(int factor)
{
    // Only power-of-2 factors
    if (factor == 1 || factor == 2 || factor == 4 || factor == 8)
        m_oversampling = factor;
    buildSincTable();
}

void Limiter10::setSampleRate(double rate)
{
    m_sampleRate = qMax(8000.0, rate);
    updateCoefficients();
}

/* ---- Update attack/release coefficients ---- */

void Limiter10::updateCoefficients()
{
    double sr = qMax(m_sampleRate, 1.0);
    m_attackCoeff = 1.0 - qExp(-1.0 / (m_attackMs * 0.001 * sr));
    m_releaseCoeff = 1.0 - qExp(-1.0 / (m_releaseMs * 0.001 * sr));
    m_gainRelease = 1.0 - qExp(-1.0 / (m_releaseMs * 0.001 * sr));
}

/* ---- Build sinc interpolation table ---- */

void Limiter10::buildSincTable()
{
    // Sinc interpolation: 4-phase, 8-tap polyphase filter
    int tapsPerPhase = 8;
    int numPhases = m_oversampling;
    int totalTaps = tapsPerPhase * numPhases;

    // Design lowpass filter cutoff at Nyquist / oversampling
    double cutoff = 1.0 / m_oversampling;
    m_sincCoeffs.resize(totalTaps);

    double sum = 0.0;
    for (int i = 0; i < totalTaps; ++i) {
        int center = totalTaps / 2;
        double n = i - center;
        double sinc;
        if (qFuzzyIsNull(n))
            sinc = 1.0;
        else
            sinc = qSin(M_PI * cutoff * n) / (M_PI * n);

        // Apply Hann window
        double window = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (totalTaps - 1)));
        m_sincCoeffs[i] = sinc * window;
        sum += m_sincCoeffs[i];
    }

    // Normalize
    for (int i = 0; i < totalTaps; ++i)
        m_sincCoeffs[i] /= sum;
}

/* ---- Oversample using sinc interpolation ---- */

QVector<double> Limiter10::oversample(const QVector<double>& input) const
{
    int n = input.size();
    int os = m_oversampling;
    QVector<double> out(n * os, 0.0);

    int halfTaps = m_sincCoeffs.size() / (2 * os);

    for (int i = 0; i < n; ++i) {
        for (int phase = 0; phase < os; ++phase) {
            double sum = 0.0;
            for (int t = -halfTaps; t <= halfTaps; ++t) {
                int srcIdx = i + t;
                if (srcIdx >= 0 && srcIdx < n) {
                    int coeffIdx = (t + halfTaps) * os + phase;
                    if (coeffIdx >= 0 && coeffIdx < m_sincCoeffs.size())
                        sum += input[srcIdx] * m_sincCoeffs[coeffIdx];
                }
            }
            out[i * os + phase] = sum;
        }
    }
    return out;
}

/* ---- Detect true peak in oversampled signal ---- */

double Limiter10::detectTruePeak(const QVector<double>& oversampled) const
{
    double peak = 0.0;
    for (double s : oversampled)
        peak = qMax(peak, qAbs(s));
    return peak;
}

/* ---- Compute gain reduction ---- */

double Limiter10::computeGainReduction(double peak) const
{
    if (peak <= m_ceilingLinear) return 1.0;
    return m_ceilingLinear / peak;
}

/* ---- Apply gain with smooth release ---- */

double Limiter10::applyGain(double sample)
{
    return sample * m_gain;
}

/* ---- Measure true peak ---- */

double Limiter10::measureTruePeak(const QVector<double>& signal) const
{
    if (signal.isEmpty()) return 0.0;
    QVector<double> os = oversample(signal);
    return detectTruePeak(os);
}

/* ---- ISP/TPB compliance ---- */

bool Limiter10::isCompliant() const
{
    return m_ispViolations == 0;
}

/* ---- Process audio ---- */

QVector<double> Limiter10::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output;
    output.reserve(n);

    m_truePeak = 0.0;
    m_ispViolations = 0;
    m_gain = 1.0;

    // Process in blocks for true-peak detection
    int blockSize = qMax(64, m_oversampling * 16);

    for (int blockStart = 0; blockStart < n; blockStart += blockSize) {
        int blockEnd = qMin(blockStart + blockSize, n);

        // Extract block
        QVector<double> block;
        for (int i = blockStart; i < blockEnd; ++i)
            block.append(input[i]);

        // Oversample and detect true peak
        QVector<double> osBlock = oversample(block);
        double blockTP = detectTruePeak(osBlock);
        m_truePeak = qMax(m_truePeak, blockTP);

        // Compute target gain
        double targetGain = computeGainReduction(blockTP);

        // Apply gain with envelope
        for (int i = 0; i < block.size(); ++i) {
            // Smooth gain towards target
            if (targetGain < m_gain) {
                // Attack: instant reduction
                m_gain = targetGain;
            } else {
                // Release: gradual recovery
                m_gain += (1.0 - m_gain) * m_gainRelease;
            }

            double sample = input[blockStart + i] * m_gain;
            output.append(sample);

            // Check ISP violation
            if (qAbs(sample) > m_ceilingLinear * 1.04) // 4% tolerance
                m_ispViolations++;
        }
    }

    double elapsed = timer.elapsed();
    m_stats.numSamples = n;
    m_stats.truePeakDb = 20.0 * qLn(qMax(m_truePeak, 1e-10)) / qLn(10.0);
    m_stats.peakReductionDb = m_stats.truePeakDb - m_ceilingDb;
    m_stats.ispCount = m_ispViolations;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processingCompleted(n, m_stats.truePeakDb, elapsed);
    return output;
}

/* ---- Reset ---- */

void Limiter10::resetStatistics()
{
    m_gain = 1.0;
    m_truePeak = 0.0;
    m_ispViolations = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

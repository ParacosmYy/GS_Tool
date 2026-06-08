/**
 * @file Limiter7.cpp
 * @brief Limiter7 实现
 *
 * 实现砖墙限制器：前瞻缓冲、4x过采样真峰值检测、增益平滑衰减。
 */

#include "utils/dsp214/Limiter7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Limiter7::Limiter7(QObject *parent) : QObject(parent)
{
    buildOversamplingFilter();
}

Limiter7::~Limiter7() = default;

/* ---- Build 4x oversampling FIR filter ---- */

void Limiter7::buildOversamplingFilter()
{
    // Short sinc interpolator for 4x upsampling
    // 8-tap FIR, cutoff at pi/4
    int taps = 8;
    m_osCoeffs.resize(taps);
    double sum = 0.0;
    for (int i = 0; i < taps; ++i) {
        double n = i - (taps - 1) / 2.0;
        double x = n * M_PI / 4.0;
        double sinc = (qAbs(n) < 1e-10) ? 1.0 : qSin(x) / x;
        // Hann window
        double w = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (taps - 1)));
        m_osCoeffs[i] = sinc * w;
        sum += m_osCoeffs[i];
    }
    // Normalize
    for (auto& c : m_osCoeffs) c /= sum;

    m_osHistory.resize(taps, 0.0);
}

/* ---- Configuration ---- */

void Limiter7::setParameters(double ceilingDb, double lookaheadMs,
                              double releaseMs, double sampleRate)
{
    m_sampleRate = qMax(8000.0, sampleRate);
    m_ceilingLinear = qPow(10.0, ceilingDb / 20.0);
    m_thresholdLinear = m_ceilingLinear;
    m_lookahead = qMax(1, static_cast<int>(lookaheadMs * m_sampleRate / 1000.0));

    double releaseSamples = releaseMs * m_sampleRate / 1000.0;
    m_releaseCoeff = (releaseSamples > 0.0) ? qExp(-1.0 / releaseSamples) : 0.0;

    m_delayBuffer.resize(m_lookahead, 0.0);
    m_delayPos = 0;

    m_stats.ceiling = ceilingDb;
    m_stats.threshold = ceilingDb;
    m_stats.lookaheadSamples = m_lookahead;
}

/* ---- Compute gain for peak level ---- */

double Limiter7::computeGain(double peakLevel) const
{
    if (peakLevel <= m_ceilingLinear) return 1.0;
    return m_ceilingLinear / peakLevel;
}

/* ---- Measure true-peak via 4x oversampling ---- */

double Limiter7::measureTruePeak(double input) const
{
    // Insert input into history
    const_cast<Limiter7*>(this)->m_osHistory.prepend(input);
    if (const_cast<Limiter7*>(this)->m_osHistory.size() > m_osCoeffs.size())
        const_cast<Limiter7*>(this)->m_osHistory.removeLast();

    double maxPeak = qAbs(input);

    // 4x upsampling: compute 4 interpolated values
    for (int phase = 0; phase < 4; ++phase) {
        double val = 0.0;
        for (int i = 0; i < m_osCoeffs.size(); ++i) {
            int idx = i * 4 + phase;
            int histIdx = idx / 4;
            if (histIdx < m_osHistory.size())
                val += m_osHistory[histIdx] * m_osCoeffs[i];
        }
        maxPeak = qMax(maxPeak, qAbs(val));
    }
    return maxPeak;
}

/* ---- Process single sample ---- */

double Limiter7::processOne(double input)
{
    // Measure true-peak via 4x oversampling
    double truePeak = measureTruePeak(input);
    m_truePeak = qMax(m_truePeak, truePeak);

    // Compute target gain
    double targetGain = computeGain(truePeak);

    // Smooth gain: fast attack (instant), slow release
    if (targetGain < m_gain) {
        m_gain = targetGain;  // Instant attack
    } else {
        m_gain = m_releaseCoeff * m_gain + (1.0 - m_releaseCoeff) * targetGain;
    }

    m_peakReduction = qMax(m_peakReduction, 1.0 - m_gain);

    // Lookahead: store input in delay buffer, output delayed sample
    double delayed = m_delayBuffer[m_delayPos];
    m_delayBuffer[m_delayPos] = input * m_gain;
    m_delayPos = (m_delayPos + 1) % m_lookahead;

    return delayed;
}

/* ---- Process buffer ---- */

QVector<double> Limiter7::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n);
    m_truePeak = 0.0;
    m_peakReduction = 0.0;

    for (int i = 0; i < n; ++i)
        output[i] = processOne(input[i]);

    m_stats.totalSamples += n;
    m_stats.peakReduction = 20.0 * qLn(qMax(1e-10, 1.0 - m_peakReduction)) / M_LN10;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, m_stats.totalSamples);
    emit processingCompleted(n, m_stats.peakReduction, timer.elapsed());

    return output;
}

/* ---- Gain reduction in dB ---- */

double Limiter7::gainReductionDb() const
{
    if (m_gain <= 0.0) return -120.0;
    return 20.0 * qLn(m_gain) / M_LN10;
}

/* ---- True-peak level ---- */

double Limiter7::truePeakLevel() const
{
    if (m_truePeak <= 0.0) return -120.0;
    return 20.0 * qLn(m_truePeak) / M_LN10;
}

/* ---- Reset ---- */

void Limiter7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_gain = 1.0;
    m_peakReduction = 0.0;
    m_truePeak = 0.0;
    m_delayBuffer.fill(0.0);
    m_delayPos = 0;
    m_osHistory.fill(0.0);
}

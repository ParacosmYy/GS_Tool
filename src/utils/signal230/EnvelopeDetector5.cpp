/**
 * @file EnvelopeDetector5.cpp
 * @brief EnvelopeDetector5 实现
 *
 * 实现包络检测器：峰值保持衰减建模与RMS峰值比动态分类。
 */

#include "utils/signal230/EnvelopeDetector5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

EnvelopeDetector5::EnvelopeDetector5(int sampleRate, QObject *parent)
    : QObject(parent), m_sampleRate(qMax(8000, sampleRate))
{
    setParameters(1.0, 50.0, 100.0);
}

EnvelopeDetector5::~EnvelopeDetector5() = default;

/* ---- Configuration ---- */

void EnvelopeDetector5::setParameters(double attackMs, double releaseMs,
                                        double peakHoldMs)
{
    double attack = qMax(0.01, attackMs) / 1000.0;
    double release = qMax(0.1, releaseMs) / 1000.0;
    m_attackCoeff = qExp(-1.0 / (m_sampleRate * attack));
    m_releaseCoeff = qExp(-1.0 / (m_sampleRate * release));
    m_peakHoldSamples = qMax(1,
        static_cast<int>(m_sampleRate * peakHoldMs / 1000.0));
}

/* ---- Classify dynamics ---- */

EnvelopeDetector5::DynamicsClass EnvelopeDetector5::classify(
    double rmsToPeak) const
{
    if (rmsToPeak >= 0.9) return DynamicsClass::Flat;
    if (rmsToPeak >= 0.7) return DynamicsClass::Compressed;
    if (rmsToPeak >= 0.5) return DynamicsClass::Normal;
    if (rmsToPeak >= 0.3) return DynamicsClass::Dynamic;
    return DynamicsClass::Sparse;
}

/* ---- Process single sample ---- */

double EnvelopeDetector5::processSample(double sample)
{
    double absVal = qAbs(sample);

    // Peak envelope with attack/release
    if (absVal > m_peakEnvelope) {
        m_peakEnvelope = m_attackCoeff * m_peakEnvelope +
            (1.0 - m_attackCoeff) * absVal;
    } else {
        m_peakEnvelope = m_releaseCoeff * m_peakEnvelope +
            (1.0 - m_releaseCoeff) * absVal;
    }

    // Peak-hold decay modeling
    if (absVal > m_peakHoldValue) {
        m_peakHoldValue = absVal;
        m_peakHoldCounter = m_peakHoldSamples;
    } else if (m_peakHoldCounter > 0) {
        m_peakHoldCounter--;
    } else {
        // Decay peak-hold value
        m_peakHoldValue *= m_releaseCoeff;
    }

    // RMS envelope (exponential moving average of squared samples)
    m_rmsAccum = m_releaseCoeff * m_rmsAccum +
        (1.0 - m_releaseCoeff) * sample * sample;
    m_rmsEnvelope = qSqrt(qMax(0.0, m_rmsAccum));

    // Use peak-hold for output envelope
    return qMax(m_peakEnvelope, m_peakHoldValue);
}

/* ---- Process block ---- */

QVector<double> EnvelopeDetector5::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) return {};

    QVector<double> envelope(n);
    for (int i = 0; i < n; ++i)
        envelope[i] = processSample(input[i]);

    // Compute block-level RMS-to-peak
    double blockRms = 0.0;
    double blockPeak = 0.0;
    for (int i = 0; i < n; ++i) {
        blockRms += input[i] * input[i];
        double a = qAbs(input[i]);
        if (a > blockPeak) blockPeak = a;
    }
    blockRms = qSqrt(blockRms / n);

    double rtp = (blockPeak > 1e-10) ? blockRms / blockPeak : 0.0;

    m_stats.numSamples += n;
    m_stats.numBlocks++;
    m_rmsToPeakSum += rtp;
    m_stats.avgRmsToPeak = m_rmsToPeakSum / m_stats.numBlocks;
    m_stats.peakHoldMax = qMax(m_stats.peakHoldMax, m_peakHoldValue);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit blockProcessed(n, rtp, timer.elapsed());
    emit dynamicsClassified(m_stats.numBlocks - 1,
                              static_cast<int>(classify(rtp)));
    return envelope;
}

/* ---- Analyze block ---- */

EnvelopeDetector5::BlockResult EnvelopeDetector5::analyzeBlock(
    const QVector<double>& block) const
{
    BlockResult result;
    int n = block.size();
    if (n == 0) return result;

    double rms = 0.0;
    double peak = 0.0;
    for (int i = 0; i < n; ++i) {
        double s = block[i];
        rms += s * s;
        double a = qAbs(s);
        if (a > peak) peak = a;
    }
    rms = qSqrt(rms / n);

    result.peakEnvelope = peak;
    result.rmsEnvelope = rms;
    result.rmsToPeakRatio = (peak > 1e-10) ? rms / peak : 0.0;
    result.dynamicsClass = classify(result.rmsToPeakRatio);
    return result;
}

/* ---- Reset ---- */

void EnvelopeDetector5::reset()
{
    m_peakEnvelope = 0.0;
    m_rmsEnvelope = 0.0;
    m_peakHoldValue = 0.0;
    m_peakHoldCounter = 0;
    m_rmsAccum = 0.0;
    m_rmsCount = 0;
}

/* ---- Reset statistics ---- */

void EnvelopeDetector5::resetStatistics()
{
    reset();
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_rmsToPeakSum = 0.0;
}

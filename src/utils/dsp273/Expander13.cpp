/**
 * @file Expander13.cpp
 * @brief Expander13 实现
 *
 * 实现扩展器：向下并行压缩混合与瞬态保持透明动态范围增强。
 */

#include "utils/dsp273/Expander13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Expander13::Expander13(QObject *parent)
    : QObject(parent) {}

Expander13::~Expander13() = default;

/* ---- Configuration ---- */

void Expander13::setThreshold(double thresholdDb) { m_thresholdDb = qBound(-120.0, thresholdDb, 0.0); }
void Expander13::setRatio(double ratio) { m_ratio = qBound(1.0, ratio, 20.0); }
void Expander13::setAttack(double attackMs) { m_attackMs = qBound(0.01, attackMs, 500.0); }
void Expander13::setRelease(double releaseMs) { m_releaseMs = qBound(1.0, releaseMs, 5000.0); }
void Expander13::setBlend(double blend) { m_blend = qBound(0.0, blend, 1.0); }

/* ---- Compute gain from input level in dB ---- */

double Expander13::computeGain(double inputDb) const
{
    // Below threshold: expand (increase attenuation)
    // Above threshold: unity gain
    if (inputDb >= m_thresholdDb) return 0.0;

    double diff = m_thresholdDb - inputDb;
    // Expansion: output_dB = threshold + (input - threshold) / ratio
    double gainDb = diff * (1.0 - 1.0 / m_ratio);
    return gainDb;  // Negative = attenuation
}

/* ---- Transient detection via first-order difference ---- */

double Expander13::detectTransient(double input)
{
    double diff = qFabs(input - m_prevInput);
    m_prevInput = input;

    // Smooth transient coefficient
    double alpha = 0.1;
    m_transientCoeff = alpha * diff + (1.0 - alpha) * m_transientCoeff;

    // If transient detected (rapid change), return a boost factor
    double threshold = 0.05;
    if (m_transientCoeff > threshold) {
        double strength = qMin(m_transientCoeff / threshold, 2.0);
        return 1.0 + 0.5 * (strength - 1.0);  // Boost gain by up to 50%
    }
    return 1.0;
}

/* ---- Envelope follower with separate attack/release ---- */

double Expander13::smoothEnvelope(double inputLevel)
{
    double coeff;
    if (inputLevel > m_envelope) {
        // Attack: fast response to rising levels
        double attackCoeff = qExp(-1.0 / (m_attackMs * 0.001 * m_sampleRate));
        coeff = attackCoeff;
    } else {
        // Release: slower recovery
        double releaseCoeff = qExp(-1.0 / (m_releaseMs * 0.001 * m_sampleRate));
        coeff = releaseCoeff;
    }
    m_envelope = coeff * m_envelope + (1.0 - coeff) * inputLevel;
    return m_envelope;
}

/* ---- Process block ---- */

QVector<double> Expander13::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n);
    double sumGainReduction = 0.0;
    double peak = 0.0;

    for (int i = 0; i < n; ++i) {
        double absVal = qFabs(input[i]);
        if (absVal > peak) peak = absVal;

        // Envelope follower
        double env = smoothEnvelope(absVal);

        // Convert to dB
        double envDb = (env > 1e-10) ? 20.0 * qLn(env) / M_LN10 : -120.0;

        // Compute expansion gain
        double gainDb = computeGain(envDb);

        // Transient preservation: reduce gain reduction during transients
        double transientBoost = detectTransient(input[i]);
        gainDb *= (2.0 - transientBoost);  // Reduce attenuation during transients

        m_gainReduction = gainDb;
        sumGainReduction += gainDb;

        // Convert gain back to linear
        double gainLin = qPow(10.0, gainDb / 20.0);

        // Parallel compression blend: mix dry with expanded
        double expanded = input[i] * gainLin;
        output[i] = input[i] * (1.0 - m_blend) + expanded * m_blend;
    }

    double elapsed = timer.elapsed();
    m_stats.numSamples += n;
    m_stats.avgGainReduction = (m_stats.totalOps > 0)
        ? (m_stats.avgGainReduction * m_stats.totalOps + sumGainReduction / n) / (m_stats.totalOps + 1)
        : sumGainReduction / n;
    m_stats.peakLevel = qMax(m_stats.peakLevel, peak);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingDone(n, m_gainReduction, elapsed);

    return output;
}

/* ---- Accessors ---- */

double Expander13::gainReduction() const { return m_gainReduction; }

/* ---- Reset state ---- */

void Expander13::reset()
{
    m_envelope = 0.0;
    m_gainReduction = 0.0;
    m_prevInput = 0.0;
    m_transientCoeff = 0.0;
    m_prevTransient = 0.0;
}

/* ---- Reset statistics ---- */

void Expander13::resetStatistics()
{
    reset();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

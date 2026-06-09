/**
 * @file EnvelopeDetector7.cpp
 * @brief EnvelopeDetector7 实现
 *
 * 实现包络检测：峰值滤波器级联加权谐波强调与RMS-峰值比跟踪。
 */

#include "utils/signal258/EnvelopeDetector7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

EnvelopeDetector7::EnvelopeDetector7(QObject *parent)
    : QObject(parent)
{
    updateCoefficients();
}
EnvelopeDetector7::~EnvelopeDetector7() = default;

/* ---- Configuration ---- */

void EnvelopeDetector7::setAttack(double attackMs) { m_attackMs = qMax(0.01, attackMs); updateCoefficients(); }
void EnvelopeDetector7::setRelease(double releaseMs) { m_releaseMs = qMax(0.1, releaseMs); updateCoefficients(); }
void EnvelopeDetector7::setSampleRate(double rate) { m_sampleRate = qMax(8000.0, rate); updateCoefficients(); }

void EnvelopeDetector7::setPeakingStages(const QVector<PeakingStage>& stages)
{
    m_filters.clear();
    for (const PeakingStage& s : stages)
        m_filters.append(designPeakingFilter(s));
}

/* ---- Update coefficients ---- */

void EnvelopeDetector7::updateCoefficients()
{
    double sr = qMax(m_sampleRate, 1.0);
    m_attackCoeff = 1.0 - qExp(-1.0 / (m_attackMs * 0.001 * sr));
    m_releaseCoeff = 1.0 - qExp(-1.0 / (m_releaseMs * 0.001 * sr));
}

/* ---- Design biquad peaking filter ---- */

EnvelopeDetector7::BiquadState EnvelopeDetector7::designPeakingFilter(
    const PeakingStage& stage) const
{
    BiquadState bq;
    double A = qPow(10.0, stage.gain / 40.0); // Gain in dB -> linear
    double w0 = 2.0 * M_PI * stage.centerFreq / m_sampleRate;
    double cosW0 = qCos(w0);
    double sinW0 = qSin(w0);
    double alpha = sinW0 / (2.0 * qMax(stage.q, 0.01));

    double sqrtA = qSqrt(A);
    bq.b0 = 1.0 + alpha * A;
    bq.b1 = -2.0 * cosW0;
    bq.b2 = 1.0 - alpha * A;
    double a0 = 1.0 + alpha / A;
    bq.a1 = 2.0 * cosW0;
    bq.a2 = -(1.0 - alpha / A);

    // Normalize by a0
    bq.b0 /= a0;
    bq.b1 /= a0;
    bq.b2 /= a0;
    bq.a1 /= a0;
    bq.a2 /= a0;

    return bq;
}

/* ---- Apply peaking filter cascade ---- */

double EnvelopeDetector7::applyPeakingCascade(double sample)
{
    double x = sample;
    for (int i = 0; i < m_filters.size(); ++i) {
        BiquadState& f = m_filters[i];
        double y = f.b0 * x + f.b1 * f.x1 + f.b2 * f.x2
                   - f.a1 * f.y1 - f.a2 * f.y2;
        f.x2 = f.x1;
        f.x1 = x;
        f.y2 = f.y1;
        f.y1 = y;
        x = y;
    }
    return x;
}

/* ---- Envelope follower ---- */

double EnvelopeDetector7::followEnvelope(double sample)
{
    double absSample = qAbs(sample);
    if (absSample > m_envelope) {
        // Attack
        m_envelope += m_attackCoeff * (absSample - m_envelope);
    } else {
        // Release
        m_envelope -= m_releaseCoeff * (m_envelope - absSample);
    }
    return m_envelope;
}

/* ---- Compute RMS ---- */

double EnvelopeDetector7::computeRMS(const QVector<double>& env) const
{
    if (env.isEmpty()) return 0.0;
    double sum = 0.0;
    for (double v : env)
        sum += v * v;
    return qSqrt(sum / env.size());
}

/* ---- Main process ---- */

QVector<double> EnvelopeDetector7::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> envelope;
    envelope.reserve(n);

    m_envelope = 0.0;

    for (int i = 0; i < n; ++i) {
        // Step 1: Apply peaking filter cascade for harmonic emphasis
        double emphasized = applyPeakingCascade(input[i]);

        // Step 2: Full-wave rectification
        double rectified = qAbs(emphasized);

        // Step 3: Envelope follower (attack/release smoothing)
        double env = followEnvelope(rectified);
        envelope.append(env);
    }

    m_lastEnvelope = envelope;

    // Compute statistics
    double peak = 0.0;
    for (double v : envelope)
        peak = qMax(peak, v);
    double rms = computeRMS(envelope);
    double ratio = (peak > 1e-10) ? rms / peak : 0.0;

    double elapsed = timer.elapsed();
    m_stats.numSamples = n;
    m_stats.peakEnvelope = peak;
    m_stats.rmsEnvelope = rms;
    m_stats.rmsToPeakRatio = ratio;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit detectionCompleted(n, peak, ratio, elapsed);
    return envelope;
}

/* ---- RMS-to-peak ratio ---- */

double EnvelopeDetector7::rmsToPeakRatio() const
{
    if (m_lastEnvelope.isEmpty()) return 0.0;
    double peak = 0.0;
    for (double v : m_lastEnvelope)
        peak = qMax(peak, v);
    double rms = computeRMS(m_lastEnvelope);
    return (peak > 1e-10) ? rms / peak : 0.0;
}

/* ---- Envelope at index ---- */

double EnvelopeDetector7::envelopeAt(int index) const
{
    if (index < 0 || index >= m_lastEnvelope.size()) return 0.0;
    return m_lastEnvelope[index];
}

/* ---- Reset ---- */

void EnvelopeDetector7::resetStatistics()
{
    m_envelope = 0.0;
    m_filters.clear();
    m_lastEnvelope.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

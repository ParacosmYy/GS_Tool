/**
 * @file SignalGenerator5.cpp
 * @brief SignalGenerator5 实现
 *
 * 实现加法合成信号发生器：Fourier部分波叠加与包络脚本控制。
 */

#include "utils/signal229/SignalGenerator5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SignalGenerator5::SignalGenerator5(QObject *parent) : QObject(parent) {}
SignalGenerator5::~SignalGenerator5() = default;

/* ---- Configuration ---- */

void SignalGenerator5::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(1.0, sampleRate);
    m_stats.sampleRate = m_sampleRate;
}

void SignalGenerator5::setPartials(const QVector<Partial>& partials)
{
    m_partials = partials;
    m_stats.numPartials = partials.size();
}

void SignalGenerator5::setAmplitudeEnvelope(
    const QVector<EnvelopePoint>& points)
{
    m_ampEnvelope = points;
    std::sort(m_ampEnvelope.begin(), m_ampEnvelope.end(),
              [](const EnvelopePoint& a, const EnvelopePoint& b) {
                  return a.time < b.time;
              });
}

void SignalGenerator5::setPhaseEnvelope(const QVector<EnvelopePoint>& points)
{
    m_phaseEnvelope = points;
    std::sort(m_phaseEnvelope.begin(), m_phaseEnvelope.end(),
              [](const EnvelopePoint& a, const EnvelopePoint& b) {
                  return a.time < b.time;
              });
}

void SignalGenerator5::addPartial(double freq, double amp, double phase)
{
    Partial p;
    p.frequency = qMax(0.0, freq);
    p.amplitude = amp;
    p.phase = phase;
    m_partials.append(p);
    m_stats.numPartials = m_partials.size();
}

/* ---- Interpolate envelope ---- */

double SignalGenerator5::interpolateEnvelope(
    const QVector<EnvelopePoint>& env, double t) const
{
    if (env.isEmpty()) return 1.0;
    if (env.size() == 1) return env[0].value;
    if (t <= env.first().time) return env.first().value;
    if (t >= env.last().time) return env.last().value;

    // Binary search for segment
    int lo = 0, hi = env.size() - 1;
    while (lo < hi - 1) {
        int mid = (lo + hi) / 2;
        if (env[mid].time <= t) lo = mid;
        else hi = mid;
    }

    // Linear interpolation
    double t0 = env[lo].time;
    double t1 = env[hi].time;
    double v0 = env[lo].value;
    double v1 = env[hi].value;
    double frac = (t1 > t0) ? (t - t0) / (t1 - t0) : 0.0;
    return v0 + frac * (v1 - v0);
}

/* ---- Evaluate at time t ---- */

double SignalGenerator5::evaluateAt(double t) const
{
    double ampEnv = interpolateEnvelope(m_ampEnvelope, t);
    double phaseEnv = interpolateEnvelope(m_phaseEnvelope, t);

    double sample = 0.0;
    for (const auto& p : m_partials) {
        double omega = 2.0 * M_PI * p.frequency;
        double phase = p.phase + phaseEnv * 2.0 * M_PI;
        sample += p.amplitude * qSin(omega * t + phase);
    }
    return sample * ampEnv;
}

/* ---- Generate for duration ---- */

QVector<double> SignalGenerator5::generate(double duration)
{
    duration = qMax(0.0, duration);
    int numSamples = static_cast<int>(duration * m_sampleRate);
    return generateSamples(numSamples);
}

/* ---- Generate N samples ---- */

QVector<double> SignalGenerator5::generateSamples(int numSamples)
{
    QElapsedTimer timer;
    timer.start();

    numSamples = qMax(0, numSamples);
    QVector<double> output(numSamples, 0.0);
    double dt = 1.0 / m_sampleRate;

    for (int i = 0; i < numSamples; ++i) {
        double t = i * dt;
        output[i] = evaluateAt(t);
    }

    // Normalize peak amplitude to [-1, 1] if needed
    double peak = 0.0;
    for (int i = 0; i < numSamples; ++i)
        peak = qMax(peak, qAbs(output[i]));
    if (peak > 1.0) {
        double scale = 1.0 / peak;
        for (int i = 0; i < numSamples; ++i)
            output[i] *= scale;
    }

    m_stats.numSamples = numSamples;
    m_stats.duration = numSamples / m_sampleRate;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit generationCompleted(
        numSamples, m_stats.duration, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void SignalGenerator5::resetStatistics()
{
    m_partials.clear();
    m_ampEnvelope.clear();
    m_phaseEnvelope.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

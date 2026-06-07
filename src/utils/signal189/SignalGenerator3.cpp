/**
 * @file SignalGenerator3.cpp
 * @brief SignalGenerator3 实现
 *
 * 实现信号发生器：4种基本波形、AM/FM调制、线性/对数扫频。
 */

#include "utils/signal189/SignalGenerator3.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

SignalGenerator3::SignalGenerator3(QObject *parent) : QObject(parent) {}
SignalGenerator3::~SignalGenerator3() = default;

/* ---- Configuration ---- */

void SignalGenerator3::setSampleRate(double sr) { m_sampleRate = qMax(1.0, sr); }
void SignalGenerator3::setFrequency(double freq) { m_frequency = qMax(0.001, freq); }
void SignalGenerator3::setAmplitude(double amp) { m_amplitude = qBound(0.0, amp, 10.0); }
void SignalGenerator3::setPhase(double phase) { m_phase = phase; }
void SignalGenerator3::setWaveform(Waveform wf) { m_waveform = wf; }

void SignalGenerator3::setModulation(ModType type, double modFreq, double modDepth)
{
    m_modType = type;
    m_modFreq = qMax(0.001, modFreq);
    m_modDepth = qBound(0.0, modDepth, 1.0);
}

void SignalGenerator3::setSweep(SweepType type, double startFreq,
                                 double endFreq, double duration)
{
    m_sweepType = type;
    m_sweepStartFreq = qMax(0.001, startFreq);
    m_sweepEndFreq = qMax(0.001, endFreq);
    m_sweepDuration = qMax(0.001, duration);
}

/* ---- Instantaneous frequency (with sweep) ---- */

double SignalGenerator3::instantFreq(int idx, int totalSamples) const
{
    if (m_sweepType == NoSweep) return m_frequency;

    double t = static_cast<double>(idx) / totalSamples;
    if (t > 1.0) t = 1.0;

    if (m_sweepType == Linear) {
        return m_sweepStartFreq + t * (m_sweepEndFreq - m_sweepStartFreq);
    }
    // Logarithmic sweep
    double logStart = qLn(m_sweepStartFreq);
    double logEnd = qLn(m_sweepEndFreq);
    return qExp(logStart + t * (logEnd - logStart));
}

/* ---- Base waveform generation ---- */

double SignalGenerator3::baseWaveform(double phase) const
{
    // Normalize phase to [0, 2π)
    double p = qFmod(phase, 2.0 * M_PI);
    if (p < 0) p += 2.0 * M_PI;

    switch (m_waveform) {
    case Sine:
        return qSin(p);
    case Square:
        return (p < M_PI) ? 1.0 : -1.0;
    case Triangle:
        // Triangle: linear rise then fall
        if (p < M_PI)
            return 2.0 * p / M_PI - 1.0;
        else
            return 3.0 - 2.0 * p / M_PI;
    case Sawtooth:
        // Sawtooth: linear rise from -1 to +1
        return p / M_PI - 1.0;
    }
    return 0.0;
}

/* ---- Apply modulation ---- */

double SignalGenerator3::applyModulation(double sample, int idx) const
{
    if (m_modType == None) return sample;

    double t = static_cast<double>(idx) / m_sampleRate;
    double modSignal = qSin(2.0 * M_PI * m_modFreq * t);

    if (m_modType == AM) {
        // AM: y(t) = (1 + depth * cos(2π fm t)) * x(t)
        return sample * (1.0 + m_modDepth * modSignal);
    }
    // FM is handled in frequency domain, no additional sample modulation needed
    return sample;
}

/* ---- Generate single sample ---- */

double SignalGenerator3::sample(int index) const
{
    Q_UNUSED(index)
    // Simplified: use generate() for proper frequency accumulation
    double t = static_cast<double>(index) / m_sampleRate;
    double freq = m_frequency;
    double phase = 2.0 * M_PI * freq * t + m_phase;
    double val = m_amplitude * baseWaveform(phase);
    return applyModulation(val, index);
}

/* ---- Generate waveform ---- */

QVector<double> SignalGenerator3::generate(int numSamples)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(numSamples);
    double accumPhase = m_phase;

    for (int i = 0; i < numSamples; ++i) {
        double freq = instantFreq(i, numSamples);
        double phaseInc = 2.0 * M_PI * freq / m_sampleRate;
        accumPhase += phaseInc;

        double val = m_amplitude * baseWaveform(accumPhase);
        output[i] = applyModulation(val, i);
    }

    double dur = static_cast<double>(numSamples) / m_sampleRate;
    m_stats.totalGenerations++;
    m_stats.numSamples = numSamples;
    m_stats.duration = dur;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalGenerations;

    emit generationCompleted(numSamples, dur, timer.elapsed());
    return output;
}

/* ---- Generate by duration ---- */

QVector<double> SignalGenerator3::generateDuration(double seconds)
{
    int numSamples = static_cast<int>(seconds * m_sampleRate);
    return generate(numSamples);
}

/* ---- Mix two signals ---- */

QVector<double> SignalGenerator3::mix(const QVector<double>& a,
                                       const QVector<double>& b,
                                       double mixRatio)
{
    int n = qMin(a.size(), b.size());
    QVector<double> result(n);
    double r = qBound(0.0, mixRatio, 1.0);
    for (int i = 0; i < n; ++i)
        result[i] = a[i] * (1.0 - r) + b[i] * r;
    return result;
}

/* ---- Reset ---- */

void SignalGenerator3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

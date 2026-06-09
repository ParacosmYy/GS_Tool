/**
 * @file SignalGenerator7.cpp
 * @brief SignalGenerator7 实现
 *
 * 实现信号发生器：AM/FM/PM调制与预定义波形插值变形。
 */

#include "utils/signal257/SignalGenerator7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cstdlib>

/* ---- Construction / Destruction ---- */

SignalGenerator7::SignalGenerator7(QObject *parent)
    : QObject(parent) {}
SignalGenerator7::~SignalGenerator7() = default;

/* ---- Configuration ---- */

void SignalGenerator7::setSampleRate(int sr) { m_sampleRate = qMax(1, sr); }
void SignalGenerator7::setFrequency(double freq) { m_frequency = qMax(0.001, freq); }
void SignalGenerator7::setAmplitude(double amp) { m_amplitude = qBound(0.0, amp, 1.0); }
void SignalGenerator7::setShape(Shape shape) { m_shape = shape; }

void SignalGenerator7::setModulation(ModType type, double modFreq, double modDepth)
{
    m_modType = type;
    m_modFreq = qMax(0.001, modFreq);
    m_modDepth = qBound(0.0, modDepth, 1.0);
}

/* ---- Generate noise sample (uniform) ---- */

double SignalGenerator7::noiseSample() const
{
    return 2.0 * (static_cast<double>(qrand()) / RAND_MAX) - 1.0;
}

/* ---- Sample a waveform shape at given phase ---- */

double SignalGenerator7::sampleShape(Shape shape, double phase) const
{
    // Normalize phase to [0, 2*pi)
    double p = qFmod(phase, 2.0 * M_PI);
    if (p < 0) p += 2.0 * M_PI;

    switch (shape) {
    case Sine:
        return qSin(p);

    case Square:
        return (p < M_PI) ? 1.0 : -1.0;

    case Triangle:
        // Triangle: linear ramp up 0..pi, down pi..2*pi
        if (p < M_PI)
            return -1.0 + 2.0 * p / M_PI;
        else
            return 3.0 - 2.0 * p / M_PI;

    case Sawtooth:
        // Sawtooth: linear ramp from -1 to +1
        return -1.0 + p / M_PI;

    case Noise:
        return noiseSample();

    default:
        return qSin(p);
    }
}

/* ---- Apply modulation ---- */

double SignalGenerator7::applyModulation(double sample, double t) const
{
    switch (m_modType) {
    case AM:
        // AM: amplitude envelope
        return sample * (1.0 + m_modDepth * qSin(2.0 * M_PI * m_modFreq * t));

    case FM: {
        // FM is handled in generate() by modulating phase
        return sample;
    }

    case PM:
        // PM is handled in generate() by modulating phase
        return sample;

    case None:
    default:
        return sample;
    }
}

/* ---- Generate samples ---- */

QVector<double> SignalGenerator7::generate(int numSamples)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    output.reserve(numSamples);

    double phaseInc = 2.0 * M_PI * m_frequency / m_sampleRate;

    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / m_sampleRate;

        // Apply FM/PM by modulating phase
        double modPhase = 0.0;
        if (m_modType == FM) {
            // FM: phase deviation proportional to integral of modulating signal
            modPhase = m_modDepth * m_frequency *
                       qSin(2.0 * M_PI * m_modFreq * t) / m_modFreq;
        } else if (m_modType == PM) {
            // PM: phase deviation proportional to modulating signal
            modPhase = m_modDepth * qSin(2.0 * M_PI * m_modFreq * t);
        }

        double sample = sampleShape(m_shape, m_phase + modPhase);
        sample *= m_amplitude;

        // Apply AM modulation
        sample = applyModulation(sample, t);

        output.append(sample);
        m_phase += phaseInc;
        if (m_phase > 2.0 * M_PI * 1000.0)
            m_phase -= 2.0 * M_PI * 1000.0; // Prevent overflow
    }

    m_stats.numSamplesGenerated += numSamples;
    m_stats.sampleRate = m_sampleRate;
    m_stats.totalOps++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit generationCompleted(numSamples, elapsed);
    return output;
}

/* ---- Generate morphed waveform between two shapes ---- */

QVector<double> SignalGenerator7::generateMorphed(int numSamples, Shape from,
                                                    Shape to, double alpha)
{
    QElapsedTimer timer;
    timer.start();

    alpha = qBound(0.0, alpha, 1.0);

    QVector<double> output;
    output.reserve(numSamples);

    double phaseInc = 2.0 * M_PI * m_frequency / m_sampleRate;

    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / m_sampleRate;

        double modPhase = 0.0;
        if (m_modType == FM) {
            modPhase = m_modDepth * m_frequency *
                       qSin(2.0 * M_PI * m_modFreq * t) / m_modFreq;
        } else if (m_modType == PM) {
            modPhase = m_modDepth * qSin(2.0 * M_PI * m_modFreq * t);
        }

        // Sample both shapes and interpolate (morphing)
        double sFrom = sampleShape(from, m_phase + modPhase);
        double sTo = sampleShape(to, m_phase + modPhase);
        double sample = (1.0 - alpha) * sFrom + alpha * sTo;
        sample *= m_amplitude;

        sample = applyModulation(sample, t);

        output.append(sample);
        m_phase += phaseInc;
        if (m_phase > 2.0 * M_PI * 1000.0)
            m_phase -= 2.0 * M_PI * 1000.0;
    }

    m_stats.numSamplesGenerated += numSamples;
    m_stats.totalOps++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit generationCompleted(numSamples, elapsed);
    return output;
}

/* ---- Reset ---- */

void SignalGenerator7::resetStatistics()
{
    m_phase = 0.0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

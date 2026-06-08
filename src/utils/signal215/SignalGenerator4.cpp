/**
 * @file SignalGenerator4.cpp
 * @brief SignalGenerator4 实现
 *
 * 实现信号发生器：多项式波形整形、AM/FM调制、带限合成。
 */

#include "utils/signal215/SignalGenerator4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SignalGenerator4::SignalGenerator4(QObject *parent) : QObject(parent) {}
SignalGenerator4::~SignalGenerator4() = default;

/* ---- Configuration ---- */

void SignalGenerator4::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(8000.0, sampleRate);
}

void SignalGenerator4::setWaveform(Waveform type, double frequency,
                                    double amplitude)
{
    m_waveform = type;
    m_frequency = qBound(0.01, frequency, m_sampleRate * 0.45);
    m_amplitude = qBound(0.0, amplitude, 1.0);
}

void SignalGenerator4::setAM(double modDepth, double modFrequency)
{
    m_amDepth = qBound(0.0, modDepth, 1.0);
    m_amFreq = qMax(0.0, modFrequency);
}

void SignalGenerator4::setFM(double modIndex, double modFrequency)
{
    m_fmIndex = qBound(0.0, modIndex, 100.0);
    m_fmFreq = qMax(0.0, modFrequency);
}

void SignalGenerator4::setArbitraryWaveform(const QVector<double>& shape)
{
    m_arbTable = shape;
    if (!m_arbTable.isEmpty())
        m_waveform = Arbitrary;
}

/* ---- Wrap phase to [0, 1) ---- */

double SignalGenerator4::wrapPhase(double phase)
{
    phase = qFabs(phase);
    return phase - qFloor(phase);
}

/* ---- Raw waveform value at phase [0,1) ---- */

double SignalGenerator4::rawWaveform(double phase) const
{
    switch (m_waveform) {
    case Sine:
        return qSin(2.0 * M_PI * phase);
    case Square:
        return (phase < 0.5) ? 1.0 : -1.0;
    case Sawtooth:
        return 2.0 * phase - 1.0;
    case Triangle:
        return (phase < 0.5) ? (4.0 * phase - 1.0) : (3.0 - 4.0 * phase);
    case Pulse:
        return (phase < 0.25) ? 1.0 : -1.0;
    case Arbitrary: {
        if (m_arbTable.isEmpty()) return 0.0;
        int n = m_arbTable.size();
        double idx = phase * n;
        int i0 = qBound(0, static_cast<int>(qFloor(idx)), n - 1);
        int i1 = (i0 + 1) % n;
        double frac = idx - qFloor(idx);
        return m_arbTable[i0] * (1.0 - frac) + m_arbTable[i1] * frac;
    }
    }
    return 0.0;
}

/* ---- PolyBLEP for band-limited anti-aliasing ---- */

double SignalGenerator4::polyBlep(double t, double phaseIncrement) const
{
    double dt = phaseIncrement;
    if (t < dt) {
        // Discontinuity at 0: ramp correction
        double x = t / dt;
        return x + x - x * x - 1.0;
    }
    if (t > 1.0 - dt) {
        // Discontinuity at 1: ramp correction
        double x = (t - 1.0) / dt;
        return x * x + x + x + 1.0;
    }
    return 0.0;
}

/* ---- Advance phase ---- */

double SignalGenerator4::advancePhase()
{
    double fmMod = 0.0;
    if (m_fmIndex > 0.0 && m_fmFreq > 0.0) {
        // FM: modulate instantaneous frequency
        fmMod = m_fmIndex * m_fmFreq / m_sampleRate
                * qSin(2.0 * M_PI * m_fmFreq * m_phase * m_sampleRate / m_frequency);
    }

    double phaseInc = (m_frequency + fmMod) / m_sampleRate;
    m_phase = wrapPhase(m_phase + phaseInc);
    return phaseInc;
}

/* ---- Generate single sample ---- */

double SignalGenerator4::generateOne()
{
    double phaseInc = m_frequency / m_sampleRate;
    double sample = rawWaveform(m_phase);

    // Apply polyBLEP for band-limited synthesis on discontinuous waveforms
    if (m_waveform == Square || m_waveform == Sawtooth ||
        m_waveform == Pulse) {
        double blep = polyBlep(m_phase, phaseInc);
        if (m_waveform == Sawtooth) {
            sample -= blep;
        } else {
            // Square/Pulse: two discontinuities per cycle
            sample += blep;
            sample -= polyBlep(wrapPhase(m_phase + 0.5), phaseInc);
        }
    }

    // AM modulation
    if (m_amDepth > 0.0 && m_amFreq > 0.0) {
        double amMod = 1.0 + m_amDepth *
            qSin(2.0 * M_PI * m_amFreq * m_phase * m_sampleRate / m_frequency);
        sample *= amMod;
    }

    advancePhase();
    return sample * m_amplitude;
}

/* ---- Generate buffer ---- */

QVector<double> SignalGenerator4::generate(int numSamples)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(numSamples, 0.0);
    for (int i = 0; i < numSamples; ++i)
        output[i] = generateOne();

    m_stats.totalGenerated += numSamples;
    m_stats.sampleRate = m_sampleRate;
    m_stats.frequency = m_frequency;
    m_stats.amplitude = m_amplitude;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalGenerated > 0)
        ? m_timeSum * 1000.0 / m_stats.totalGenerated : 0.0;
    emit generationCompleted(numSamples, timer.elapsed());

    return output;
}

/* ---- Reset ---- */

void SignalGenerator4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_phase = 0.0;
}

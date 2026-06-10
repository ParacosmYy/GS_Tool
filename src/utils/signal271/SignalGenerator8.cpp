/**
 * @file SignalGenerator8.cpp
 * @brief SignalGenerator8 实现
 *
 * 实现信号发生器：任意波形合成与DDS相位累加器精确频率控制。
 */

#include "utils/signal271/SignalGenerator8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SignalGenerator8::SignalGenerator8(QObject *parent)
    : QObject(parent)
{
    updatePhaseIncrement();
}

SignalGenerator8::~SignalGenerator8() = default;

/* ---- Configuration ---- */

void SignalGenerator8::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
    updatePhaseIncrement();
}

void SignalGenerator8::setFrequency(double freq)
{
    m_frequency = qBound(0.0, freq, m_sampleRate / 2.0);
    updatePhaseIncrement();
}

void SignalGenerator8::setAmplitude(double amp)
{
    m_amplitude = amp;
}

void SignalGenerator8::setOffset(double offset)
{
    m_offset = offset;
}

void SignalGenerator8::setPhaseBits(int bits)
{
    m_phaseBits = qBound(8, bits, 64);
    updatePhaseIncrement();
}

void SignalGenerator8::setWaveform(WaveformType type)
{
    m_waveform = type;
}

void SignalGenerator8::setArbitraryWaveform(const QVector<double>& table)
{
    m_arbTable = table;
    // Normalize to -1..1
    double maxVal = 0.0;
    for (double v : m_arbTable)
        if (qAbs(v) > maxVal) maxVal = qAbs(v);
    if (maxVal > 1e-10) {
        for (int i = 0; i < m_arbTable.size(); ++i)
            m_arbTable[i] /= maxVal;
    }
    m_waveform = Arbitrary;
}

/* ---- Phase increment computation ---- */

void SignalGenerator8::updatePhaseIncrement()
{
    // Phase increment = f * 2^N / Fs
    quint64 maxPhase = 1ULL << m_phaseBits;
    m_phaseInc = static_cast<quint64>(
        m_frequency * static_cast<double>(maxPhase) / m_sampleRate);
}

/* ---- Generate one sample from phase ---- */

double SignalGenerator8::sampleFromPhase(quint64 phase) const
{
    // Convert phase to 0..1 range
    double t = static_cast<double>(phase) / static_cast<double>(1ULL << m_phaseBits);

    switch (m_waveform) {
    case Sine:
        return qSin(2.0 * M_PI * t);

    case Square:
        return (t < 0.5) ? 1.0 : -1.0;

    case Triangle:
        return (t < 0.5) ? (4.0 * t - 1.0) : (3.0 - 4.0 * t);

    case Sawtooth:
        return 2.0 * t - 1.0;

    case Arbitrary: {
        if (m_arbTable.isEmpty()) return 0.0;
        int len = m_arbTable.size();
        double idx = t * len;
        int i0 = static_cast<int>(idx) % len;
        int i1 = (i0 + 1) % len;
        double frac = idx - static_cast<int>(idx);
        // Linear interpolation
        return m_arbTable[i0] * (1.0 - frac) + m_arbTable[i1] * frac;
    }
    }
    return 0.0;
}

/* ---- Generate N samples ---- */

QVector<double> SignalGenerator8::generate(int numSamples)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(numSamples);
    quint64 maxPhase = 1ULL << m_phaseBits;

    for (int i = 0; i < numSamples; ++i) {
        output[i] = m_amplitude * sampleFromPhase(m_phaseAcc) + m_offset;
        m_phaseAcc = (m_phaseAcc + m_phaseInc) % maxPhase;
    }

    double elapsed = timer.elapsed();
    m_stats.numSamples = numSamples;
    m_stats.sampleRate = m_sampleRate;
    m_stats.frequency = m_frequency;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit generationCompleted(numSamples, m_frequency, elapsed);

    return output;
}

/* ---- Generate chirp signal ---- */

QVector<double> SignalGenerator8::chirp(double fStart, double fEnd, int numSamples)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(numSamples);
    double phase = 0.0;

    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / m_sampleRate;
        // Instantaneous frequency: linear sweep
        double freq = fStart + (fEnd - fStart) * static_cast<double>(i) / numSamples;
        // Phase integration: phase += 2*pi*f(t)/Fs
        phase += 2.0 * M_PI * freq / m_sampleRate;

        output[i] = m_amplitude * qSin(phase) + m_offset;
    }

    double elapsed = timer.elapsed();
    m_stats.numSamples = numSamples;
    m_stats.sampleRate = m_sampleRate;
    m_stats.frequency = fStart;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit generationCompleted(numSamples, fStart, elapsed);

    return output;
}

/* ---- Reset phase ---- */

void SignalGenerator8::resetPhase()
{
    m_phaseAcc = 0;
}

/* ---- Reset statistics ---- */

void SignalGenerator8::resetStatistics()
{
    m_phaseAcc = 0;
    m_arbTable.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

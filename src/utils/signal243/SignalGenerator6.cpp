/**
 * @file SignalGenerator6.cpp
 * @brief SignalGenerator6 实现
 *
 * 实现信号发生器：任意波形定义与多音合成可配置相位偏移。
 */

#include "utils/signal243/SignalGenerator6.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

SignalGenerator6::SignalGenerator6(QObject *parent) : QObject(parent) {}
SignalGenerator6::~SignalGenerator6() = default;

/* ---- Configuration ---- */

void SignalGenerator6::setSampleRate(int rate) { m_sampleRate = qMax(1000, rate); }

void SignalGenerator6::setTone(const Tone& tone)
{
    m_tones = { tone };
    m_type = Sine;
}

void SignalGenerator6::setTones(const QVector<Tone>& tones)
{
    m_tones = tones;
    m_type = Sine;
}

void SignalGenerator6::setArbitraryWaveform(const QVector<double>& shape)
{
    m_arbWaveform = shape;
    if (!shape.isEmpty()) m_type = Arbitrary;
}

/* ---- Evaluate waveform ---- */

double SignalGenerator6::evalWaveform(double t) const
{
    // t is normalized phase [0..1)
    double phase = t - qFloor(t);

    switch (m_type) {
    case Sine:
        return qSin(2.0 * M_PI * phase);
    case Square:
        return (phase < 0.5) ? 1.0 : -1.0;
    case Triangle:
        return (phase < 0.5) ? (4.0 * phase - 1.0) : (3.0 - 4.0 * phase);
    case Sawtooth:
        return 2.0 * phase - 1.0;
    case Arbitrary:
        return evalArbitrary(phase);
    }
    return 0.0;
}

double SignalGenerator6::evalArbitrary(double t) const
{
    if (m_arbWaveform.isEmpty()) return 0.0;
    int n = m_arbWaveform.size();
    double idx = t * n;
    int i0 = qFloor(idx) % n;
    int i1 = (i0 + 1) % n;
    double frac = idx - qFloor(idx);
    return m_arbWaveform[i0] * (1.0 - frac) + m_arbWaveform[i1] * frac;
}

/* ---- Generate for duration ---- */

QVector<double> SignalGenerator6::generate(double durationMs)
{
    int numSamples = static_cast<int>(m_sampleRate * durationMs / 1000.0);
    return generateSamples(numSamples);
}

/* ---- Generate N samples ---- */

QVector<double> SignalGenerator6::generateSamples(int numSamples)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    output.resize(qMax(0, numSamples));

    if (m_tones.isEmpty() && m_type != Arbitrary) {
        m_stats.totalOps++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
        return output;
    }

    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / m_sampleRate;
        double sample = 0.0;

        if (!m_tones.isEmpty()) {
            // Multi-tone synthesis
            for (const Tone& tone : m_tones) {
                double phase = 2.0 * M_PI * tone.frequency * t + tone.phaseOffset;
                sample += tone.amplitude * qSin(phase);
            }
        } else if (m_type == Arbitrary) {
            double freq = (m_arbWaveform.size() > 0) ? 440.0 : 0.0;
            double phase = freq * t;
            sample = evalArbitrary(phase - qFloor(phase));
        }

        output[i] = sample;
    }

    m_stats.sampleRate = m_sampleRate;
    m_stats.numTones = m_tones.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit generationCompleted(numSamples, 0.0, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void SignalGenerator6::resetStatistics()
{
    m_stats = Stats{}; m_timeSum = 0.0;
}

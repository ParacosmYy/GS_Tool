/**
 * @file SignalGenerator2.cpp
 * @brief SignalGenerator2 实现
 *
 * 实现多波形信号发生器：基本波形、AM/FM/PM调制、Chirp线性调频。
 */

#include "utils/signal173/SignalGenerator2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <random>

/* ---- Construction / Destruction ---- */

SignalGenerator2::SignalGenerator2(QObject *parent)
    : QObject(parent)
{
}

SignalGenerator2::~SignalGenerator2() = default;

/* ---- Configuration ---- */

void SignalGenerator2::setSampleRate(double rate) { m_sampleRate = qMax(1.0, rate); }
void SignalGenerator2::setFrequency(double freq) { m_frequency = qMax(0.0, freq); }
void SignalGenerator2::setAmplitude(double amp) { m_amplitude = amp; }
void SignalGenerator2::setPhase(double phase) { m_phase = phase; }
void SignalGenerator2::setWaveform(Waveform type) { m_waveform = type; }
void SignalGenerator2::setModulation(const ModParams& mod) { m_mod = mod; }
void SignalGenerator2::setArbitraryWaveform(const QVector<double>& waveform) { m_arbitrary = waveform; }
void SignalGenerator2::setDCOffset(double offset) { m_dcOffset = offset; }

/* ---- Basic waveform function ---- */

double SignalGenerator2::waveformFunc(double phase) const
{
    /* Normalize phase to [0, 2*pi) */
    double p = qFmod(phase, 2.0 * M_PI);
    if (p < 0) p += 2.0 * M_PI;

    switch (m_waveform) {
    case Sine:
        return qSin(p);

    case Square:
        return (p < M_PI) ? 1.0 : -1.0;

    case Triangle:
        return (p < M_PI) ? (-1.0 + 2.0 * p / M_PI) : (3.0 - 2.0 * p / M_PI);

    case Sawtooth:
        return 1.0 - p / M_PI;

    case Noise:
        return 0.0; /* Handled in generate() */

    case Arbitrary:
        return arbitraryInterp(p);

    default:
        return qSin(p);
    }
}

double SignalGenerator2::arbitraryInterp(double phase) const
{
    if (m_arbitrary.isEmpty()) return 0.0;
    double p = qFmod(phase, 2.0 * M_PI);
    if (p < 0) p += 2.0 * M_PI;
    double frac = p / (2.0 * M_PI) * m_arbitrary.size();
    int idx0 = qBound(0, static_cast<int>(qFloor(frac)), m_arbitrary.size() - 1);
    int idx1 = (idx0 + 1) % m_arbitrary.size();
    double t = frac - qFloor(frac);
    return m_arbitrary[idx0] * (1.0 - t) + m_arbitrary[idx1] * t;
}

/* ---- Generate ---- */

QVector<double> SignalGenerator2::generate(int numSamples)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(numSamples);
    double omega = 2.0 * M_PI * m_frequency / m_sampleRate;

    /* Setup RNG for noise */
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / m_sampleRate;

        if (m_waveform == Noise) {
            output[i] = m_amplitude * dist(rng) + m_dcOffset;
            continue;
        }

        double phase = m_phase + omega * i;

        /* Apply modulation */
        switch (m_mod.type) {
        case AM: {
            double modSig = 0.5 * (1.0 + m_mod.modDepth * qSin(2.0 * M_PI * m_mod.modFreq * t + m_mod.modPhase));
            output[i] = m_amplitude * modSig * waveformFunc(phase) + m_dcOffset;
            break;
        }
        case FM: {
            double freqDev = m_mod.modDepth * qSin(2.0 * M_PI * m_mod.modFreq * t + m_mod.modPhase);
            double instFreq = m_frequency + freqDev;
            double instOmega = 2.0 * M_PI * instFreq / m_sampleRate;
            double fmPhase = m_phase + instOmega * i;
            output[i] = m_amplitude * waveformFunc(fmPhase) + m_dcOffset;
            break;
        }
        case PM: {
            double phaseMod = m_mod.modDepth * qSin(2.0 * M_PI * m_mod.modFreq * t + m_mod.modPhase);
            output[i] = m_amplitude * waveformFunc(phase + phaseMod) + m_dcOffset;
            break;
        }
        case None:
        default:
            output[i] = m_amplitude * waveformFunc(phase) + m_dcOffset;
            break;
        }
    }

    m_stats.totalGenerations++;
    m_stats.lastSamples = numSamples;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalGenerations > 0)
        ? m_timeSum / m_stats.totalGenerations : 0.0;

    emit generationCompleted(numSamples);
    return output;
}

/* ---- Chirp generation ---- */

QVector<double> SignalGenerator2::generateChirp(int numSamples, double startFreq, double endFreq)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(numSamples);
    double T = static_cast<double>(numSamples) / m_sampleRate;

    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / m_sampleRate;
        double freq = startFreq + (endFreq - startFreq) * t / T;
        double phase = 2.0 * M_PI * (startFreq * t + (endFreq - startFreq) * t * t / (2.0 * T));
        output[i] = m_amplitude * qSin(m_phase + phase) + m_dcOffset;
    }

    m_stats.totalGenerations++;
    m_stats.lastSamples = numSamples;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalGenerations > 0)
        ? m_timeSum / m_stats.totalGenerations : 0.0;

    emit generationCompleted(numSamples);
    return output;
}

/* ---- Statistics ---- */

void SignalGenerator2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

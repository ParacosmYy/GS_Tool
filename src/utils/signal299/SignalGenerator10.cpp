/**
 * @file SignalGenerator10.cpp
 * @brief SignalGenerator10 实现
 *
 * 实现信号发生器：任意波形合成与AM/FM/PM调制包络整形实现测试信号产生。
 */

#include "utils/signal299/SignalGenerator10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SignalGenerator10::SignalGenerator10(QObject *parent)
    : QObject(parent)
{
    // Initialize pink noise state (Voss-McCartney uses 16 rows)
    m_pinkRows.resize(16, 0.0);
}

SignalGenerator10::~SignalGenerator10() = default;

/* ---- Configuration ---- */

void SignalGenerator10::setSampleRate(double sr) { m_sampleRate = qBound(8000.0, sr, 192000.0); }
void SignalGenerator10::setConfig(const Config& config) { m_config = config; }

void SignalGenerator10::setArbitraryWaveform(const QVector<double>& table)
{
    m_arbTable = table;
    m_config.waveform = Arbitrary;
}

/* ---- Base waveform at given phase ---- */

double SignalGenerator10::baseWaveform(double phase) const
{
    // Normalize phase to [0, 2*pi)
    double twopi = 2.0 * M_PI;
    phase = qFmod(phase, twopi);
    if (phase < 0) phase += twopi;

    switch (m_config.waveform) {
    case Sine:
        return qSin(phase);
    case Square:
        return (phase < M_PI) ? 1.0 : -1.0;
    case Triangle:
        return (phase < M_PI) ? (-1.0 + 2.0 * phase / M_PI)
                               : (3.0 - 2.0 * phase / M_PI);
    case Sawtooth:
        return 1.0 - phase / M_PI;
    case Pulse: {
        double threshold = m_config.dutyCycle * twopi;
        return (phase < threshold) ? 1.0 : -1.0;
    }
    case WhiteNoise:
        return whiteNoise();
    case PinkNoise:
        return 0.0; // Handled in generate()
    case Arbitrary: {
        if (m_arbTable.isEmpty()) return 0.0;
        double idx = phase / twopi * m_arbTable.size();
        int i0 = static_cast<int>(idx) % m_arbTable.size();
        int i1 = (i0 + 1) % m_arbTable.size();
        double frac = idx - static_cast<int>(idx);
        return m_arbTable[i0] * (1.0 - frac) + m_arbTable[i1] * frac;
    }
    }
    return 0.0;
}

/* ---- White noise ---- */

double SignalGenerator10::whiteNoise() const
{
    return 2.0 * (static_cast<double>(qrand()) / RAND_MAX) - 1.0;
}

/* ---- Pink noise (Voss-McCartney algorithm) ---- */

double SignalGenerator10::pinkNoise()
{
    m_pinkIndex = (m_pinkIndex + 1) & 0xFFFF;

    // Update rows based on trailing zeros of index
    int idx = m_pinkIndex;
    int row = 0;
    while (row < m_pinkRows.size() && (idx & 1) == 0) {
        m_pinkRunningSum -= m_pinkRows[row];
        double r = static_cast<double>(qrand()) / RAND_MAX * 2.0 - 1.0;
        m_pinkRows[row] = r;
        m_pinkRunningSum += r;
        idx >>= 1;
        row++;
    }

    // Update the top row unconditionally
    if (row < m_pinkRows.size()) {
        m_pinkRunningSum -= m_pinkRows[row];
        double r = static_cast<double>(qrand()) / RAND_MAX * 2.0 - 1.0;
        m_pinkRows[row] = r;
        m_pinkRunningSum += r;
    }

    return m_pinkRunningSum / m_pinkRows.size();
}

/* ---- Apply modulation ---- */

double SignalGenerator10::applyModulation(double sample, int sampleIndex) const
{
    double t = sampleIndex / m_sampleRate;

    switch (m_config.modulation) {
    case AM:
        return sample * (1.0 + m_config.modDepth * qSin(2.0 * M_PI * m_config.modFrequency * t));
    case FM:
        // FM is applied at generation time via phase accumulation
        return sample;
    case PM: {
        double phaseMod = m_config.modDepth * qSin(2.0 * M_PI * m_config.modFrequency * t);
        return sample * qCos(phaseMod);
    }
    case None:
    default:
        return sample;
    }
}

/* ---- Generate N samples ---- */

SignalGenerator10::GenResult SignalGenerator10::generate(int numSamples)
{
    QElapsedTimer timer;
    timer.start();

    GenResult result;
    result.numSamples = numSamples;
    result.samples.resize(numSamples);

    double phaseStep = 2.0 * M_PI * m_config.frequency / m_sampleRate;
    bool isPinkNoise = (m_config.waveform == PinkNoise);

    for (int i = 0; i < numSamples; ++i) {
        double sample;

        if (isPinkNoise) {
            sample = pinkNoise() * m_config.amplitude;
        } else {
            double phase = m_phaseAccum + m_config.phase;

            // FM modulation: modify phase accumulator
            if (m_config.modulation == FM) {
                double t = i / m_sampleRate;
                double freqDev = m_config.modDepth * m_config.frequency * 0.5;
                phase += 2.0 * M_PI * freqDev *
                         qSin(2.0 * M_PI * m_config.modFrequency * t) /
                         m_config.modFrequency;
            }

            sample = baseWaveform(phase) * m_config.amplitude;
            sample = applyModulation(sample, i);
        }

        result.samples[i] = sample + m_config.dcOffset;
    }

    // Advance phase accumulator
    m_phaseAccum += phaseStep * numSamples;
    m_phaseAccum = qFmod(m_phaseAccum, 2.0 * M_PI);

    // Compute peak and RMS
    double peak = 0.0, rmsSum = 0.0;
    for (double s : result.samples) {
        peak = qMax(peak, qAbs(s));
        rmsSum += s * s;
    }
    result.peakAmplitude = peak;
    result.rmsAmplitude = qSqrt(rmsSum / qMax(1, numSamples));

    double elapsed = timer.elapsed();
    m_stats.lastNumSamples = numSamples;
    m_stats.totalGenerations++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalGenerations;

    emit generateDone(numSamples, peak, elapsed);
    return result;
}

/* ---- Generate chirp (frequency sweep) ---- */

SignalGenerator10::GenResult SignalGenerator10::generateChirp(
    int numSamples, double startFreq, double endFreq)
{
    QElapsedTimer timer;
    timer.start();

    GenResult result;
    result.numSamples = numSamples;
    result.samples.resize(numSamples);

    double T = numSamples / m_sampleRate;
    double phase = m_config.phase;

    for (int i = 0; i < numSamples; ++i) {
        double t = i / m_sampleRate;
        double freq = startFreq + (endFreq - startFreq) * t / T;
        phase += 2.0 * M_PI * freq / m_sampleRate;

        double sample = qSin(phase) * m_config.amplitude;
        result.samples[i] = sample + m_config.dcOffset;
    }

    m_phaseAccum = phase;

    double peak = 0.0, rmsSum = 0.0;
    for (double s : result.samples) {
        peak = qMax(peak, qAbs(s));
        rmsSum += s * s;
    }
    result.peakAmplitude = peak;
    result.rmsAmplitude = qSqrt(rmsSum / qMax(1, numSamples));

    double elapsed = timer.elapsed();
    m_stats.lastNumSamples = numSamples;
    m_stats.totalGenerations++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalGenerations;

    emit generateDone(numSamples, peak, elapsed);
    return result;
}

/* ---- Reset ---- */

void SignalGenerator10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_phaseAccum = 0.0;
}

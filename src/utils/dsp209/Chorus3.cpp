/**
 * @file Chorus3.cpp
 * @brief Chorus3 实现
 *
 * 实现合唱效果器：调制多抽头延迟、立体声相位交错声像扩散、LFO控制。
 */

#include "utils/dsp209/Chorus3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Chorus3::Chorus3(QObject *parent) : QObject(parent)
{
    updateDelaySize();
}

Chorus3::~Chorus3() = default;

/* ---- Configuration ---- */

void Chorus3::setSampleRate(double rate)
{
    m_sampleRate = qMax(8000.0, rate);
    m_phaseIncrement = m_lfoRate / m_sampleRate;
    updateDelaySize();
}

void Chorus3::setBaseDelayMs(double ms) { m_baseDelayMs = qMax(0.1, ms); updateDelaySize(); }
void Chorus3::setModDepthMs(double ms) { m_depthMs = qMax(0.0, ms); updateDelaySize(); }
void Chorus3::setLfoRate(double hz) { m_lfoRate = qMax(0.01, hz); m_phaseIncrement = m_lfoRate / m_sampleRate; }
void Chorus3::setLfoWaveform(LfoWaveform wave) { m_waveform = wave; }
void Chorus3::setNumTaps(int taps) { m_numTaps = qMax(1, qMin(taps, 8)); }
void Chorus3::setFeedback(double fb) { m_feedback = qBound(0.0, fb, 0.95); }
void Chorus3::setMix(double wetDry) { m_mix = qBound(0.0, wetDry, 1.0); }

/* ---- Update delay line size ---- */

void Chorus3::updateDelaySize()
{
    int maxDelaySamples = static_cast<int>(
        (m_baseDelayMs + m_depthMs + 1.0) * m_sampleRate / 1000.0);
    m_delayLineSize = qMax(maxDelaySamples + 2, 256);

    if (m_delayLine.size() < m_delayLineSize) {
        m_delayLine.resize(m_delayLineSize, 0.0);
    }
    m_phaseIncrement = m_lfoRate / m_sampleRate;
}

/* ---- LFO value ---- */

double Chorus3::lfoValue(double phase) const
{
    double p = qFmod(phase, 1.0);
    if (p < 0.0) p += 1.0;

    switch (m_waveform) {
    case Sine:
        return qSin(2.0 * M_PI * p);
    case Triangle:
        return (p < 0.5) ? (4.0 * p - 1.0) : (3.0 - 4.0 * p);
    case Sawtooth:
        return 2.0 * p - 1.0;
    default:
        return qSin(2.0 * M_PI * p);
    }
}

/* ---- Read delay with linear interpolation ---- */

double Chorus3::readDelay(int tap, double modOffset) const
{
    // Each tap gets a different base delay offset and LFO phase shift
    double tapDelay = m_baseDelayMs + m_depthMs * modOffset;
    tapDelay += tap * 0.5; // Stagger taps by 0.5ms
    double delaySamples = tapDelay * m_sampleRate / 1000.0;

    int intDelay = static_cast<int>(delaySamples);
    double frac = delaySamples - intDelay;

    int idx0 = (m_writePos - intDelay + m_delayLineSize) % m_delayLineSize;
    int idx1 = (idx0 - 1 + m_delayLineSize) % m_delayLineSize;

    double s0 = m_delayLine[idx0];
    double s1 = m_delayLine[idx1];

    // Linear interpolation
    return s0 * (1.0 - frac) + s1 * frac;
}

/* ---- Advance LFO ---- */

void Chorus3::advanceLfo()
{
    m_lfoPhase += m_phaseIncrement;
    if (m_lfoPhase >= 1.0) m_lfoPhase -= 1.0;
}

/* ---- Process single sample ---- */

QPair<double, double> Chorus3::processSample(double sample)
{
    // Write input + feedback to delay line
    m_delayLine[m_writePos] = sample + m_feedback * m_delayLine[m_writePos];

    // Accumulate tap outputs for left and right channels
    double left = 0.0, right = 0.0;
    for (int t = 0; t < m_numTaps; ++t) {
        // Phase interleaved: each tap gets different LFO phase offset
        double tapPhase = m_lfoPhase + static_cast<double>(t) / m_numTaps;
        double mod = lfoValue(tapPhase);
        double tapOut = readDelay(t, mod);

        // Stereo spreading: alternate taps to left/right with pan
        double pan = 0.5 + 0.5 * qSin(static_cast<double>(t) * M_PI / m_numTaps);
        left += tapOut * (1.0 - pan);
        right += tapOut * pan;
    }

    double norm = 1.0 / m_numTaps;
    left *= norm;
    right *= norm;

    // Wet/dry mix
    double dry = (1.0 - m_mix) * sample;
    QPair<double, double> out = {dry + m_mix * left, dry + m_mix * right};

    // Advance write position
    m_writePos = (m_writePos + 1) % m_delayLineSize;
    advanceLfo();

    return out;
}

/* ---- Process buffer ---- */

QVector<QPair<double, double>> Chorus3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, double>> output;
    output.reserve(input.size());

    for (int i = 0; i < input.size(); ++i)
        output.append(processSample(input[i]));

    m_stats.totalSamples += input.size();
    m_stats.numTaps = m_numTaps;
    m_stats.baseDelayMs = m_baseDelayMs;
    m_stats.depthMs = m_depthMs;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, m_stats.totalSamples / 512);

    emit processingCompleted(input.size(), timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void Chorus3::reset()
{
    std::fill(m_delayLine.begin(), m_delayLine.end(), 0.0);
    m_writePos = 0;
    m_lfoPhase = 0.0;
}

void Chorus3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    reset();
}

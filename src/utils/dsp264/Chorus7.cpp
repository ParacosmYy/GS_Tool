/**
 * @file Chorus7.cpp
 * @brief Chorus7 实现
 *
 * 实现合唱效果器：多抽头调制延迟线与相位偏移LFO立体声空间扩展增厚。
 */

#include "utils/dsp264/Chorus7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Chorus7::Chorus7(QObject *parent)
    : QObject(parent)
{
    initDelayLines();
}

Chorus7::~Chorus7() = default;

/* ---- Configuration ---- */

void Chorus7::setParameters(double baseDelayMs, double depthMs, double rateHz,
                             int numTaps, double mix, double sampleRate)
{
    m_baseDelayMs = qMax(1.0, baseDelayMs);
    m_depthMs = qMax(0.0, depthMs);
    m_rateHz = qBound(0.01, rateHz, 20.0);
    m_numTaps = qBound(1, numTaps, 8);
    m_mix = qBound(0.0, mix, 1.0);
    m_sampleRate = qMax(8000.0, sampleRate);
    initDelayLines();
}

/* ---- Init delay lines ---- */

void Chorus7::initDelayLines()
{
    // Max delay in samples: (baseDelay + depth) * samplerate / 1000 + margin
    double maxDelayMs = m_baseDelayMs + m_depthMs + 1.0;
    m_maxDelaySamples = static_cast<int>(maxDelayMs * m_sampleRate / 1000.0) + 2;

    m_delayLines.resize(m_numTaps);
    m_writePos.resize(m_numTaps, 0);
    m_lfoPhase.resize(m_numTaps);

    for (int t = 0; t < m_numTaps; ++t) {
        m_delayLines[t].resize(m_maxDelaySamples, 0.0);
        // Phase offset evenly distributed for stereo spread
        m_lfoPhase[t] = 2.0 * M_PI * t / m_numTaps;
    }
    m_lfoIncrement = 2.0 * M_PI * m_rateHz / m_sampleRate;
}

/* ---- LFO value ---- */

double Chorus7::lfoValue(int tap) const
{
    // Sine LFO with phase offset per tap
    return qSin(m_lfoPhase[tap]);
}

/* ---- Read interpolated ---- */

double Chorus7::readInterpolated(int tap, double samplePos) const
{
    int pos0 = static_cast<int>(qFloor(samplePos)) % m_maxDelaySamples;
    int pos1 = (pos0 + 1) % m_maxDelaySamples;
    if (pos0 < 0) pos0 += m_maxDelaySamples;
    double frac = samplePos - qFloor(samplePos);
    return m_delayLines[tap][pos0] * (1.0 - frac) + m_delayLines[tap][pos1] * frac;
}

/* ---- Write sample ---- */

void Chorus7::writeSample(int tap, double sample)
{
    m_delayLines[tap][m_writePos[tap]] = sample;
    m_writePos[tap] = (m_writePos[tap] + 1) % m_maxDelaySamples;
}

/* ---- Process mono to stereo ---- */

QVector<double> Chorus7::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n * 2);  // Interleaved L/R

    double baseDelaySamples = m_baseDelayMs * m_sampleRate / 1000.0;
    double depthSamples = m_depthMs * m_sampleRate / 1000.0;

    for (int i = 0; i < n; ++i) {
        double dry = input[i];
        double leftSum = 0.0;
        double rightSum = 0.0;

        for (int t = 0; t < m_numTaps; ++t) {
            // Write to delay line
            writeSample(t, dry);

            // Compute modulated delay for this tap
            double mod = lfoValue(t);
            double delay = baseDelaySamples + depthSamples * mod;
            // Read position relative to write head
            double readPos = m_writePos[t] - delay;
            while (readPos < 0) readPos += m_maxDelaySamples;

            double wet = readInterpolated(t, readPos);

            // Stereo spread: alternate taps between L and R with crossfeed
            double pan = (m_numTaps > 1) ? static_cast<double>(t) / (m_numTaps - 1) : 0.5;
            // Pan law: cos/sin panning
            double panAngle = pan * M_PI / 2.0;
            leftSum += wet * qCos(panAngle);
            rightSum += wet * qSin(panAngle);

            // Advance LFO phase
            m_lfoPhase[t] += m_lfoIncrement;
            if (m_lfoPhase[t] > 2.0 * M_PI) m_lfoPhase[t] -= 2.0 * M_PI;
        }

        // Normalize by number of taps
        if (m_numTaps > 0) {
            leftSum /= m_numTaps;
            rightSum /= m_numTaps;
        }

        // Mix dry/wet
        output[i * 2] = dry * (1.0 - m_mix) + leftSum * m_mix;
        output[i * 2 + 1] = dry * (1.0 - m_mix) + rightSum * m_mix;
    }

    double elapsed = timer.elapsed();
    m_stats.numSamples = n;
    m_stats.numTaps = m_numTaps;
    m_stats.baseDelayMs = m_baseDelayMs;
    m_stats.depthMs = m_depthMs;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingComplete(n, elapsed);
    return output;
}

/* ---- Process mono to mono ---- */

QVector<double> Chorus7::processMono(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n);

    double baseDelaySamples = m_baseDelayMs * m_sampleRate / 1000.0;
    double depthSamples = m_depthMs * m_sampleRate / 1000.0;

    for (int i = 0; i < n; ++i) {
        double dry = input[i];
        double wetSum = 0.0;

        for (int t = 0; t < m_numTaps; ++t) {
            writeSample(t, dry);

            double mod = lfoValue(t);
            double delay = baseDelaySamples + depthSamples * mod;
            double readPos = m_writePos[t] - delay;
            while (readPos < 0) readPos += m_maxDelaySamples;

            wetSum += readInterpolated(t, readPos);

            m_lfoPhase[t] += m_lfoIncrement;
            if (m_lfoPhase[t] > 2.0 * M_PI) m_lfoPhase[t] -= 2.0 * M_PI;
        }

        double wet = (m_numTaps > 0) ? wetSum / m_numTaps : 0.0;
        output[i] = dry * (1.0 - m_mix) + wet * m_mix;
    }

    double elapsed = timer.elapsed();
    m_stats.numSamples = n;
    m_stats.numTaps = m_numTaps;
    m_stats.baseDelayMs = m_baseDelayMs;
    m_stats.depthMs = m_depthMs;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingComplete(n, elapsed);
    return output;
}

/* ---- Reset ---- */

void Chorus7::reset()
{
    for (int t = 0; t < m_numTaps; ++t) {
        m_delayLines[t].fill(0.0);
        m_writePos[t] = 0;
        m_lfoPhase[t] = 2.0 * M_PI * t / m_numTaps;
    }
}

void Chorus7::resetStatistics()
{
    reset();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file Chorus5.cpp
 * @brief Chorus5 实现
 *
 * 实现合唱效果器：多声部调制延迟线与立体声相位偏移LFO调制。
 */

#include "utils/dsp236/Chorus5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Chorus5::Chorus5(QObject *parent) : QObject(parent) { initBuffers(); }
Chorus5::~Chorus5() = default;

/* ---- Configuration ---- */

void Chorus5::setSampleRate(int rate) { m_sampleRate = qMax(8000, rate); initBuffers(); }
void Chorus5::setBaseDelay(double ms) { m_baseDelay = qMax(1.0, ms); initBuffers(); }
void Chorus5::setDepth(double ms) { m_depth = qMax(0.0, ms); initBuffers(); }
void Chorus5::setRate(double hz) { m_rate = qBound(0.01, hz, 20.0); }
void Chorus5::setStereoSpread(double spread) { m_spread = qBound(0.0, spread, 1.0); }
void Chorus5::setFeedback(double fb) { m_feedback = qBound(0.0, fb, 0.95); }
void Chorus5::setNumVoices(int voices) { m_numVoices = qBound(1, voices, 8); initBuffers(); }

/* ---- Delay buffer size ---- */

int Chorus5::delayBufferSize() const
{
    // Max delay in samples: (baseDelay + depth) * sampleRate / 1000 + margin
    int maxDelay = static_cast<int>((m_baseDelay + m_depth) * m_sampleRate / 1000.0) + 2;
    return qMax(64, maxDelay * 2);  // double for safety
}

/* ---- Initialize buffers ---- */

void Chorus5::initBuffers()
{
    int bufSize = delayBufferSize();
    m_delayBuffer.resize(m_numVoices);
    m_writePos.resize(m_numVoices, 0);
    m_lfoPhase.resize(m_numVoices, 0.0);

    for (int v = 0; v < m_numVoices; ++v) {
        m_delayBuffer[v].resize(bufSize, 0.0);
        m_writePos[v] = 0;
        m_lfoPhase[v] = v * 2.0 * M_PI / m_numVoices;  // phase offset per voice
    }
    m_stats.numVoices = m_numVoices;
    m_stats.sampleRate = m_sampleRate;
}

/* ---- Compute LFO value (sine) ---- */

double Chorus5::lfoValue(int voiceIdx) const
{
    return qSin(m_lfoPhase[voiceIdx]);
}

/* ---- Read from delay buffer with linear interpolation ---- */

double Chorus5::readDelay(int voiceIdx, double delaySamples) const
{
    int bufSize = m_delayBuffer[voiceIdx].size();
    int writePos = m_writePos[voiceIdx];

    double readPos = writePos - delaySamples;
    while (readPos < 0) readPos += bufSize;
    while (readPos >= bufSize) readPos -= bufSize;

    int pos0 = static_cast<int>(readPos);
    int pos1 = (pos0 + 1) % bufSize;
    double frac = readPos - pos0;

    return m_delayBuffer[voiceIdx][pos0] * (1.0 - frac) + m_delayBuffer[voiceIdx][pos1] * frac;
}

/* ---- Advance LFO phase ---- */

void Chorus5::advanceLFO()
{
    double phaseInc = 2.0 * M_PI * m_rate / m_sampleRate;
    for (int v = 0; v < m_numVoices; ++v) {
        m_lfoPhase[v] += phaseInc;
        if (m_lfoPhase[v] >= 2.0 * M_PI)
            m_lfoPhase[v] -= 2.0 * M_PI;
    }
}

/* ---- Process single sample ---- */

QPair<double, double> Chorus5::processSample(double sample)
{
    double leftSum = 0.0;
    double rightSum = 0.0;

    double baseDelaySamples = m_baseDelay * m_sampleRate / 1000.0;
    double depthSamples = m_depth * m_sampleRate / 1000.0;

    for (int v = 0; v < m_numVoices; ++v) {
        // Compute modulated delay
        double lfo = lfoValue(v);
        double delaySamp = baseDelaySamples + depthSamples * lfo;

        // Read from delay buffer
        double delayed = readDelay(v, delaySamp);

        // Write input + feedback to delay buffer
        int bufSize = m_delayBuffer[v].size();
        m_delayBuffer[v][m_writePos[v]] = sample + m_feedback * delayed;
        m_writePos[v] = (m_writePos[v] + 1) % bufSize;

        // Stereo panning per voice
        // Spread voices across stereo field
        double panAngle = (m_spread > 0.0)
            ? v * M_PI * m_spread / m_numVoices - M_PI * m_spread / 2.0
            : 0.0;
        double panLeft = qCos(panAngle);
        double panRight = qSin(panAngle);

        double mix = 1.0 / m_numVoices;
        leftSum += delayed * panLeft * mix;
        rightSum += delayed * panRight * mix;
    }

    // Dry + wet
    double wetLevel = 0.7;
    double dryLevel = 1.0 - wetLevel * 0.3;
    double left = sample * dryLevel + leftSum * wetLevel;
    double right = sample * dryLevel + rightSum * wetLevel;

    advanceLFO();
    return {left, right};
}

/* ---- Process buffer ---- */

QVector<QVector<double>> Chorus5::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> left(n), right(n);

    for (int i = 0; i < n; ++i) {
        auto [l, r] = processSample(input[i]);
        left[i] = l;
        right[i] = r;
    }

    m_stats.totalSamples += n;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processingCompleted(n, timer.elapsed());
    return {left, right};
}

/* ---- Reset ---- */

void Chorus5::resetStatistics()
{
    initBuffers();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

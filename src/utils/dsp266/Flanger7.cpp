/**
 * @file Flanger7.cpp
 * @brief Flanger7 实现
 *
 * 实现镶边效果器：立体声展宽调制梳状滤波器与反馈阻尼金属扫频效果。
 */

#include "utils/dsp266/Flanger7.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

Flanger7::Flanger7(QObject *parent)
    : QObject(parent)
{
    setParameters(44100.0, 0.5, 5.0, 1.0, 0.5, 0.7, 0.6);
}

Flanger7::~Flanger7() = default;

/* ---- Configuration ---- */

void Flanger7::setParameters(double sampleRate, double minDelayMs, double maxDelayMs,
                               double depth, double rateHz, double feedback,
                               double stereoWidth)
{
    m_sampleRate = qMax(8000.0, sampleRate);
    m_minDelayMs = qMax(0.1, minDelayMs);
    m_maxDelayMs = qMax(m_minDelayMs + 0.1, maxDelayMs);
    m_depth = qBound(0.0, depth, 1.0);
    m_rateHz = qBound(0.01, rateHz, 20.0);
    m_feedback = qBound(0.0, feedback, 0.95);
    m_stereoWidth = qBound(0.0, stereoWidth, 1.0);

    // Allocate delay buffer: max delay in samples + safety margin
    int maxSamples = static_cast<int>(m_maxDelayMs * m_sampleRate / 1000.0) + 2;
    m_bufferSize = qMax(64, maxSamples);
    m_delayBufferL.resize(m_bufferSize, 0.0);
    m_delayBufferR.resize(m_bufferSize, 0.0);
    m_writePos = 0;
    m_phase = 0.0;
}

/* ---- LFO ---- */

double Flanger7::lfoValue()
{
    // Sinusoidal LFO for smooth sweep
    double val = qSin(2.0 * M_PI * m_phase);
    m_phase += m_rateHz / m_sampleRate;
    if (m_phase >= 1.0) m_phase -= 1.0;
    return val;
}

/* ---- Fractional delay read ---- */

double Flanger7::readBuffer(const QVector<double>& buffer,
                              double delaySamples) const
{
    // Linear interpolation for fractional delay
    double exactPos = static_cast<double>(m_writePos) - delaySamples;
    while (exactPos < 0.0) exactPos += m_bufferSize;
    while (exactPos >= m_bufferSize) exactPos -= m_bufferSize;

    int idx0 = static_cast<int>(exactPos);
    int idx1 = (idx0 + 1) % m_bufferSize;
    double frac = exactPos - idx0;

    return buffer[idx0] * (1.0 - frac) + buffer[idx1] * frac;
}

/* ---- Process single sample ---- */

void Flanger7::processSample(double input, double& outLeft, double& outRight)
{
    // Compute modulated delay
    double lfo = lfoValue();
    double delayRange = (m_maxDelayMs - m_minDelayMs) / 2.0;
    double centerDelay = (m_maxDelayMs + m_minDelayMs) / 2.0;

    // Left channel: normal LFO
    double delayL = (centerDelay + lfo * delayRange * m_depth) * m_sampleRate / 1000.0;

    // Right channel: inverted LFO for stereo widening
    double delayR = (centerDelay - lfo * delayRange * m_depth * m_stereoWidth)
                    * m_sampleRate / 1000.0;

    delayL = qBound(1.0, delayL, m_bufferSize - 1.0);
    delayR = qBound(1.0, delayR, m_bufferSize - 1.0);

    // Read delayed samples with feedback
    double delayedL = readBuffer(m_delayBufferL, delayL);
    double delayedR = readBuffer(m_delayBufferR, delayR);

    // Apply feedback damping
    double fbL = delayedL * m_feedback * m_damping;
    double fbR = delayedR * m_feedback * m_damping;

    // Write to buffer: input + feedback
    m_delayBufferL[m_writePos] = input + fbL;
    m_delayBufferR[m_writePos] = input + fbR;

    // Output: dry + wet (modulated comb)
    outLeft = input + delayedL * m_depth;
    outRight = input + delayedR * m_depth;

    m_writePos = (m_writePos + 1) % m_bufferSize;
}

/* ---- Process buffer ---- */

QVector<QVector<double>> Flanger7::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> outL(input.size());
    QVector<double> outR(input.size());

    for (int i = 0; i < input.size(); ++i)
        processSample(input[i], outL[i], outR[i]);

    double elapsed = timer.elapsed();
    m_stats.numSamplesProcessed += input.size();
    m_stats.bufferSize = m_bufferSize;
    m_stats.sampleRate = m_sampleRate;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingDone(input.size(), elapsed);

    return {outL, outR};
}

/* ---- Reset ---- */

void Flanger7::reset()
{
    m_delayBufferL.fill(0.0);
    m_delayBufferR.fill(0.0);
    m_writePos = 0;
    m_phase = 0.0;
}

void Flanger7::resetStatistics()
{
    reset();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

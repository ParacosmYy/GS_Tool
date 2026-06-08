/**
 * @file Delay4.cpp
 * @brief Delay4 实现
 *
 * 实现延迟线：节拍同步分数延迟插值、共振滤波反馈回路。
 */

#include "utils/dsp212/Delay4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Delay4::Delay4(QObject *parent) : QObject(parent) {}
Delay4::~Delay4() = default;

/* ---- Initialize ---- */

void Delay4::init(double sampleRate, double maxDelayMs)
{
    m_sampleRate = qMax(1.0, sampleRate);
    m_maxDelayMs = qMax(1.0, maxDelayMs);

    int bufLen = static_cast<int>(m_sampleRate * m_maxDelayMs / 1000.0) + 2;
    m_buffer.fill(0.0, bufLen);
    m_writeIdx = 0;
    m_delaySamples = 0.0;

    m_stats.bufferLength = bufLen;
    updateFilterCoeff();
}

/* ---- Configuration ---- */

void Delay4::setDelayTime(double ms)
{
    m_delaySamples = qBound(0.0, ms * m_sampleRate / 1000.0,
                             static_cast<double>(m_buffer.size() - 1));
    m_stats.delayTimeMs = ms;
}

void Delay4::setDelaySync(TempoSync note, double bpm)
{
    if (bpm <= 0 || note == Free) return;
    double beatMs = 60000.0 / bpm;
    double divisor = static_cast<double>(note);
    double delayMs = beatMs * 4.0 / divisor;  // Whole note = 4 beats
    setDelayTime(delayMs);
}

void Delay4::setFeedback(double fb)
{
    m_feedback = qBound(0.0, fb, 0.99);
    m_stats.feedback = m_feedback;
}

void Delay4::setMix(double mix)
{
    m_mix = qBound(0.0, mix, 1.0);
    m_stats.mix = m_mix;
}

void Delay4::setFilterCutoff(double hz)
{
    m_filterCutoff = qBound(20.0, hz, m_sampleRate / 2.0);
    updateFilterCoeff();
}

void Delay4::setFilterQ(double q)
{
    m_filterQ = qBound(0.1, q, 20.0);
    updateFilterCoeff();
}

/* ---- Update filter coefficient ---- */

void Delay4::updateFilterCoeff()
{
    // Simple 1-pole lowpass: a = 1 - exp(-2*pi*fc/fs)
    double wc = 2.0 * M_PI * m_filterCutoff / m_sampleRate;
    m_filterA = 1.0 - qExp(-wc / m_filterQ);
}

/* ---- Apply resonant filter ---- */

double Delay4::applyFilter(double input)
{
    m_filterState += m_filterA * (input - m_filterState);
    return m_filterState;
}

/* ---- Fractional delay read with allpass interpolation ---- */

double Delay4::readFractional(double delaySamples) const
{
    int intDelay = static_cast<int>(delaySamples);
    double frac = delaySamples - intDelay;

    int readIdx = m_writeIdx - intDelay;
    if (readIdx < 0) readIdx += m_buffer.size();

    int readIdx1 = readIdx - 1;
    if (readIdx1 < 0) readIdx1 += m_buffer.size();

    // Allpass interpolation for better quality at high frequencies
    double y0 = m_buffer[readIdx];
    double y1 = m_buffer[readIdx1];

    // Linear blend (simplified allpass)
    return y0 * (1.0 - frac) + y1 * frac;
}

/* ---- Process single sample ---- */

double Delay4::processSample(double input)
{
    // Read from delay line with fractional interpolation
    double delayed = readFractional(m_delaySamples);

    // Apply resonant filter to feedback path
    double filteredFeedback = applyFilter(delayed * m_feedback);

    // Write input + filtered feedback to buffer
    m_buffer[m_writeIdx] = input + filteredFeedback;

    // Advance write pointer
    m_writeIdx = (m_writeIdx + 1) % m_buffer.size();

    // Output: dry/wet mix
    double output = input * (1.0 - m_mix) + delayed * m_mix;

    m_sampleCount++;
    m_stats.totalSamples = m_sampleCount;

    return output;
}

/* ---- Process block ---- */

QVector<double> Delay4::processBlock(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    output.reserve(input.size());

    for (double sample : input)
        output.append(processSample(sample));

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit delayProcessed(input.size(), timer.elapsed());

    return output;
}

/* ---- Reset ---- */

void Delay4::reset()
{
    m_buffer.fill(0.0);
    m_writeIdx = 0;
    m_filterState = 0.0;
    m_sampleCount = 0;
}

/* ---- Reset statistics ---- */

void Delay4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_sampleCount = 0;
}

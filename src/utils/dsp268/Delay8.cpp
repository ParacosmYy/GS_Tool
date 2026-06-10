/**
 * @file Delay8.cpp
 * @brief Delay8 实现
 *
 * 实现延迟效果：节拍同步调制与磁带回声Wow/Flutter模拟。
 */

#include "utils/dsp268/Delay8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Delay8::Delay8(QObject *parent)
    : QObject(parent)
{
    updateBuffer();
}

Delay8::~Delay8() = default;

/* ---- Configuration ---- */

void Delay8::setSampleRate(int rate)
{
    m_sampleRate = qMax(8000, rate);
    updateBuffer();
}

void Delay8::setDelayTime(double ms)
{
    m_delayMs = qMax(1.0, ms);
    updateBuffer();
}

void Delay8::setTempoSync(SyncNote note, double bpm)
{
    m_delayMs = syncToMs(note, bpm);
    updateBuffer();
}

void Delay8::setFeedback(double fb)
{
    m_feedback = qBound(0.0, fb, 0.95);
}

void Delay8::setMix(double mix)
{
    m_mix = qBound(0.0, mix, 1.0);
}

void Delay8::setWowDepth(double depth)
{
    m_wowDepth = qBound(0.0, depth, 1.0);
}

void Delay8::setFlutterRate(double rate)
{
    m_flutterRate = qBound(0.1, rate, 20.0);
    m_lfoIncrement = 2.0 * M_PI * m_flutterRate / m_sampleRate;
}

/* ---- Tempo sync conversion ---- */

double Delay8::syncToMs(SyncNote note, double bpm)
{
    double quarterMs = 60000.0 / qMax(1.0, bpm);
    switch (note) {
    case WholeNote:      return quarterMs * 4.0;
    case HalfNote:       return quarterMs * 2.0;
    case QuarterNote:    return quarterMs;
    case EighthNote:     return quarterMs * 0.5;
    case SixteenthNote:  return quarterMs * 0.25;
    case DottedQuarter:  return quarterMs * 1.5;
    case TripletEighth:  return quarterMs / 3.0;
    default:             return quarterMs;
    }
}

/* ---- Update buffer ---- */

void Delay8::updateBuffer()
{
    m_delaySamples = static_cast<int>(m_delayMs * m_sampleRate / 1000.0);
    int newSize = qMax(1, m_delaySamples + 256); // extra margin for wow modulation

    if (m_buffer.size() < newSize) {
        int oldSize = m_buffer.size();
        m_buffer.resize(newSize);
        for (int i = oldSize; i < newSize; ++i)
            m_buffer[i] = 0.0;
    }
    m_lfoIncrement = 2.0 * M_PI * m_flutterRate / m_sampleRate;
}

/* ---- Modulated delay offset ---- */

double Delay8::modulatedDelay() const
{
    // Wow: slow sinusoidal modulation of delay time
    double wowOffset = m_wowDepth * qSin(m_lfoPhase) * 0.02 * m_delaySamples;
    return m_delaySamples + wowOffset;
}

/* ---- Process block ---- */

QVector<double> Delay8::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n, 0.0);
    int bufSize = m_buffer.size();

    for (int i = 0; i < n; ++i) {
        // Read from modulated delay position (fractional delay via linear interp)
        double delayPos = modulatedDelay();
        int readPosInt = m_writePos - static_cast<int>(delayPos);
        double frac = delayPos - static_cast<int>(delayPos);

        // Wrap read position
        while (readPosInt < 0) readPosInt += bufSize;
        int readNext = (readPosInt + 1) % bufSize;

        // Linear interpolation for fractional delay
        double delayed = m_buffer[readPosInt] * (1.0 - frac)
                       + m_buffer[readNext] * frac;

        // Write input + feedback into buffer
        m_buffer[m_writePos] = input[i] + m_feedback * delayed;

        // Output: dry/wet mix
        output[i] = input[i] * (1.0 - m_mix) + delayed * m_mix;

        // Advance write position and LFO
        m_writePos = (m_writePos + 1) % bufSize;
        m_lfoPhase += m_lfoIncrement;
        if (m_lfoPhase > 2.0 * M_PI) m_lfoPhase -= 2.0 * M_PI;
    }

    double elapsed = timer.elapsed();
    m_stats.bufferSize = bufSize;
    m_stats.sampleRate = m_sampleRate;
    m_stats.delayTimeMs = m_delayMs;
    m_stats.feedback = m_feedback;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingCompleted(n, m_delayMs, elapsed);

    return output;
}

/* ---- Reset ---- */

void Delay8::reset()
{
    m_buffer.fill(0.0);
    m_writePos = 0;
    m_lfoPhase = 0.0;
}

/* ---- Reset statistics ---- */

void Delay8::resetStatistics()
{
    reset();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file Flanger2.cpp
 * @brief Flanger2 实现
 *
 * 实现镶边效果器：调制梳状滤波器、立体声相位偏移、反馈共振控制。
 */

#include "utils/dsp192/Flanger2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Flanger2::Flanger2(QObject *parent) : QObject(parent)
{
    int bufSize = static_cast<int>(44100 * 0.010); // 10ms max delay
    m_delayL.resize(bufSize, 0.0);
    m_delayR.resize(bufSize, 0.0);
}

Flanger2::~Flanger2() = default;

/* ---- Configuration ---- */

void Flanger2::setSampleRate(int sr)
{
    m_sampleRate = qMax(8000, sr);
    int bufSize = static_cast<int>(m_sampleRate * 0.010);
    m_delayL.resize(bufSize, 0.0);
    m_delayR.resize(bufSize, 0.0);
    m_writePos = 0;
}

void Flanger2::setRate(double hz) { m_rate = qMax(0.01, qMin(hz, 20.0)); }
void Flanger2::setDepth(double ms) { m_depth = qMax(0.1, qMin(ms, 10.0)); }
void Flanger2::setFeedback(double fb) { m_feedback = qMax(0.0, qMin(fb, 0.99)); }
void Flanger2::setStereoPhase(double deg) { m_stereoPhase = qMax(0.0, qMin(deg, 360.0)); }
void Flanger2::setLFOType(LFOType type) { m_lfoType = type; }

/* ---- LFO value at given phase ---- */

double Flanger2::lfoValue(double phase) const
{
    double p = qFmod(phase, 2.0 * M_PI);
    if (p < 0) p += 2.0 * M_PI;

    switch (m_lfoType) {
    case Sine:
        return qSin(p);
    case Triangle:
        return (p < M_PI) ? (-1.0 + 2.0 * p / M_PI) : (3.0 - 2.0 * p / M_PI);
    case Sawtooth:
        return 1.0 - p / M_PI;
    case Square:
        return (p < M_PI) ? 1.0 : -1.0;
    }
    return qSin(p);
}

/* ---- Read from delay buffer with linear interpolation ---- */

double Flanger2::readBuffer(const QVector<double>& buf, double index) const
{
    int n = buf.size();
    if (n == 0) return 0.0;

    int idx0 = static_cast<int>(qFloor(index)) % n;
    int idx1 = (idx0 + 1) % n;
    if (idx0 < 0) idx0 += n;

    double frac = index - qFloor(index);
    return buf[idx0] * (1.0 - frac) + buf[idx1] * frac;
}

/* ---- Advance LFO phase ---- */

void Flanger2::advancePhase()
{
    double phaseInc = 2.0 * M_PI * m_rate / m_sampleRate;
    m_phase += phaseInc;
    if (m_phase >= 2.0 * M_PI) m_phase -= 2.0 * M_PI;
}

/* ---- Process mono ---- */

QVector<double> Flanger2::processMono(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    QVector<double> output(N);
    int bufSize = m_delayL.size();
    double maxDelaySamples = m_sampleRate * m_depth / 1000.0;

    for (int i = 0; i < N; ++i) {
        // LFO modulates delay time
        double mod = lfoValue(m_phase);
        double delaySamples = maxDelaySamples * (1.0 + mod) * 0.5;

        // Read delayed signal with feedback
        double readPos = static_cast<double>(m_writePos) - delaySamples;
        while (readPos < 0) readPos += bufSize;

        double delayed = readBuffer(m_delayL, readPos);

        // Write input + feedback
        m_delayL[m_writePos] = input[i] + m_feedback * delayed;
        m_writePos = (m_writePos + 1) % bufSize;

        // Output: dry + wet
        output[i] = input[i] + delayed;
        advancePhase();
    }

    m_stats.totalProcessed += N;
    m_stats.sampleRate = m_sampleRate;
    m_stats.bufferSize = bufSize;
    m_stats.depth = m_depth;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalProcessed > 0)
        ? m_timeSum / m_stats.totalProcessed * 1000 : 0.0;

    emit processingCompleted(N, timer.elapsed());
    return output;
}

/* ---- Process stereo (interleaved L,R) ---- */

QVector<double> Flanger2::processStereo(const QVector<double>& interleaved)
{
    QElapsedTimer timer;
    timer.start();

    int N = interleaved.size() / 2;
    QVector<double> output(N * 2);
    int bufSize = m_delayL.size();
    double maxDelaySamples = m_sampleRate * m_depth / 1000.0;
    double stereoOffset = m_stereoPhase * M_PI / 180.0;

    for (int i = 0; i < N; ++i) {
        double inL = interleaved[i * 2];
        double inR = interleaved[i * 2 + 1];

        // Left channel LFO
        double modL = lfoValue(m_phase);
        double delayL = maxDelaySamples * (1.0 + modL) * 0.5;

        // Right channel with phase offset
        double modR = lfoValue(m_phase + stereoOffset);
        double delayR = maxDelaySamples * (1.0 + modR) * 0.5;

        // Read/write left
        double rPosL = static_cast<double>(m_writePos) - delayL;
        while (rPosL < 0) rPosL += bufSize;
        double delL = readBuffer(m_delayL, rPosL);
        m_delayL[m_writePos] = inL + m_feedback * delL;

        // Read/write right
        double rPosR = static_cast<double>(m_writePos) - delayR;
        while (rPosR < 0) rPosR += bufSize;
        double delR = readBuffer(m_delayR, rPosR);
        m_delayR[m_writePos] = inR + m_feedback * delR;

        m_writePos = (m_writePos + 1) % bufSize;

        output[i * 2] = inL + delL;
        output[i * 2 + 1] = inR + delR;
        advancePhase();
    }

    m_stats.totalProcessed += N * 2;
    m_timeSum += timer.elapsed();
    emit processingCompleted(N * 2, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void Flanger2::reset()
{
    m_delayL.fill(0.0);
    m_delayR.fill(0.0);
    m_writePos = 0;
    m_phase = 0.0;
}

void Flanger2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    reset();
}

/**
 * @file Flanger6.cpp
 * @brief Flanger6 实现
 *
 * 实现镶边效果器：双调制梳状滤波器、立体声自动声像与反馈共振。
 */

#include "utils/dsp252/Flanger6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Flanger6::Flanger6(QObject *parent) : QObject(parent) {}
Flanger6::~Flanger6() = default;

/* ---- Configuration ---- */

void Flanger6::prepare(int sampleRate, int maxDelaySamples)
{
    m_sampleRate = qMax(8000, sampleRate);
    m_maxDelay = qMax(64, maxDelaySamples);

    m_delayBuf1.resize(m_maxDelay, 0.0);
    m_delayBuf2.resize(m_maxDelay, 0.0);
    m_writePos = 0;

    m_lfoPhase1 = 0.0;
    m_lfoPhase2 = M_PI / 3.0; // Phase offset for second comb
    m_panPhase = 0.0;

    m_stats.sampleRate = m_sampleRate;
}

void Flanger6::setParams(double lfoRate, double depth, double feedback)
{
    m_lfoRate = qBound(0.01, lfoRate, 20.0);
    m_depth = qBound(0.0, depth, 1.0);
    m_feedback = qBound(0.0, feedback, 0.95);
}

void Flanger6::setStereoPan(double panRate, double panWidth)
{
    m_panRate = qBound(0.01, panRate, 10.0);
    m_panWidth = qBound(0.0, panWidth, 1.0);
}

/* ---- LFO value (sinusoidal) ---- */

double Flanger6::lfoValue(double phase) const
{
    return 0.5 * (1.0 + qSin(phase)); // Normalized to 0..1
}

/* ---- Fractional delay read with linear interpolation ---- */

double Flanger6::readFractional(const QVector<double>& buf,
                                  double delaySamples) const
{
    int delayInt = qFloor(delaySamples);
    double frac = delaySamples - delayInt;

    int idx0 = (m_writePos - delayInt + m_maxDelay) % m_maxDelay;
    int idx1 = (idx0 - 1 + m_maxDelay) % m_maxDelay;

    return buf[idx0] * (1.0 - frac) + buf[idx1] * frac;
}

/* ---- Advance LFO phase ---- */

void Flanger6::advancePhase(double& phase, double rate)
{
    phase += 2.0 * M_PI * rate / m_sampleRate;
    if (phase >= 2.0 * M_PI) phase -= 2.0 * M_PI;
}

/* ---- Process stereo interleaved samples ---- */

void Flanger6::process(QVector<double>& samples, int numFrames)
{
    QElapsedTimer timer;
    timer.start();

    // Base delay in samples (e.g., 1-5ms range)
    double baseDelay = 0.001 * m_sampleRate; // 1ms
    double sweepRange = 0.004 * m_sampleRate * m_depth; // 0-4ms sweep

    for (int i = 0; i < numFrames; ++i) {
        int idx = i * 2;
        if (idx + 1 >= samples.size()) break;

        double inL = samples[idx];
        double inR = samples[idx + 1];
        double inMono = 0.5 * (inL + inR);

        // Comb filter 1 delay
        double delay1 = baseDelay + sweepRange * lfoValue(m_lfoPhase1);
        // Comb filter 2 delay (phase-offset)
        double delay2 = baseDelay + sweepRange * lfoValue(m_lfoPhase2);

        // Read from delay buffers
        double comb1 = readFractional(m_delayBuf1, delay1);
        double comb2 = readFractional(m_delayBuf2, delay2);

        // Write input + feedback to delay buffers
        m_delayBuf1[m_writePos] = inMono + m_feedback * comb1;
        m_delayBuf2[m_writePos] = inMono + m_feedback * comb2;

        // Stereo auto-pan
        double panL = 0.5 + 0.5 * m_panWidth * qSin(m_panPhase);
        double panR = 1.0 - panL;

        // Mix original with comb outputs
        double outL = inL + comb1 * panL + comb2 * panR;
        double outR = inR + comb1 * panR + comb2 * panL;

        samples[idx] = outL;
        samples[idx + 1] = outR;

        // Advance write position and LFO phases
        m_writePos = (m_writePos + 1) % m_maxDelay;
        advancePhase(m_lfoPhase1, m_lfoRate);
        advancePhase(m_lfoPhase2, m_lfoRate);
        advancePhase(m_panPhase, m_panRate);
    }

    m_stats.numFrames += numFrames;
    m_stats.numChannels = 2;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processCompleted(numFrames, timer.elapsed());
}

/* ---- Process mono input to stereo output ---- */

QVector<double> Flanger6::processMono(const QVector<double>& input)
{
    int n = input.size();
    // Convert to stereo interleaved, process, then return
    QVector<double> stereo(n * 2);
    for (int i = 0; i < n; ++i) {
        stereo[i * 2] = input[i];
        stereo[i * 2 + 1] = input[i];
    }

    process(stereo, n);
    return stereo;
}

/* ---- Reset ---- */

void Flanger6::resetStatistics()
{
    m_delayBuf1.fill(0.0);
    m_delayBuf2.fill(0.0);
    m_writePos = 0;
    m_lfoPhase1 = 0.0;
    m_lfoPhase2 = M_PI / 3.0;
    m_panPhase = 0.0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

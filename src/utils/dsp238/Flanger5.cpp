/**
 * @file Flanger5.cpp
 * @brief Flanger5 实现
 *
 * 实现镶边效果器：扫掠梳状滤波、再生反馈与可变深度立体声调制。
 */

#include "utils/dsp238/Flanger5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Flanger5::Flanger5(QObject *parent) : QObject(parent) {}
Flanger5::~Flanger5() = default;

/* ---- Initialize ---- */

bool Flanger5::init(int sampleRate)
{
    if (sampleRate <= 0) return false;
    m_sampleRate = sampleRate;
    m_stats.sampleRate = sampleRate;

    // Allocate delay lines: max delay = base + range + margin
    double maxDelaySec = m_params.delayBase + m_params.delayRange + 0.002;
    m_delayLineSize = static_cast<int>(maxDelaySec * sampleRate) + 2;
    m_delayLeft.resize(m_delayLineSize, 0.0);
    m_delayRight.resize(m_delayLineSize, 0.0);
    m_writePos = 0;
    m_lfoPhase = 0.0;

    // Compute LFO phase increment per sample
    m_lfoPhaseInc = m_params.rate * 2.0 * M_PI / sampleRate;
    return true;
}

/* ---- Set parameters ---- */

void Flanger5::setParameters(const Parameters& params)
{
    m_params = params;
    m_params.depth = qBound(0.0, params.depth, 1.0);
    m_params.feedback = qBound(0.0, params.feedback, 0.95);
    m_params.mix = qBound(0.0, params.mix, 1.0);
    m_lfoPhaseInc = m_params.rate * 2.0 * M_PI / m_sampleRate;

    // Resize delay lines if needed
    double maxDelaySec = m_params.delayBase + m_params.delayRange + 0.002;
    int needed = static_cast<int>(maxDelaySec * m_sampleRate) + 2;
    if (needed > m_delayLineSize) {
        m_delayLineSize = needed;
        m_delayLeft.resize(m_delayLineSize, 0.0);
        m_delayRight.resize(m_delayLineSize, 0.0);
    }
}

/* ---- LFO value ---- */

double Flanger5::lfoValue(double phase) const
{
    double p = qFmod(phase, 2.0 * M_PI);
    if (p < 0) p += 2.0 * M_PI;
    switch (m_params.waveform) {
    case Sine:     return qSin(p);
    case Triangle: return (p < M_PI) ? (-1.0 + 2.0 * p / M_PI) : (3.0 - 2.0 * p / M_PI);
    case Sawtooth: return 1.0 - p / M_PI;
    default:       return qSin(p);
    }
}

/* ---- Fractional read ---- */

double Flanger5::readFractional(const QVector<double>& buf, double index) const
{
    int idx0 = static_cast<int>(index) % m_delayLineSize;
    if (idx0 < 0) idx0 += m_delayLineSize;
    int idx1 = (idx0 + 1) % m_delayLineSize;
    double frac = index - qFloor(index);
    return buf[idx0] * (1.0 - frac) + buf[idx1] * frac;
}

/* ---- Process stereo sample ---- */

void Flanger5::processSample(double& left, double& right)
{
    // Compute LFO modulated delay for left channel
    double lfoL = lfoValue(m_lfoPhase);
    double delaySamplesL = (m_params.delayBase + m_params.delayRange * lfoL * m_params.depth) * m_sampleRate;

    // Write to delay buffer
    m_delayLeft[m_writePos] = left + m_params.feedback * m_delayLeft[m_writePos];
    if (m_params.stereo)
        m_delayRight[m_writePos] = right + m_params.feedback * m_delayRight[m_writePos];

    // Compute delayed read position
    double readPosL = m_writePos - delaySamplesL;
    if (readPosL < 0) readPosL += m_delayLineSize;

    // Read delayed signal with interpolation
    double delayedL = readFractional(m_delayLeft, readPosL);

    // Mix dry and wet for left
    double wetL = delayedL * m_params.mix;
    left = left * (1.0 - m_params.mix) + wetL;

    // Right channel with phase offset
    if (m_params.stereo) {
        double phaseOffset = m_params.stereoPhase * M_PI / 180.0;
        double lfoR = lfoValue(m_lfoPhase + phaseOffset);
        double delaySamplesR = (m_params.delayBase + m_params.delayRange * lfoR * m_params.depth) * m_sampleRate;
        double readPosR = m_writePos - delaySamplesR;
        if (readPosR < 0) readPosR += m_delayLineSize;
        double delayedR = readFractional(m_delayRight, readPosR);
        double wetR = delayedR * m_params.mix;
        right = right * (1.0 - m_params.mix) + wetR;
    }

    // Advance write position and LFO phase
    m_writePos = (m_writePos + 1) % m_delayLineSize;
    m_lfoPhase += m_lfoPhaseInc;
}

/* ---- Process mono ---- */

void Flanger5::processMono(QVector<double>& buffer)
{
    QElapsedTimer timer;
    timer.start();

    double dummy = 0.0;
    for (int i = 0; i < buffer.size(); ++i) {
        dummy = 0.0;
        processSample(buffer[i], dummy);
    }

    m_stats.numSamplesProcessed += buffer.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processingCompleted(buffer.size(), timer.elapsed());
}

/* ---- Process stereo ---- */

void Flanger5::processStereo(QVector<double>& buffer)
{
    QElapsedTimer timer;
    timer.start();

    // Interleaved stereo: [L0,R0,L1,R1,...]
    for (int i = 0; i + 1 < buffer.size(); i += 2)
        processSample(buffer[i], buffer[i + 1]);

    m_stats.numSamplesProcessed += buffer.size() / 2;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processingCompleted(buffer.size() / 2, timer.elapsed());
}

/* ---- Reset ---- */

void Flanger5::reset()
{
    m_delayLeft.fill(0.0);
    m_delayRight.fill(0.0);
    m_writePos = 0;
    m_lfoPhase = 0.0;
}

/* ---- Reset statistics ---- */

void Flanger5::resetStatistics()
{
    m_delayLeft.clear(); m_delayRight.clear();
    m_delayLineSize = 0;
    m_stats = Stats{}; m_timeSum = 0.0;
}

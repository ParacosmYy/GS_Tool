/**
 * @file Flanger4.cpp
 * @brief Flanger4 实现
 *
 * 实现镶边效果器：双调制延迟线、立体声交叉反馈、过零镶边。
 */

#include "utils/dsp224/Flanger4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Flanger4::Flanger4(QObject *parent) : QObject(parent)
{
    setParameters();
}

Flanger4::~Flanger4() = default;

/* ---- Configuration ---- */

void Flanger4::setParameters(int sampleRate, double baseDelayMs, double depthMs,
                               double rateHz, double feedback, double mix,
                               Waveform wave)
{
    m_sampleRate = qMax(8000, sampleRate);
    m_baseDelayMs = qMax(0.1, baseDelayMs);
    m_depthMs = qMax(0.0, depthMs);
    m_rateHz = qBound(0.01, rateHz, 20.0);
    m_feedback = qBound(0.0, feedback, 0.95);
    m_mix = qBound(0.0, mix, 1.0);
    m_waveform = wave;

    // Allocate delay lines: max delay = baseDelay + depth + margin
    double maxDelayMs = m_baseDelayMs + m_depthMs + 1.0;
    m_delayLength = static_cast<int>(maxDelayMs * m_sampleRate / 1000.0) + 2;
    m_delayLength = qMax(m_delayLength, 4);

    m_delayLineL.resize(m_delayLength, 0.0);
    m_delayLineR.resize(m_delayLength, 0.0);
    m_writePos = 0;

    m_lfoInc = m_rateHz / m_sampleRate;
    m_lfoPhase = 0.0;
    m_stats.sampleRate = m_sampleRate;
}

/* ---- LFO tick ---- */

double Flanger4::tickLFO()
{
    double val = 0.0;
    switch (m_waveform) {
    case Sine:
        val = qSin(2.0 * M_PI * m_lfoPhase);
        break;
    case Triangle:
        val = (m_lfoPhase < 0.5) ? (4.0 * m_lfoPhase - 1.0) : (3.0 - 4.0 * m_lfoPhase);
        break;
    case Sawtooth:
        val = 2.0 * m_lfoPhase - 1.0;
        break;
    case Square:
        val = (m_lfoPhase < 0.5) ? 1.0 : -1.0;
        break;
    }

    m_lfoPhase += m_lfoInc;
    if (m_lfoPhase >= 1.0) m_lfoPhase -= 1.0;
    return val;
}

/* ---- Fractional delay read ---- */

double Flanger4::readFractional(const QVector<double>& line, double position) const
{
    // Linear interpolation for fractional delay
    int posInt = static_cast<int>(position);
    double frac = position - posInt;

    int idx0 = posInt % m_delayLength;
    int idx1 = (posInt + 1) % m_delayLength;
    if (idx0 < 0) idx0 += m_delayLength;
    if (idx1 < 0) idx1 += m_delayLength;

    return line[idx0] * (1.0 - frac) + line[idx1] * frac;
}

/* ---- Process single sample ---- */

void Flanger4::processSample(double inL, double inR, double* outL, double* outR)
{
    // Advance LFO
    double lfoVal = tickLFO();

    // Compute current delay in samples
    double delayMs = m_baseDelayMs + m_depthMs * lfoVal;
    double delaySamples = delayMs * m_sampleRate / 1000.0;

    // Through-zero: allow negative delay via crossed feedback
    bool negativeDelay = (delaySamples < 0.0);
    double absDelay = qAbs(delaySamples);

    // Read from delay lines with fractional offset
    double readPos = static_cast<double>(m_writePos) - absDelay;
    while (readPos < 0.0) readPos += m_delayLength;
    while (readPos >= m_delayLength) readPos -= m_delayLength;

    double delayedL = readFractional(m_delayLineL, readPos);
    double delayedR = readFractional(m_delayLineR, readPos);

    // Stereo-crossed feedback: L feedback goes to R delay and vice versa
    double feedbackL = m_feedback * (negativeDelay ? delayedR : delayedL);
    double feedbackR = m_feedback * (negativeDelay ? delayedL : delayedR);

    // Write to delay lines (dry + crossed feedback)
    m_delayLineL[m_writePos] = inL + feedbackR;
    m_delayLineR[m_writePos] = inR + feedbackL;

    // Advance write position
    m_writePos = (m_writePos + 1) % m_delayLength;

    // Output: dry + wet mix
    *outL = inL + m_mix * (delayedL - inL);
    *outR = inR + m_mix * (delayedR - inR);
}

/* ---- Process block ---- */

QVector<double> Flanger4::processBlock(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int numSamples = input.size() / 2; // Stereo pairs
    QVector<double> output(input.size(), 0.0);

    for (int i = 0; i < numSamples; ++i) {
        double inL = (2 * i < input.size()) ? input[2 * i] : 0.0;
        double inR = (2 * i + 1 < input.size()) ? input[2 * i + 1] : 0.0;
        double outL, outR;
        processSample(inL, inR, &outL, &outR);
        output[2 * i] = outL;
        output[2 * i + 1] = outR;
    }

    m_stats.blockSize = numSamples;
    m_stats.totalSamples += numSamples;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingCompleted(numSamples, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void Flanger4::reset()
{
    m_delayLineL.fill(0.0);
    m_delayLineR.fill(0.0);
    m_writePos = 0;
    m_lfoPhase = 0.0;
}

/* ---- Reset statistics ---- */

void Flanger4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    reset();
}

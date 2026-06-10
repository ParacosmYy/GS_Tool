/**
 * @file Flanger8.cpp
 * @brief Flanger8 实现
 *
 * 实现镶边效果器：可变延迟深度调制与立体声交叉反馈的维度扫频效果。
 */

#include "utils/dsp280/Flanger8.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

Flanger8::Flanger8(QObject *parent)
    : QObject(parent)
{
    m_delayL.resize(MAX_DELAY, 0.0);
    m_delayR.resize(MAX_DELAY, 0.0);
    reset();
}

Flanger8::~Flanger8() = default;

/* ---- Configuration ---- */

void Flanger8::setParams(const Params& params)
{
    m_params.rate = qBound(0.01, params.rate, 20.0);
    m_params.depth = qBound(0.0, params.depth, 1.0);
    m_params.feedback = qBound(0.0, params.feedback, 0.95);
    m_params.mix = qBound(0.0, params.mix, 1.0);
    m_params.stereoPhase = qBound(0.0, params.stereoPhase, 360.0);
    m_params.crossFeed = qBound(0.0, params.crossFeed, 0.95);
    m_params.baseDelay = qBound(1, params.baseDelay, MAX_DELAY / 2);
}

void Flanger8::setSampleRate(double rate) { m_sampleRate = qBound(8000.0, rate, 192000.0); }

/* ---- LFO: sine oscillator ---- */

double Flanger8::lfo(double phase) const
{
    return qSin(2.0 * M_PI * phase);
}

/* ---- Advance LFO phase ---- */

double Flanger8::advancePhase(double phase) const
{
    double inc = m_params.rate / m_sampleRate;
    phase += inc;
    if (phase >= 1.0) phase -= 1.0;
    return phase;
}

/* ---- Fractional delay read with linear interpolation ---- */

double Flanger8::readDelay(const QVector<double>& buffer, double delaySamples) const
{
    double readPos = static_cast<double>(m_writePos) - delaySamples;
    while (readPos < 0) readPos += MAX_DELAY;
    while (readPos >= MAX_DELAY) readPos -= MAX_DELAY;

    int idx0 = static_cast<int>(readPos);
    int idx1 = (idx0 + 1) % MAX_DELAY;
    double frac = readPos - idx0;

    return buffer[idx0] * (1.0 - frac) + buffer[idx1] * frac;
}

/* ---- Reset ---- */

void Flanger8::reset()
{
    m_delayL.fill(0.0);
    m_delayR.fill(0.0);
    m_writePos = 0;
    m_phaseL = 0.0;
    m_phaseR = m_params.stereoPhase / 360.0;
    m_crossL = 0.0;
    m_crossR = 0.0;
}

/* ---- Process mono to stereo ---- */

Flanger8::StereoBuffer Flanger8::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    StereoBuffer output;
    int n = input.size();
    output.left.resize(n, 0.0);
    output.right.resize(n, 0.0);

    // Maximum modulation in samples (flanger: 0..10ms range)
    double maxModSamples = 0.01 * m_sampleRate;  // 10ms

    for (int i = 0; i < n; ++i) {
        double in = input[i];

        // LFO modulation for each channel
        double modL = lfo(m_phaseL);
        double modR = lfo(m_phaseR);

        // Delay amount = baseDelay + depth * maxMod * (0.5 + 0.5 * lfo)
        double delayL = m_params.baseDelay + m_params.depth * maxModSamples *
                        (0.5 + 0.5 * modL);
        double delayR = m_params.baseDelay + m_params.depth * maxModSamples *
                        (0.5 + 0.5 * modR);

        // Read from delay lines with cross-feedback
        double delayedL = readDelay(m_delayL, delayL) + m_crossR * m_params.crossFeed;
        double delayedR = readDelay(m_delayR, delayR) + m_crossL * m_params.crossFeed;

        // Write input + feedback to delay lines
        m_delayL[m_writePos] = in + delayedL * m_params.feedback;
        m_delayR[m_writePos] = in + delayedR * m_params.feedback;

        // Store cross-feedback state
        m_crossL = delayedL;
        m_crossR = delayedR;

        // Mix dry + wet
        output.left[i] = in * (1.0 - m_params.mix) + delayedL * m_params.mix;
        output.right[i] = in * (1.0 - m_params.mix) + delayedR * m_params.mix;

        // Advance write position
        m_writePos = (m_writePos + 1) % MAX_DELAY;

        // Advance LFO phases
        m_phaseL = advancePhase(m_phaseL);
        m_phaseR = advancePhase(m_phaseR);
    }

    double elapsed = timer.elapsed();
    m_stats.numSamples = n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingDone(n, elapsed);

    return output;
}

/* ---- Process stereo input ---- */

Flanger8::StereoBuffer Flanger8::processStereo(const QVector<double>& inL,
                                                const QVector<double>& inR)
{
    QElapsedTimer timer;
    timer.start();

    StereoBuffer output;
    int n = qMin(inL.size(), inR.size());
    output.left.resize(n, 0.0);
    output.right.resize(n, 0.0);

    double maxModSamples = 0.01 * m_sampleRate;

    for (int i = 0; i < n; ++i) {
        double lIn = inL[i];
        double rIn = inR[i];

        double modL = lfo(m_phaseL);
        double modR = lfo(m_phaseR);

        double delayL = m_params.baseDelay + m_params.depth * maxModSamples *
                        (0.5 + 0.5 * modL);
        double delayR = m_params.baseDelay + m_params.depth * maxModSamples *
                        (0.5 + 0.5 * modR);

        // Cross-feedback: left reads from right's delay line and vice versa
        double delayedL = readDelay(m_delayL, delayL) + m_crossR * m_params.crossFeed;
        double delayedR = readDelay(m_delayR, delayR) + m_crossL * m_params.crossFeed;

        m_delayL[m_writePos] = lIn + delayedL * m_params.feedback;
        m_delayR[m_writePos] = rIn + delayedR * m_params.feedback;

        m_crossL = delayedL;
        m_crossR = delayedR;

        output.left[i] = lIn * (1.0 - m_params.mix) + delayedL * m_params.mix;
        output.right[i] = rIn * (1.0 - m_params.mix) + delayedR * m_params.mix;

        m_writePos = (m_writePos + 1) % MAX_DELAY;
        m_phaseL = advancePhase(m_phaseL);
        m_phaseR = advancePhase(m_phaseR);
    }

    double elapsed = timer.elapsed();
    m_stats.numSamples = n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingDone(n, elapsed);

    return output;
}

/* ---- Reset ---- */

void Flanger8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    reset();
}

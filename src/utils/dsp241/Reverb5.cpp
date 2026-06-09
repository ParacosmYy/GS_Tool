/**
 * @file Reverb5.cpp
 * @brief Reverb5 实现
 *
 * 实现施罗德混响：并联梳状滤波器与全通反馈环路。
 */

#include "utils/dsp241/Reverb5.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

Reverb5::Reverb5(QObject *parent) : QObject(parent)
{
    initFilters();
}
Reverb5::~Reverb5() = default;

/* ---- Configuration ---- */

void Reverb5::setSampleRate(int rate)
{
    m_sampleRate = qMax(8000, rate);
    initFilters();
}

void Reverb5::setDecayTime(double ms)
{
    m_decayTimeMs = qMax(10.0, ms);
    initFilters();
}

void Reverb5::setWetDryMix(double mix) { m_wetDry = qBound(0.0, mix, 1.0); }

/* ---- Initialize filters ---- */

void Reverb5::initFilters()
{
    // Schroeder comb filter delay lengths (prime-based, in samples)
    static const int combDelays[kNumCombs] = { 1557, 1617, 1491, 1422 };
    // All-pass delay lengths
    static const int apDelays[kNumAllPass] = { 225, 556 };

    double t60 = m_decayTimeMs / 1000.0;

    // Comb filters: feedback = 10^(-3 * delay / (sampleRate * T60))
    m_combBuffers.resize(kNumCombs);
    m_combFeedback.resize(kNumCombs);
    m_combIndex.resize(kNumCombs, 0);
    m_combDelay.resize(kNumCombs);

    for (int i = 0; i < kNumCombs; ++i) {
        int delay = qMax(1, combDelays[i] * m_sampleRate / 44100);
        m_combDelay[i] = delay;
        m_combBuffers[i].resize(delay, 0.0);
        // Compute feedback gain for desired T60
        double expArg = -3.0 * delay / (static_cast<double>(m_sampleRate) * t60);
        m_combFeedback[i] = qPow(10.0, expArg);
    }

    // All-pass filters: feedback ~0.5
    m_apBuffers.resize(kNumAllPass);
    m_apFeedback.resize(kNumAllPass, 0.5);
    m_apIndex.resize(kNumAllPass, 0);
    m_apDelay.resize(kNumAllPass);

    for (int i = 0; i < kNumAllPass; ++i) {
        int delay = qMax(1, apDelays[i] * m_sampleRate / 44100);
        m_apDelay[i] = delay;
        m_apBuffers[i].resize(delay, 0.0);
    }

    m_stats.sampleRate = m_sampleRate;
    m_stats.decayTimeMs = m_decayTimeMs;
}

/* ---- Process single sample ---- */

double Reverb5::processOne(double sample)
{
    // Step 1: Parallel comb filter bank
    double combSum = 0.0;
    for (int i = 0; i < kNumCombs; ++i) {
        double delayed = m_combBuffers[i][m_combIndex[i]];
        double filtered = sample + delayed * m_combFeedback[i];
        m_combBuffers[i][m_combIndex[i]] = filtered;
        m_combIndex[i] = (m_combIndex[i] + 1) % m_combDelay[i];
        combSum += delayed;
    }
    combSum /= kNumCombs;

    // Step 2: Series all-pass feedback loop
    double apOut = combSum;
    for (int i = 0; i < kNumAllPass; ++i) {
        double delayed = m_apBuffers[i][m_apIndex[i]];
        double input = apOut + delayed * m_apFeedback[i];
        m_apBuffers[i][m_apIndex[i]] = input;
        m_apIndex[i] = (m_apIndex[i] + 1) % m_apDelay[i];
        apOut = delayed - input * m_apFeedback[i];
    }

    // Step 3: Wet/dry mix
    return sample * (1.0 - m_wetDry) + apOut * m_wetDry;
}

/* ---- Process buffer ---- */

QVector<double> Reverb5::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    output.resize(input.size());
    for (int i = 0; i < input.size(); ++i)
        output[i] = processOne(input[i]);

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processingCompleted(input.size(), timer.elapsed());
    return output;
}

/* ---- Reset delay lines ---- */

void Reverb5::reset()
{
    for (int i = 0; i < kNumCombs; ++i) {
        m_combBuffers[i].fill(0.0);
        m_combIndex[i] = 0;
    }
    for (int i = 0; i < kNumAllPass; ++i) {
        m_apBuffers[i].fill(0.0);
        m_apIndex[i] = 0;
    }
}

/* ---- Reset statistics ---- */

void Reverb5::resetStatistics()
{
    reset();
    m_stats = Stats{}; m_timeSum = 0.0;
}

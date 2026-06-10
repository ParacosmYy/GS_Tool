/**
 * @file Reverb8.cpp
 * @brief Reverb8 实现
 *
 * 实现混响器：Schroeder并行梳状滤波与级联全通网络人工房间脉冲响应模拟。
 */

#include "utils/dsp269/Reverb8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Reverb8::Reverb8(QObject *parent)
    : QObject(parent)
{
    initFilters();
}

Reverb8::~Reverb8() = default;

/* ---- Configuration ---- */

void Reverb8::setSampleRate(double rate)
{
    m_sampleRate = qMax(8000.0, rate);
    initFilters();
}

void Reverb8::setDecayTime(double seconds)
{
    m_decayTime = qBound(0.1, seconds, 10.0);
    // Recalculate feedback coefficients
    for (auto& comb : m_combs) {
        if (comb.delay > 0)
            comb.feedback = qPow(0.001, static_cast<double>(comb.delay) / (m_decayTime * m_sampleRate));
    }
}

void Reverb8::setWetDryMix(double mix)
{
    m_wetDry = qBound(0.0, mix, 1.0);
}

/* ---- Initialize Schroeder filter topology ---- */

void Reverb8::initFilters()
{
    // Schroeder's classic 4-comb + 2-allpass configuration
    // Comb delays in samples (prime-based for diffusion)
    int combDelays[] = {1119, 1189, 1277, 1357, 1423, 1491, 1559, 1621};
    int allPassDelays[] = {225, 557, 443, 373};
    int numCombs = 8;
    int numAP = 4;

    m_combs.resize(numCombs);
    m_allPasses.resize(numAP);

    for (int i = 0; i < numCombs; ++i) {
        int d = static_cast<int>(combDelays[i] * m_sampleRate / 44100.0);
        d = qMax(1, d);
        m_combs[i].buffer.resize(d);
        m_combs[i].buffer.fill(0.0);
        m_combs[i].delay = d;
        m_combs[i].index = 0;
        m_combs[i].feedback = qPow(0.001, static_cast<double>(d) / (m_decayTime * m_sampleRate));
    }

    for (int i = 0; i < numAP; ++i) {
        int d = static_cast<int>(allPassDelays[i] * m_sampleRate / 44100.0);
        d = qMax(1, d);
        m_allPasses[i].buffer.resize(d);
        m_allPasses[i].buffer.fill(0.0);
        m_allPasses[i].delay = d;
        m_allPasses[i].index = 0;
        m_allPasses[i].feedback = 0.5;
    }
}

/* ---- Comb filter processing ---- */

double Reverb8::processComb(CombFilter& comb, double input)
{
    double output = comb.buffer[comb.index];
    comb.buffer[comb.index] = input + output * comb.feedback;
    comb.index = (comb.index + 1) % comb.delay;
    return output;
}

/* ---- All-pass filter processing ---- */

double Reverb8::processAllPass(AllPassFilter& ap, double input)
{
    double delayed = ap.buffer[ap.index];
    double output = -input + delayed;
    ap.buffer[ap.index] = input + delayed * ap.feedback;
    ap.index = (ap.index + 1) % ap.delay;
    return output;
}

/* ---- Process block ---- */

QVector<double> Reverb8::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double sample = input[i];

        // Parallel comb filters (sum)
        double combSum = 0.0;
        for (auto& comb : m_combs)
            combSum += processComb(comb, sample);
        combSum /= m_combs.size();

        // Series all-pass filters
        double apOut = combSum;
        for (auto& ap : m_allPasses)
            apOut = processAllPass(ap, apOut);

        // Wet/dry mix
        output[i] = sample * (1.0 - m_wetDry) + apOut * m_wetDry;
    }

    double elapsed = timer.elapsed();
    m_stats.blockSize = n;
    m_stats.numCombFilters = m_combs.size();
    m_stats.numAllPassFilters = m_allPasses.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingCompleted(n, elapsed);

    return output;
}

/* ---- Generate impulse response ---- */

QVector<double> Reverb8::impulseResponse(int length) const
{
    // Create temporary copy of filters to avoid mutating state
    Reverb8 temp(nullptr);
    temp.m_sampleRate = m_sampleRate;
    temp.m_decayTime = m_decayTime;
    temp.m_wetDry = 1.0;
    temp.initFilters();

    QVector<double> impulse(length, 0.0);
    impulse[0] = 1.0;
    return temp.process(impulse);
}

/* ---- Reset delay lines ---- */

void Reverb8::reset()
{
    for (auto& comb : m_combs) {
        comb.buffer.fill(0.0);
        comb.index = 0;
    }
    for (auto& ap : m_allPasses) {
        ap.buffer.fill(0.0);
        ap.index = 0;
    }
}

/* ---- Reset statistics ---- */

void Reverb8::resetStatistics()
{
    reset();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

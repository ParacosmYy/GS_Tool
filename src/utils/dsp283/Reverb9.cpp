/**
 * @file Reverb9.cpp
 * @brief Reverb9 实现
 *
 * 实现混响：后反射扩散场与早反射抽头延迟的混合房间声学仿真。
 */

#include "utils/dsp283/Reverb9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Reverb9::Reverb9(QObject *parent)
    : QObject(parent)
{
    initFilters();
}

Reverb9::~Reverb9() = default;

/* ---- Configuration ---- */

void Reverb9::setParams(const ReverbParams& params) { m_params = params; initFilters(); }
void Reverb9::setSampleRate(double sr) { m_sampleRate = qMax(8000.0, sr); initFilters(); }

/* ---- Initialize Schroeder reverb filters ---- */

void Reverb9::initFilters()
{
    double sr = m_sampleRate;
    double scale = m_params.roomSize + 0.3;

    // Comb filter delay lengths (tuned for 44100 Hz)
    static const int baseCombLen[4] = {1116, 1188, 1277, 1356};
    // Allpass filter delay lengths
    static const int baseApLen[4] = {556, 441, 341, 225};

    double dampFactor = m_params.damping * 0.4;
    double fbBase = 0.28 + m_params.roomSize * 0.42;

    for (int i = 0; i < 4; ++i) {
        int len = qMax(10, static_cast<int>(baseCombLen[i] * scale * sr / 44100.0));
        m_combs[i].buffer.resize(len, 0.0);
        m_combs[i].pos = 0;
        m_combs[i].feedback = fbBase - dampFactor * 0.1;
        m_combs[i].damp1 = dampFactor;
        m_combs[i].damp2 = 1.0 - dampFactor;
        m_combs[i].filterStore = 0.0;

        int apLen = qMax(4, static_cast<int>(baseApLen[i] * scale * sr / 44100.0));
        m_allpass[i].buffer.resize(apLen, 0.0);
        m_allpass[i].pos = 0;
        m_allpass[i].feedback = 0.5;
    }

    // Early reflections buffer
    int earlySize = static_cast<int>(0.1 * sr);  // 100ms pre-delay buffer
    m_earlyBuffer.resize(qMax(earlySize, 1), 0.0);
    m_earlyPos = 0;

    generateEarlyTaps();
}

/* ---- Generate early reflection taps ---- */

void Reverb9::generateEarlyTaps()
{
    m_earlyTaps.clear();
    m_earlyGains.clear();

    // Simulate 8 early reflections with decreasing gain
    static const double baseDelays[8] = {
        0.003, 0.007, 0.012, 0.019, 0.026, 0.033, 0.041, 0.050
    };
    double gain = 1.0;
    for (int i = 0; i < 8; ++i) {
        int tap = qMax(1, static_cast<int>(baseDelays[i] * m_sampleRate * m_params.roomSize));
        tap = qMin(tap, m_earlyBuffer.size() - 1);
        m_earlyTaps.append(tap);
        gain *= 0.72;
        m_earlyGains.append(gain);
    }
}

/* ---- Process comb filter sample ---- */

double Reverb9::processComb(CombFilter& c, double input)
{
    double output = c.buffer[c.pos];
    c.filterStore = output * c.damp2 + c.filterStore * c.damp1;
    c.buffer[c.pos] = input + c.filterStore * c.feedback;
    c.pos = (c.pos + 1) % c.buffer.size();
    return output;
}

/* ---- Process allpass filter sample ---- */

double Reverb9::processAllpass(AllpassFilter& ap, double input)
{
    double bufOut = ap.buffer[ap.pos];
    double output = -input + bufOut;
    ap.buffer[ap.pos] = input + bufOut * ap.feedback;
    ap.pos = (ap.pos + 1) % ap.buffer.size();
    return output;
}

/* ---- Main processing ---- */

Reverb9::ReverbOutput Reverb9::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    ReverbOutput result;
    int n = input.size();
    result.audio.resize(n);

    double peakLevel = 0.0;
    double sumSq = 0.0;

    for (int i = 0; i < n; ++i) {
        double dry = input[i] * m_params.dryLevel;

        // Early reflections via tapped delay line
        double early = 0.0;
        for (int t = 0; t < m_earlyTaps.size(); ++t) {
            int readPos = (m_earlyPos - m_earlyTaps[t] + m_earlyBuffer.size())
                          % m_earlyBuffer.size();
            early += m_earlyBuffer[readPos] * m_earlyGains[t];
        }
        m_earlyBuffer[m_earlyPos] = input[i];
        m_earlyPos = (m_earlyPos + 1) % m_earlyBuffer.size();

        // Late reflections: parallel comb + series allpass (Schroeder)
        double lateInput = input[i] + early * 0.25;
        double lateSum = 0.0;
        for (int c = 0; c < 4; ++c)
            lateSum += processComb(m_combs[c], lateInput);
        lateSum *= 0.25;

        for (int a = 0; a < 4; ++a)
            lateSum = processAllpass(m_allpass[a], lateSum);

        // Stereo width simulation: modulate wet signal
        double wet = (early * 0.3 + lateSum * 0.7) * m_params.wetLevel;
        double sample = dry + wet;
        result.audio[i] = sample;

        double absVal = qAbs(sample);
        if (absVal > peakLevel) peakLevel = absVal;
        sumSq += sample * sample;
    }

    result.peakLevel = peakLevel;
    result.rmsLevel = (n > 0) ? qSqrt(sumSq / n) : 0.0;

    double elapsed = timer.elapsed();
    m_stats.numSamples = n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processDone(n, peakLevel, elapsed);

    return result;
}

/* ---- Clear buffers ---- */

void Reverb9::clearBuffers()
{
    for (int i = 0; i < 4; ++i) {
        m_combs[i].buffer.fill(0.0);
        m_combs[i].pos = 0;
        m_combs[i].filterStore = 0.0;
        m_allpass[i].buffer.fill(0.0);
        m_allpass[i].pos = 0;
    }
    m_earlyBuffer.fill(0.0);
    m_earlyPos = 0;
}

/* ---- Reset ---- */

void Reverb9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    clearBuffers();
}

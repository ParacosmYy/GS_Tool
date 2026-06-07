/**
 * @file Phaser3.cpp
 * @brief Phaser3 实现
 *
 * 实现移相器：全通级联调制、立体声正交LFO、空间扫描效果。
 */

#include "utils/dsp211/Phaser3.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

Phaser3::Phaser3(int sampleRate, QObject *parent)
    : QObject(parent), m_sampleRate(sampleRate), m_stages(6),
      m_lfoFreq(0.5), m_depth(0.7), m_feedback(0.4), m_wetDry(0.5)
{
    setStages(m_stages);
}

Phaser3::~Phaser3() = default;

/* ---- Configuration ---- */

void Phaser3::setStages(int stages)
{
    m_stages = qBound(1, stages, 12);
    m_apL.resize(m_stages);
    m_apR.resize(m_stages);
    for (auto& s : m_apL) s = AllpassState{};
    for (auto& s : m_apR) s = AllpassState{};
    m_stats.numStages = m_stages;
}

void Phaser3::setLFOFrequency(double freq)
{
    m_lfoFreq = qBound(0.01, freq, 20.0);
    m_stats.lfoFrequency = m_lfoFreq;
}

void Phaser3::setDepth(double depth)
{
    m_depth = qBound(0.0, depth, 1.0);
    m_stats.depth = m_depth;
}

void Phaser3::setFeedback(double fb)
{
    m_feedback = qBound(0.0, fb, 0.95);
    m_stats.feedback = m_feedback;
}

void Phaser3::setSampleRate(int sr) { m_sampleRate = qMax(1, sr); }
void Phaser3::setWetDryMix(double mix) { m_wetDry = qBound(0.0, mix, 1.0); }

/* ---- LFO value ---- */

double Phaser3::lfoValue(double phase) const
{
    // Sine LFO: output in [0, 1]
    return 0.5 + 0.5 * qSin(2.0 * M_PI * phase);
}

/* ---- Second-order allpass ---- */

double Phaser3::allpass2(double input, double coeff, AllpassState& state)
{
    // Second-order allpass: y[n] = -a*x[n] + x[n-2] + a*(a*y[n-1] + y[n-2] - x[n-1])
    // Simplified form for stability
    double a = coeff;
    double output = a * (input + state.y1) - state.x1 + state.y2;
    // Avoid division, use stable direct form II
    // H(z) = (a + z^{-2}) / (1 + a*z^{-2})
    double y = a * input + state.x2 - a * state.y2;
    state.x2 = state.x1;
    state.x1 = input;
    state.y2 = state.y1;
    state.y1 = y;
    return y;
}

/* ---- Process cascade ---- */

double Phaser3::processCascade(double input, QVector<AllpassState>& states,
                                double modAmount)
{
    double sample = input;
    for (int s = 0; s < m_stages; ++s) {
        // Allpass coefficient varies with modulation
        // Map LFO to allpass coefficient range [0.3, 0.9]
        double stageOffset = static_cast<double>(s) / m_stages;
        double coeff = 0.3 + 0.6 * modAmount * (1.0 - 0.3 * stageOffset);
        coeff = qBound(0.1, coeff, 0.95);
        sample = allpass2(sample, coeff, states[s]);
    }
    return sample;
}

/* ---- Process single stereo sample ---- */

void Phaser3::processSample(double inputL, double inputR,
                             double& outputL, double& outputR)
{
    // Compute LFO values (quadrature: 90 degrees apart)
    double modL = lfoValue(m_lfoPhaseL);
    double modR = lfoValue(m_lfoPhaseR);

    // Apply feedback
    double fbInputL = inputL + m_fbL * m_feedback;
    double fbInputR = inputR + m_fbR * m_feedback;

    // Process through allpass cascades
    double wetL = processCascade(fbInputL, m_apL, modL * m_depth);
    double wetR = processCascade(fbInputR, m_apR, modR * m_depth);

    // Store feedback
    m_fbL = wetL;
    m_fbR = wetR;

    // Wet/dry mix
    outputL = inputL * (1.0 - m_wetDry) + wetL * m_wetDry;
    outputR = inputR * (1.0 - m_wetDry) + wetR * m_wetDry;

    // Advance LFO phase (quadrature: right channel is 0.25 cycles ahead)
    double phaseInc = m_lfoFreq / m_sampleRate;
    m_lfoPhaseL += phaseInc;
    m_lfoPhaseR += phaseInc;
    if (m_lfoPhaseL >= 1.0) m_lfoPhaseL -= 1.0;
    if (m_lfoPhaseR >= 1.0) m_lfoPhaseR -= 1.0;

    // Set right channel to quadrature offset
    m_lfoPhaseR = m_lfoPhaseL + 0.25;
    if (m_lfoPhaseR >= 1.0) m_lfoPhaseR -= 1.0;
}

/* ---- Process mono buffer ---- */

QVector<double> Phaser3::processMono(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double outL, outR;
        processSample(input[i], input[i], outL, outR);
        output[i] = (outL + outR) * 0.5;
    }

    m_stats.totalSamples += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSamples > 0)
        ? m_timeSum * 1000.0 / m_stats.totalSamples : 0.0;
    emit processingCompleted(n, timer.elapsed());
    return output;
}

/* ---- Process stereo interleaved ---- */

QVector<double> Phaser3::processStereo(const QVector<double>& interleaved)
{
    QElapsedTimer timer;
    timer.start();

    int n = interleaved.size() / 2;
    QVector<double> output(interleaved.size(), 0.0);

    for (int i = 0; i < n; ++i) {
        double outL, outR;
        processSample(interleaved[2 * i], interleaved[2 * i + 1], outL, outR);
        output[2 * i] = outL;
        output[2 * i + 1] = outR;
    }

    m_stats.totalSamples += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSamples > 0)
        ? m_timeSum * 1000.0 / m_stats.totalSamples : 0.0;
    emit processingCompleted(n, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void Phaser3::reset()
{
    for (auto& s : m_apL) s = AllpassState{};
    for (auto& s : m_apR) s = AllpassState{};
    m_lfoPhaseL = 0.0;
    m_lfoPhaseR = 0.25;
    m_fbL = 0.0;
    m_fbR = 0.0;
}

void Phaser3::resetStatistics()
{
    m_stats = Stats{};
    m_stats.numStages = m_stages;
    m_timeSum = 0.0;
    reset();
}

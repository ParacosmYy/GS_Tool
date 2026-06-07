/**
 * @file Phaser2.cpp
 * @brief Phaser2 实现
 *
 * 实现移相器：级联全通滤波器、LFO深度扫频、立体声正交调制。
 */

#include "utils/dsp193/Phaser2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Phaser2::Phaser2(QObject *parent) : QObject(parent)
{
    setStages(m_stages);
    updateLfoRate();
}

Phaser2::~Phaser2() = default;

/* ---- Configuration ---- */

void Phaser2::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
    m_stats.sampleRate = m_sampleRate;
    updateLfoRate();
}

void Phaser2::setStages(int stages)
{
    m_stages = qBound(1, stages, 24);
    m_stats.numStages = m_stages;
    m_allpass.resize(m_stages);
    for (auto& s : m_allpass) {
        s.x1 = 0.0;
        s.y1 = 0.0;
        s.coeff = 0.5;
    }
}

void Phaser2::setLfoFrequency(double freq)
{
    m_lfoFreq = qBound(0.01, freq, 20.0);
    updateLfoRate();
}

void Phaser2::setLfoDepth(double depth)
{
    m_lfoDepth = qBound(0.0, depth, 1.0);
}

void Phaser2::setFeedback(double fb)
{
    m_feedback = qBound(0.0, fb, 0.99);
}

void Phaser2::setMix(double mix)
{
    m_mix = qBound(0.0, mix, 1.0);
}

/* ---- Internal helpers ---- */

void Phaser2::updateLfoRate()
{
    m_lfoPhaseInc = 2.0 * M_PI * m_lfoFreq / m_sampleRate;
}

double Phaser2::advanceLfo()
{
    m_lfoPhase += m_lfoPhaseInc;
    if (m_lfoPhase >= 2.0 * M_PI)
        m_lfoPhase -= 2.0 * M_PI;

    // Map sine to [0..1] range for coefficient modulation
    return 0.5 + 0.5 * qSin(m_lfoPhase);
}

double Phaser2::allpass(AllPassStage& stage, double input)
{
    // First-order all-pass: y = -coeff * x + x1 + coeff * y1
    double output = -stage.coeff * input + stage.x1 + stage.coeff * stage.y1;
    stage.x1 = input;
    stage.y1 = output;
    return output;
}

/* ---- Process mono sample → stereo ---- */

QPair<double, double> Phaser2::processSample(double input)
{
    // Get LFO modulation value
    double mod = advanceLfo();

    // Map modulation to all-pass coefficient range [0.2 .. 0.9]
    double baseCoeff = 0.2 + mod * m_lfoDepth * 0.7;

    // Apply coefficient to all stages
    for (int i = 0; i < m_allpass.size(); ++i) {
        // Slight per-stage variation for richer phasing
        double variation = 1.0 + 0.05 * i;
        m_allpass[i].coeff = qBound(0.1, baseCoeff * variation, 0.95);
    }

    // Add feedback to input
    double sig = input + m_feedbackBuf * m_feedback;

    // Cascade through all-pass stages
    double filtered = sig;
    for (int i = 0; i < m_allpass.size(); ++i)
        filtered = allpass(m_allpass[i], filtered);

    // Store feedback
    m_feedbackBuf = filtered;

    // Stereo quadrature modulation:
    // L uses sine-phase modulated signal, R uses cosine-phase
    double lfoSin = qSin(m_lfoPhase);
    double lfoCos = qCos(m_lfoPhase);

    // Wet/dry mix
    double wetL = filtered * (1.0 + 0.3 * lfoSin);
    double wetR = filtered * (1.0 + 0.3 * lfoCos);

    double outL = input * (1.0 - m_mix) + wetL * m_mix;
    double outR = input * (1.0 - m_mix) + wetR * m_mix;

    return {outL, outR};
}

/* ---- Process buffer ---- */

QVector<double> Phaser2::processBuffer(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n * 2);  // interleaved stereo L,R

    for (int i = 0; i < n; ++i) {
        auto [l, r] = processSample(input[i]);
        output[i * 2] = l;
        output[i * 2 + 1] = r;
    }

    m_stats.totalSamples += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, m_stats.totalSamples / 256);

    emit processingCompleted(n, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void Phaser2::reset()
{
    for (auto& s : m_allpass) {
        s.x1 = 0.0;
        s.y1 = 0.0;
    }
    m_feedbackBuf = 0.0;
    m_lfoPhase = 0.0;
}

void Phaser2::resetStatistics()
{
    m_stats = Stats{};
    m_stats.sampleRate = m_sampleRate;
    m_stats.numStages = m_stages;
    m_timeSum = 0.0;
    reset();
}

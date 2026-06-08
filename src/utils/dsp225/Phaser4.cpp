/**
 * @file Phaser4.cpp
 * @brief Phaser4 实现
 *
 * 实现12阶全通网络相位器：随机LFO相位偏移、合奏厚度调制、反馈控制。
 */

#include "utils/dsp225/Phaser4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/* ---- Construction / Destruction ---- */

Phaser4::Phaser4(QObject *parent) : QObject(parent)
{
    m_allpassX.resize(ALLPASS_STAGES, 0.0);
    m_allpassY.resize(ALLPASS_STAGES, 0.0);
    m_lfoPhase.resize(ALLPASS_STAGES, 0.0);
    m_lfoPhaseOffset.resize(ALLPASS_STAGES, 0.0);
    initPhaseOffsets();
}

Phaser4::~Phaser4() = default;

/* ---- Initialize random phase offsets ---- */

void Phaser4::initPhaseOffsets()
{
    static std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(0.0, 2.0 * M_PI);
    for (int i = 0; i < ALLPASS_STAGES; ++i)
        m_lfoPhaseOffset[i] = dist(rng);
}

/* ---- Allpass coefficient ---- */

double Phaser4::allpassCoefficient(double freqHz) const
{
    // First-order allpass: a = (tan(pi*fc/fs) - 1) / (tan(pi*fc/fs) + 1)
    double tanVal = qTan(M_PI * freqHz / m_sampleRate);
    return (tanVal - 1.0) / (tanVal + 1.0);
}

/* ---- LFO value for stage ---- */

double Phaser4::lfoValueForStage(int stage)
{
    // Each stage has its own phase with randomized offset for ensemble thickness
    double phase = m_lfoPhase[stage] + m_lfoPhaseOffset[stage] * m_ensembleSpread;
    return qSin(phase);
}

/* ---- Process one sample ---- */

double Phaser4::processSample(double input)
{
    double sample = input + m_feedbackBuffer * m_feedback;

    // Process through 12-stage allpass chain
    for (int s = 0; s < ALLPASS_STAGES; ++s) {
        // Compute LFO-modulated center frequency for this stage
        double lfoVal = lfoValueForStage(s);
        double modFreq = m_baseFreq * (1.0 + m_lfoDepth * lfoVal);
        modFreq = qBound(20.0, modFreq, m_sampleRate * 0.45);

        double a = allpassCoefficient(modFreq);
        double x = sample;
        double y = a * x + m_allpassX[s] - a * m_allpassY[s];

        m_allpassX[s] = x;
        m_allpassY[s] = y;
        sample = y;

        // Advance per-stage LFO phase
        m_lfoPhase[s] += 2.0 * M_PI * m_lfoRate / m_sampleRate;
        if (m_lfoPhase[s] >= 2.0 * M_PI) m_lfoPhase[s] -= 2.0 * M_PI;
    }

    // Mix dry/wet (50/50 for phaser effect)
    double output = 0.5 * input + 0.5 * sample;
    m_feedbackBuffer = sample;

    return output;
}

/* ---- Prepare ---- */

bool Phaser4::prepare(double sampleRate, int blockSize)
{
    if (sampleRate < 1000.0 || blockSize < 1) return false;

    m_sampleRate = sampleRate;
    m_blockSize = blockSize;
    m_stats.sampleRate = sampleRate;
    m_stats.blockSize = blockSize;
    m_stats.numStages = ALLPASS_STAGES;

    // Reset allpass states
    m_allpassX.fill(0.0);
    m_allpassY.fill(0.0);
    m_lfoPhase.fill(0.0);
    m_feedbackBuffer = 0.0;

    return true;
}

/* ---- Process block ---- */

QVector<double> Phaser4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(input.size());
    for (int i = 0; i < input.size(); ++i)
        output[i] = processSample(input[i]);

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit processingCompleted(input.size(), timer.elapsed());
    return output;
}

/* ---- Setters ---- */

void Phaser4::setLfoRate(double rateHz)
{
    m_lfoRate = qBound(0.01, rateHz, 20.0);
}

void Phaser4::setLfoDepth(double depth)
{
    m_lfoDepth = qBound(0.0, depth, 1.0);
}

void Phaser4::setFeedback(double feedback)
{
    m_feedback = qBound(0.0, feedback, 0.95);
}

void Phaser4::setBaseFrequency(double freqHz)
{
    m_baseFreq = qBound(100.0, freqHz, 10000.0);
}

void Phaser4::setEnsembleSpread(double spread)
{
    m_ensembleSpread = qBound(0.0, spread, 1.0);
}

/* ---- Reset ---- */

void Phaser4::resetStatistics()
{
    m_stats = Stats{};
    m_allpassX.fill(0.0);
    m_allpassY.fill(0.0);
    m_lfoPhase.fill(0.0);
    m_feedbackBuffer = 0.0;
    m_timeSum = 0.0;
    initPhaseOffsets();
}

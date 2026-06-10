/**
 * @file Phaser8.cpp
 * @brief Phaser8 实现
 *
 * 实现相位器：多级全通反馈与包络跟踪调制深度的动态相位扫描效果。
 */

#include "utils/dsp281/Phaser8.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

Phaser8::Phaser8(QObject *parent)
    : QObject(parent)
{
    int stages = qBound(2, m_params.stages, 12);
    m_xz.resize(stages, 0.0);
    m_yz.resize(stages, 0.0);
}

Phaser8::~Phaser8() = default;

/* ---- Configuration ---- */

void Phaser8::setParameters(const Parameters& params)
{
    m_params = params;
    m_params.depth = qBound(0.0, params.depth, 1.0);
    m_params.feedback = qBound(0.0, params.feedback, 0.95);
    m_params.mix = qBound(0.0, params.mix, 1.0);
    m_params.stages = qBound(2, params.stages, 12);
    m_params.rate = qBound(0.01, params.rate, 20.0);

    int sz = m_params.stages;
    if (m_xz.size() != sz) {
        m_xz.resize(sz, 0.0);
        m_yz.resize(sz, 0.0);
    }
    m_stats.numStages = sz;
}

void Phaser8::setSampleRate(double rate)
{
    m_sampleRate = qBound(8000.0, rate, 192000.0);
}

/* ---- LFO value (sine oscillator with phase accumulator) ---- */

double Phaser8::lfoValue()
{
    // Advance phase
    double inc = m_params.rate / m_sampleRate;
    m_lfoPhase += inc;
    if (m_lfoPhase >= 1.0) m_lfoPhase -= 1.0;

    // Sine LFO mapped to 0..1 range
    return 0.5 + 0.5 * qSin(2.0 * M_PI * m_lfoPhase);
}

/* ---- All-pass coefficient from center frequency ---- */

double Phaser8::allPassCoeff(double freq) const
{
    // First-order all-pass: coeff = (tan(pi*fc/fs) - 1) / (tan(pi*fc/fs) + 1)
    double tanVal = qTan(M_PI * freq / m_sampleRate);
    return (tanVal - 1.0) / (tanVal + 1.0);
}

/* ---- Process single all-pass stage ---- */

double Phaser8::allPassStage(double input, int stage, double coeff)
{
    // First-order all-pass: y[n] = coeff * (x[n] - y[n-1]) + x[n-1]
    double output = coeff * (input - m_yz[stage]) + m_xz[stage];
    m_xz[stage] = input;
    m_yz[stage] = output;
    return output;
}

/* ---- Envelope follower ---- */

double Phaser8::followEnvelope(double sample)
{
    // Simple envelope follower with attack/release
    double absVal = qAbs(sample);
    double attackCoeff = 0.01;   // Fast attack
    double releaseCoeff = 0.0001; // Slow release

    if (absVal > m_envelope)
        m_envelope += attackCoeff * (absVal - m_envelope);
    else
        m_envelope += releaseCoeff * (absVal - m_envelope);

    return m_envelope;
}

/* ---- Process a single sample ---- */

double Phaser8::processSample(double input)
{
    // Get LFO value and compute dynamic modulation depth
    double lfo = lfoValue();
    double envDepth = followEnvelope(input);

    // Dynamic depth: envelope-following modulates the modulation range
    double effectiveDepth = m_params.depth * (0.5 + 0.5 * envDepth);

    // Modulate center frequency for each stage
    double freqSpread = m_params.baseFrequency * effectiveDepth;
    double freqBase = m_params.baseFrequency;

    // Add feedback
    double fbInput = input + m_feedbackSample * m_params.feedback;

    // Process through all-pass stages
    double sample = fbInput;
    for (int s = 0; s < m_params.stages; ++s) {
        // Each stage gets a slightly different modulation offset
        double stageOffset = static_cast<double>(s) / m_params.stages;
        double modFreq = freqBase + freqSpread * (lfo - 0.5 + stageOffset * 0.5);
        modFreq = qBound(20.0, modFreq, m_sampleRate * 0.45);

        double coeff = allPassCoeff(modFreq);
        sample = allPassStage(sample, s, coeff);
    }

    // Store feedback
    m_feedbackSample = sample;

    // Dry/wet mix
    double output = input * (1.0 - m_params.mix) + sample * m_params.mix;

    m_stats.totalSamples++;
    return output;
}

/* ---- Process a block of samples ---- */

QVector<double> Phaser8::processBlock(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n);
    double peakEnv = 0.0;

    for (int i = 0; i < n; ++i) {
        output[i] = processSample(input[i]);
        if (m_envelope > peakEnv) peakEnv = m_envelope;
    }

    double elapsed = timer.elapsed();
    m_stats.totalBlocks++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBlocks;
    emit blockDone(n, peakEnv, elapsed);

    return output;
}

/* ---- Reset ---- */

void Phaser8::reset()
{
    m_xz.fill(0.0);
    m_yz.fill(0.0);
    m_lfoPhase = 0.0;
    m_envelope = 0.0;
    m_feedbackSample = 0.0;
}

void Phaser8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    reset();
}

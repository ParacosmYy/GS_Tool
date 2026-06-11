/**
 * @file Phaser9.cpp
 * @brief Phaser9 实现
 *
 * 实现移相器：级联全通网络与LFO调制陷波扫频实现多级相位抵消扫频效果。
 */

#include "utils/dsp295/Phaser9.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

Phaser9::Phaser9(QObject *parent)
    : QObject(parent)
{
    reset();
}

Phaser9::~Phaser9() = default;

/* ---- Configuration ---- */

void Phaser9::setRate(double Hz) { m_rate = qBound(0.01, Hz, 20.0); }
void Phaser9::setDepth(double depth) { m_depth = qBound(0.0, depth, 1.0); }
void Phaser9::setStages(int stages) { m_stages = qBound(1, stages, 12); m_allPassHistory.resize(m_stages); }
void Phaser9::setFeedback(double fb) { m_feedback = qBound(0.0, fb, 0.95); }
void Phaser9::setMix(double mix) { m_mix = qBound(0.0, mix, 1.0); }
void Phaser9::setLFOType(LFOType type) { m_lfoType = type; }

/* ---- Compute LFO value ---- */

double Phaser9::computeLFO() const
{
    double phase = m_lfoPhase;
    switch (m_lfoType) {
    case LFOType::Sine:
        return qSin(2.0 * M_PI * phase);
    case LFOType::Triangle:
        return (phase < 0.5) ? (4.0 * phase - 1.0) : (3.0 - 4.0 * phase);
    case LFOType::Square:
        return (phase < 0.5) ? 1.0 : -1.0;
    }
    return qSin(2.0 * M_PI * phase);
}

/* ---- Frequency to all-pass coefficient ---- */

double Phaser9::freqToAllPassCoeff(double freq) const
{
    // Map frequency to all-pass coefficient
    // Center frequency range: ~200Hz to ~8kHz for phaser sweep
    double w = 2.0 * M_PI * freq / m_sampleRate;
    double cw = qCos(w);
    // All-pass coefficient: a = (1 - tan(w/2)) / (1 + tan(w/2))
    // Simplified using cosine approximation
    double tanHalf = qTan(w / 2.0);
    return (1.0 - tanHalf) / (1.0 + tanHalf);
}

/* ---- Single all-pass filter stage ---- */

double Phaser9::allPassStage(double input, double coeff, double& history) const
{
    // First-order all-pass: y[n] = coeff*(x[n] + y[n-1]) - x[n-1]
    double output = coeff * (input + history) - history * coeff * coeff;
    // Stable all-pass implementation
    double y = -coeff * input + history;
    history = input - coeff * y;
    return y;
}

/* ---- Process audio frame ---- */

Phaser9::ProcessResult Phaser9::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    ProcessResult result;
    int n = input.size();
    result.output.resize(n);

    // Base frequency for notch sweep (center of modulation range)
    double baseFreqMin = 200.0;
    double baseFreqMax = 8000.0;

    double peakLevel = 0.0;
    double sumSq = 0.0;

    for (int i = 0; i < n; ++i) {
        // Advance LFO phase
        m_lfoPhase += m_rate / m_sampleRate;
        if (m_lfoPhase >= 1.0) m_lfoPhase -= 1.0;

        // Compute modulated frequency
        double lfoVal = computeLFO();
        double modFreq = baseFreqMin + (baseFreqMax - baseFreqMin)
                         * (0.5 + 0.5 * m_depth * lfoVal);

        // Process through cascaded all-pass stages
        double sample = input[i] + m_feedback * m_feedbackSample;
        double coeff = freqToAllPassCoeff(modFreq);

        for (int s = 0; s < m_stages && s < m_allPassHistory.size(); ++s) {
            // Slightly detune each stage for richer effect
            double stageCoeff = coeff * (1.0 + 0.05 * s);
            stageCoeff = qBound(-0.99, stageCoeff, 0.99);
            sample = allPassStage(sample, stageCoeff, m_allPassHistory[s]);
        }

        m_feedbackSample = sample;

        // Mix dry and wet signals
        double wet = sample;
        double dry = input[i];
        result.output[i] = dry * (1.0 - m_mix) + wet * m_mix;

        double absOut = qAbs(result.output[i]);
        peakLevel = qMax(peakLevel, absOut);
        sumSq += result.output[i] * result.output[i];
    }

    result.peakLevel = peakLevel;
    result.rmsLevel = n > 0 ? qSqrt(sumSq / n) : 0.0;

    double elapsed = timer.elapsed();
    m_stats.frameSize = n;
    m_stats.totalFrames++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit frameProcessed(n, peakLevel, elapsed);
    return result;
}

/* ---- Reset ---- */

void Phaser9::reset()
{
    m_lfoPhase = 0.0;
    m_feedbackSample = 0.0;
    m_allPassHistory.resize(m_stages);
    m_allPassHistory.fill(0.0);
}

void Phaser9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

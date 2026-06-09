/**
 * @file Phaser6.cpp
 * @brief Phaser6 实现
 *
 * 实现移相器：多级全通滤波网络与每级随机LFO相位偏移厚调制。
 */

#include "utils/dsp253/Phaser6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <QtGlobal>

/* ---- Construction / Destruction ---- */

Phaser6::Phaser6(QObject *parent) : QObject(parent)
{
    initPhaseOffsets();
}
Phaser6::~Phaser6() = default;

/* ---- Initialize random LFO phase offsets ---- */

void Phaser6::initPhaseOffsets()
{
    m_lfoPhase.resize(m_stages);
    m_allpassX1.resize(m_stages, 0.0);
    m_allpassY1.resize(m_stages, 0.0);

    // Random phase offset in [0, 2*pi) per stage for thicker modulation
    for (int i = 0; i < m_stages; ++i) {
        m_lfoPhase[i] = (static_cast<double>(qrand()) / RAND_MAX) * 2.0 * M_PI;
    }
}

/* ---- Configuration ---- */

void Phaser6::setStages(int stages)
{
    m_stages = qBound(2, stages, 12);
    initPhaseOffsets();
}

void Phaser6::setLFORate(double hz)
{
    m_lfoRate = qBound(0.01, hz, 20.0);
}

void Phaser6::setDepth(double depth)
{
    m_depth = qBound(0.0, depth, 1.0);
}

void Phaser6::setFeedback(double fb)
{
    m_feedback = qBound(0.0, fb, 0.95);
}

void Phaser6::setMix(double mix)
{
    m_mix = qBound(0.0, mix, 1.0);
}

/* ---- Compute LFO value for a stage ---- */

double Phaser6::lfoValue(int stage) const
{
    // Sine LFO with per-stage random phase offset
    double phase = m_phase + m_lfoPhase[stage];
    return qSin(phase);
}

/* ---- Single all-pass filter stage ---- */

double Phaser6::allpass(double input, int stage, double coeff)
{
    // First-order all-pass: y[n] = coeff*(x[n] + y[n-1]) - x[n-1]
    double output = coeff * (input + m_allpassY1[stage]) - m_allpassX1[stage];
    m_allpassX1[stage] = input;
    m_allpassY1[stage] = output;
    return output;
}

/* ---- Process block ---- */

QVector<double> Phaser6::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n, 0.0);
    double phaseIncrement = 2.0 * M_PI * m_lfoRate / m_sampleRate;

    for (int i = 0; i < n; ++i) {
        double sample = input[i] + m_feedbackBuf * m_feedback;

        // Process through all-pass stages
        double filtered = sample;
        for (int s = 0; s < m_stages; ++s) {
            // LFO modulates the all-pass coefficient
            double lfo = lfoValue(s);
            // Map LFO [-1,1] -> coefficient range
            double minFreq = 200.0;
            double maxFreq = 8000.0;
            double freq = minFreq + (maxFreq - minFreq) * (0.5 + 0.5 * lfo * m_depth);
            double coeff = (qTan(M_PI * freq / m_sampleRate) - 1.0)
                           / (qTan(M_PI * freq / m_sampleRate) + 1.0);

            filtered = allpass(filtered, s, coeff);
        }

        // Store feedback
        m_feedbackBuf = filtered;

        // Dry/wet mix
        output[i] = input[i] * (1.0 - m_mix) + filtered * m_mix;

        // Advance master LFO phase
        m_phase += phaseIncrement;
        if (m_phase >= 2.0 * M_PI) m_phase -= 2.0 * M_PI;
    }

    m_stats.numSamples += n;
    m_stats.numBlocks++;
    m_stats.numStages = m_stages;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processingCompleted(n, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void Phaser6::reset()
{
    m_allpassX1.fill(0.0);
    m_allpassY1.fill(0.0);
    m_phase = 0.0;
    m_feedbackBuf = 0.0;
}

/* ---- Reset statistics ---- */

void Phaser6::resetStatistics()
{
    reset();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

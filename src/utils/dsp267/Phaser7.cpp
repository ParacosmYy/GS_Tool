/**
 * @file Phaser7.cpp
 * @brief Phaser7 实现
 *
 * 实现相位器：全通级联与正交LFO调制及反馈谐振的扫频相位消除。
 */

#include "utils/dsp267/Phaser7.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

Phaser7::Phaser7(QObject *parent)
    : QObject(parent)
{
    m_apX1.resize(12, 0.0);
    m_apY1.resize(12, 0.0);
}

Phaser7::~Phaser7() = default;

/* ---- Configuration ---- */

void Phaser7::setParameters(int stages, double lfoRate, double depth,
                             double feedback)
{
    m_stages = qBound(2, stages, 12);
    m_lfoRate = qBound(0.01, lfoRate, 20.0);
    m_depth = qBound(0.0, depth, 1.0);
    m_feedback = qBound(0.0, feedback, 0.95);
}

void Phaser7::setSampleRate(double sampleRate)
{
    m_sampleRate = qBound(8000.0, sampleRate, 192000.0);
}

/* ---- All-pass coefficient ---- */

double Phaser7::allPassCoeff(double normalizedFreq) const
{
    // Clamp to avoid instability
    double freq = qBound(0.0001, normalizedFreq, 0.4999);
    double tanVal = qTan(M_PI * freq);
    return (tanVal - 1.0) / (tanVal + 1.0);
}

/* ---- Quadrature LFO advance ---- */

void Phaser7::advanceLFO()
{
    m_lfoPhase += 2.0 * M_PI * m_lfoRate / m_sampleRate;
    if (m_lfoPhase >= 2.0 * M_PI) m_lfoPhase -= 2.0 * M_PI;
}

/* ---- Process single sample ---- */

double Phaser7::processSample(double input)
{
    // Quadrature LFO: use both sin and cos for stereo-like modulation
    double lfoSin = qSin(m_lfoPhase);
    double lfoCos = qCos(m_lfoPhase);

    // Map LFO to frequency sweep range (base: 200-8000 Hz)
    double sweepBase = 300.0;
    double sweepRange = 4000.0;

    double sample = input + m_feedback * m_fbSample;

    // Process through all-pass cascade
    for (int s = 0; s < m_stages; ++s) {
        // Stagger LFO phase per stage using quadrature mix
        double phaseOffset = static_cast<double>(s) / m_stages;
        double lfoValue = lfoSin * (1.0 - phaseOffset) + lfoCos * phaseOffset;
        double modulated = sweepBase + sweepRange * m_depth * (0.5 + 0.5 * lfoValue);
        double normFreq = modulated / m_sampleRate;

        double coeff = allPassCoeff(normFreq);
        double x1 = m_apX1[s];
        double y1 = m_apY1[s];

        // First-order all-pass: y[n] = coeff * (x[n] + y[n-1]) - x[n-1]
        double output = coeff * (sample + y1) - x1;
        m_apX1[s] = sample;
        m_apY1[s] = output;
        sample = output;
    }

    // Store feedback
    m_fbSample = sample;

    advanceLFO();
    return sample;
}

/* ---- Process block ---- */

QVector<double> Phaser7::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(input.size());
    for (int i = 0; i < input.size(); ++i) {
        output[i] = processSample(input[i]);
    }

    double elapsed = timer.elapsed();
    m_stats.blockSize = input.size();
    m_stats.numStages = m_stages;
    m_stats.feedbackLevel = m_feedback;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit phaserUpdated(input.size(), m_lfoRate, elapsed);

    return output;
}

/* ---- Reset ---- */

void Phaser7::reset()
{
    m_apX1.fill(0.0);
    m_apY1.fill(0.0);
    m_fbSample = 0.0;
    m_lfoPhase = 0.0;
}

/* ---- Reset statistics ---- */

void Phaser7::resetStatistics()
{
    reset();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

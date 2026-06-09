/**
 * @file Phaser5.cpp
 * @brief Phaser5 实现
 *
 * 实现移相器：全通级联调制、LFO驱动扫频与反馈混合。
 */

#include "utils/dsp239/Phaser5.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

Phaser5::Phaser5(QObject *parent) : QObject(parent)
{
    setStages(m_stages);
}

Phaser5::~Phaser5() = default;

/* ---- Configuration ---- */

void Phaser5::setStages(int stages)
{
    m_stages = qBound(2, stages, 12);
    m_x1.resize(m_stages, 0.0);
    m_y1.resize(m_stages, 0.0);
    m_stats.numStages = m_stages;
}

void Phaser5::setLfoRate(double hz) { m_lfoRate = qBound(0.01, hz, 20.0); }
void Phaser5::setLfoShape(LfoShape shape) { m_lfoShape = shape; }
void Phaser5::setDepth(double depth) { m_depth = qBound(0.0, depth, 1.0); }
void Phaser5::setFeedback(double fb) { m_feedback = qBound(-0.99, fb, 0.99); }
void Phaser5::setBaseFrequency(double hz) { m_baseFreq = qBound(20.0, hz, 16000.0); }

/* ---- LFO advance ---- */

double Phaser5::advanceLfo()
{
    double val = 0.0;
    if (m_lfoShape == Sine)
        val = 0.5 * (1.0 + qSin(2.0 * M_PI * m_lfoPhase));
    else
        val = 1.0 - qAbs(2.0 * m_lfoPhase - 1.0);

    // Advance phase (assume 44100 Hz sample rate internally)
    m_lfoPhase += m_lfoRate / 44100.0;
    if (m_lfoPhase >= 1.0) m_lfoPhase -= 1.0;

    return val;
}

/* ---- All-pass coefficient ---- */

double Phaser5::allPassCoeff(double freq) const
{
    // Map frequency to all-pass coefficient: c = (tan(pi*fc/fs) - 1)/(tan(pi*fc/fs) + 1)
    double w = M_PI * freq / 44100.0;
    double t = qTan(w);
    return (t - 1.0) / (t + 1.0);
}

/* ---- Process single all-pass stage ---- */

double Phaser5::processStage(double input, int stage, double coeff)
{
    double output = coeff * input + m_x1[stage] - coeff * m_y1[stage];
    m_x1[stage] = input;
    m_y1[stage] = output;
    return output;
}

/* ---- Process single sample ---- */

double Phaser5::processOne(double input)
{
    // Add feedback
    double sample = input + m_feedback * m_feedbackSample;

    // LFO modulates the frequency sweep
    double lfo = advanceLfo();
    double sweepFreq = m_baseFreq * (1.0 + m_depth * (lfo * 2.0 - 1.0) * 3.0);
    sweepFreq = qBound(20.0, sweepFreq, 16000.0);

    double coeff = allPassCoeff(sweepFreq);

    // Cascade through all-pass stages
    for (int i = 0; i < m_stages; ++i)
        sample = processStage(sample, i, coeff);

    m_feedbackSample = sample;

    // Mix dry and wet signals
    double output = input * (1.0 - m_depth * 0.5) + sample * m_depth * 0.5;
    return output;
}

/* ---- Process block ---- */

QVector<double> Phaser5::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    output.resize(input.size());

    for (int i = 0; i < input.size(); ++i)
        output[i] = processOne(input[i]);

    m_stats.blockSize = input.size();
    m_stats.numBlocks++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    double lfoVal = m_lfoPhase;
    emit blockProcessed(input.size(), lfoVal, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void Phaser5::reset()
{
    m_x1.fill(0.0);
    m_y1.fill(0.0);
    m_feedbackSample = 0.0;
    m_lfoPhase = 0.0;
}

/* ---- Reset statistics ---- */

void Phaser5::resetStatistics()
{
    reset();
    m_stats = Stats{}; m_timeSum = 0.0;
}

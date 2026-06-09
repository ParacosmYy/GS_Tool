/**
 * @file Expander11.cpp
 * @brief Expander11 实现
 *
 * 实现下行扩展器：程序依赖释放与智能迟滞门控自然瞬态保留。
 */

#include "utils/dsp259/Expander11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Expander11::Expander11(QObject *parent)
    : QObject(parent) {}
Expander11::~Expander11() = default;

/* ---- Configuration ---- */

void Expander11::setThreshold(double thresholdDb) { m_thresholdDb = thresholdDb; }
void Expander11::setRatio(double ratio) { m_ratio = qMax(1.0, ratio); }
void Expander11::setAttack(double attackMs) { m_attackMs = qMax(0.01, attackMs); }
void Expander11::setRelease(double releaseMs) { m_releaseMs = qMax(1.0, releaseMs); }
void Expander11::setHysteresis(double hysteresisDb) { m_hysteresisDb = qMax(0.0, hysteresisDb); }
void Expander11::setSampleRate(double sampleRate) { m_sampleRate = qMax(1.0, sampleRate); }

/* ---- dB / linear conversion ---- */

double Expander11::dbToLinear(double db) const { return qPow(10.0, db / 20.0); }
double Expander11::linearToDb(double linear) const
{
    return (linear > 0.0) ? 20.0 * qLn(linear) / qLn(10.0) : -120.0;
}

/* ---- Program-dependent release ---- */

double Expander11::programRelease(double overshootDb) const
{
    // Release adapts to how far below threshold
    double factor = qBound(0.5, 1.0 + qAbs(overshootDb) / 20.0, 4.0);
    return m_releaseMs * factor;
}

/* ---- Smooth gain ballistics ---- */

double Expander11::smoothGain(double targetDb, double dt)
{
    if (targetDb < m_gainDb) {
        // Attack: gain is decreasing
        double coeff = 1.0 - qExp(-dt / (m_attackMs * 0.001));
        return m_gainDb + (targetDb - m_gainDb) * coeff;
    } else {
        // Release: gain is recovering
        double overshoot = targetDb - m_thresholdDb;
        double releaseMs = programRelease(overshoot);
        double coeff = 1.0 - qExp(-dt / (releaseMs * 0.001));
        return m_gainDb + (targetDb - m_gainDb) * coeff;
    }
}

/* ---- Process single sample ---- */

double Expander11::processSample(double sample)
{
    double dt = 1.0 / m_sampleRate;
    double absSample = qFabs(sample);
    double inputDb = linearToDb(absSample);

    // Update envelope (peak detection)
    if (inputDb > m_envelopeDb) {
        double coeff = 1.0 - qExp(-dt / (m_attackMs * 0.001));
        m_envelopeDb += (inputDb - m_envelopeDb) * coeff;
    } else {
        double releaseMs = programRelease(m_envelopeDb - m_thresholdDb);
        double coeff = 1.0 - qExp(-dt / (releaseMs * 0.001));
        m_envelopeDb += (inputDb - m_envelopeDb) * coeff;
    }

    // Smart gate with hysteresis
    double openThreshold = m_thresholdDb;
    double closeThreshold = m_thresholdDb - m_hysteresisDb;

    if (m_gateOpen) {
        if (m_envelopeDb < closeThreshold) m_gateOpen = false;
    } else {
        if (m_envelopeDb >= openThreshold) m_gateOpen = true;
    }

    // Compute target gain
    double targetGainDb = 0.0;
    if (m_envelopeDb < m_thresholdDb) {
        double belowThreshold = m_thresholdDb - m_envelopeDb;
        double expansion = belowThreshold * (1.0 - 1.0 / m_ratio);
        targetGainDb = -expansion;
    }

    // Smooth gain
    m_gainDb = smoothGain(targetGainDb, dt);
    m_gainLinear = dbToLinear(m_gainDb);

    // Apply gain
    double output = sample * m_gainLinear;

    // Update stats
    m_gainReductionSum += qAbs(m_gainDb);
    m_stats.numFrames++;
    return output;
}

/* ---- Process frame ---- */

void Expander11::processFrame(QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i < frame.size(); ++i)
        frame[i] = processSample(frame[i]);

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_stats.peakGainReduction = qMin(m_stats.peakGainReduction, m_gainDb);
    m_stats.avgGainReduction = m_gainReductionSum / qMax(m_stats.numFrames, 1);
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit frameProcessed(frame.size(), m_gainDb, elapsed);
}

/* ---- State queries ---- */

double Expander11::currentGainDb() const { return m_gainDb; }
double Expander11::currentEnvelopeDb() const { return m_envelopeDb; }
bool Expander11::isGateOpen() const { return m_gateOpen; }

/* ---- Reset ---- */

void Expander11::resetStatistics()
{
    m_gainLinear = 1.0;
    m_gainDb = 0.0;
    m_envelopeDb = -120.0;
    m_gateOpen = false;
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_gainReductionSum = 0.0;
}

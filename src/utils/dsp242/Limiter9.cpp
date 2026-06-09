/**
 * @file Limiter9.cpp
 * @brief Limiter9 实现
 *
 * 实现砖墙限制器：前瞻缓冲与自动增益控制可配置攻击/释放包络。
 */

#include "utils/dsp242/Limiter9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Limiter9::Limiter9(QObject *parent) : QObject(parent) { updateCoefficients(); }
Limiter9::~Limiter9() = default;

/* ---- Configuration ---- */

void Limiter9::setCeiling(double db) { m_ceilingDb = db; m_ceilingLin = dbToLinear(db); }
void Limiter9::setAttackTime(double ms) { m_attackMs = qMax(0.01, ms); updateCoefficients(); }
void Limiter9::setReleaseTime(double ms) { m_releaseMs = qMax(0.01, ms); updateCoefficients(); }
void Limiter9::setSampleRate(double rate) { m_sampleRate = qMax(1.0, rate); updateCoefficients(); }
void Limiter9::setLookaheadTime(double ms)
{
    m_lookaheadMs = qMax(0.0, ms);
    m_lookaheadSize = static_cast<int>(m_lookaheadMs * 0.001 * m_sampleRate);
    m_lookaheadBuf.resize(m_lookaheadSize, 0.0);
    m_bufPos = 0;
}

/* ---- dB / Linear conversion ---- */

double Limiter9::dbToLinear(double db) { return qPow(10.0, db / 20.0); }
double Limiter9::linearToDb(double lin) { return (lin > 0) ? 20.0 * qLn(lin) / qLn(10.0) : -120.0; }

/* ---- Update coefficients ---- */

void Limiter9::updateCoefficients()
{
    // One-pole smoothing: coeff = exp(-1 / (time * sampleRate))
    m_attackCoeff = qExp(-1.0 / (m_attackMs * 0.001 * m_sampleRate));
    m_releaseCoeff = qExp(-1.0 / (m_releaseMs * 0.001 * m_sampleRate));
    m_lookaheadSize = static_cast<int>(m_lookaheadMs * 0.001 * m_sampleRate);
    m_lookaheadBuf.resize(m_lookaheadSize, 0.0);
}

/* ---- Process single sample ---- */

double Limiter9::processOne(double sample)
{
    // Push into lookahead buffer
    double delayed = 0.0;
    if (m_lookaheadSize > 0) {
        delayed = m_lookaheadBuf[m_bufPos];
        m_lookaheadBuf[m_bufPos] = sample;
        m_bufPos = (m_bufPos + 1) % m_lookaheadSize;
    } else {
        delayed = sample;
    }

    // Scan lookahead buffer for peak
    double peak = qAbs(sample);
    for (int i = 0; i < m_lookaheadSize; ++i)
        peak = qMax(peak, qAbs(m_lookaheadBuf[i]));

    // Compute target gain
    double targetGain = (peak > m_ceilingLin) ? m_ceilingLin / peak : 1.0;

    // Smooth gain envelope
    double coeff = (targetGain < m_gain) ? m_attackCoeff : m_releaseCoeff;
    m_gain = coeff * m_gain + (1.0 - coeff) * targetGain;

    // Track stats
    double inputDb = linearToDb(qAbs(sample));
    if (inputDb > m_peakInputDb) m_peakInputDb = inputDb;

    if (qAbs(sample) > m_ceilingLin)
        emit clippingDetected(0, inputDb);

    return delayed * m_gain;
}

/* ---- Process buffer ---- */

QVector<double> Limiter9::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    double peakBefore = 0.0;
    for (double s : input) peakBefore = qMax(peakBefore, qAbs(s));

    QVector<double> output;
    output.resize(input.size());
    for (int i = 0; i < input.size(); ++i)
        output[i] = processOne(input[i]);

    double peakAfter = 0.0;
    for (double s : output) peakAfter = qMax(peakAfter, qAbs(s));

    double sumGain = 0.0;
    for (int i = 0; i < input.size() && i < output.size(); ++i) {
        if (qAbs(input[i]) > 1e-10)
            sumGain += linearToDb(qAbs(output[i]) / qAbs(input[i]));
    }

    m_stats.bufferSize = input.size();
    m_stats.peakReductionDb = linearToDb(peakBefore) - linearToDb(peakAfter);
    m_stats.avgGainDb = (input.size() > 0) ? sumGain / input.size() : 0.0;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processingCompleted(input.size(), m_stats.peakReductionDb, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void Limiter9::reset()
{
    m_lookaheadBuf.fill(0.0);
    m_bufPos = 0;
    m_gain = 1.0;
    m_peakInputDb = -120.0;
}

void Limiter9::resetStatistics()
{
    reset();
    m_stats = Stats{}; m_timeSum = 0.0;
}

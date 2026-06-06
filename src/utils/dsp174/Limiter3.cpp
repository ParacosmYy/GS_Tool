/**
 * @file Limiter3.cpp
 * @brief Limiter3 实现
 *
 * 实现砖墙限幅器：前视延迟缓冲、真峰值检测、增益平滑。
 */

#include "utils/dsp174/Limiter3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Limiter3::Limiter3(QObject *parent)
    : QObject(parent)
{
    m_thresholdLin = dbToLinear(m_thresholdDb);
    m_ceilingLin = dbToLinear(m_ceilingDb);
    m_attackCoeff = (m_attackMs > 0.0) ? qExp(-1.0 / (m_attackMs * m_sampleRate * 0.001)) : 0.0;
    m_releaseCoeff = (m_releaseMs > 0.0) ? qExp(-1.0 / (m_releaseMs * m_sampleRate * 0.001)) : 0.0;
    m_delayBufferL.resize(m_lookahead, 0.0);
    m_delayBufferR.resize(m_lookahead, 0.0);
}

Limiter3::~Limiter3() = default;

/* ---- Configuration ---- */

void Limiter3::setThreshold(double db)
{
    m_thresholdDb = db;
    m_thresholdLin = dbToLinear(db);
}

void Limiter3::setCeiling(double db)
{
    m_ceilingDb = db;
    m_ceilingLin = dbToLinear(db);
}

void Limiter3::setAttack(double ms)
{
    m_attackMs = qMax(0.01, ms);
    m_attackCoeff = qExp(-1.0 / (m_attackMs * m_sampleRate * 0.001));
}

void Limiter3::setRelease(double ms)
{
    m_releaseMs = qMax(0.1, ms);
    m_releaseCoeff = qExp(-1.0 / (m_releaseMs * m_sampleRate * 0.001));
}

void Limiter3::setLookahead(int samples)
{
    m_lookahead = qMax(0, samples);
    m_delayBufferL.resize(m_lookahead, 0.0);
    m_delayBufferR.resize(m_lookahead, 0.0);
    m_delayPos = 0;
}

void Limiter3::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
    /* Recalculate coefficients */
    setAttack(m_attackMs);
    setRelease(m_releaseMs);
}

/* ---- dB / Linear conversions ---- */

double Limiter3::dbToLinear(double db) { return qPow(10.0, db * 0.05); }
double Limiter3::linearToDb(double lin) { return 20.0 * qLog10(qMax(lin, 1e-30)); }

/* ---- Gain reduction ---- */

double Limiter3::computeGainReduction(double peak) const
{
    if (peak <= m_thresholdLin) return 1.0;
    /* Reduce so that peak * gain = threshold */
    return m_thresholdLin / qMax(peak, 1e-30);
}

/* ---- Smooth gain (envelope follower) ---- */

void Limiter3::smoothGain(double& currentGain, double targetGain, int /*samples*/)
{
    if (targetGain < currentGain) {
        /* Attack: reduce gain quickly */
        currentGain = m_attackCoeff * currentGain + (1.0 - m_attackCoeff) * targetGain;
    } else {
        /* Release: restore gain slowly */
        currentGain = m_releaseCoeff * currentGain + (1.0 - m_releaseCoeff) * targetGain;
    }
    /* Clamp gain so output never exceeds ceiling */
    currentGain = qMin(currentGain, m_ceilingLin);
}

/* ---- True peak detection (4x oversampling) ---- */

double Limiter3::truePeak(const QVector<double>& buffer, int idx, int size) const
{
    double peak = 0.0;
    /* Check 4x interpolated positions */
    for (int sub = 0; sub < 4; ++sub) {
        double t = sub * 0.25;
        int i0 = idx - 1;
        int i1 = idx;
        int i2 = idx + 1;
        int i3 = idx + 2;
        /* Clamp indices */
        i0 = qBound(0, i0, size - 1);
        i1 = qBound(0, i1, size - 1);
        i2 = qBound(0, i2, size - 1);
        i3 = qBound(0, i3, size - 1);

        /* Cubic interpolation (Catmull-Rom) */
        double y0 = buffer[i0], y1 = buffer[i1];
        double y2 = buffer[i2], y3 = buffer[i3];
        double t2 = t * t, t3 = t2 * t;
        double val = 0.5 * ((2.0 * y1) + (-y0 + y2) * t +
                    (2.0 * y0 - 5.0 * y1 + 4.0 * y2 - y3) * t2 +
                    (-y0 + 3.0 * y1 - 3.0 * y2 + y3) * t3);
        peak = qMax(peak, qAbs(val));
    }
    return peak;
}

/* ---- Process mono ---- */

QVector<double> Limiter3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n, 0.0);
    double peakReduction = 0.0;

    for (int i = 0; i < n; ++i) {
        /* Write into lookahead delay buffer */
        m_delayBufferL[m_delayPos] = input[i];

        /* Lookahead: find peak in the upcoming buffer */
        double maxPeak = 0.0;
        for (int j = 0; j < m_lookahead; ++j) {
            int readPos = (m_delayPos + 1 + j) % m_lookahead;
            maxPeak = qMax(maxPeak, qAbs(m_delayBufferL[readPos]));
        }

        /* True peak detection on the delayed sample */
        int readIdx = (m_delayPos + 1) % m_lookahead;
        double tp = truePeak(m_delayBufferL, readIdx, m_lookahead);
        maxPeak = qMax(maxPeak, tp);

        /* Compute target gain */
        double targetGain = computeGainReduction(maxPeak);
        smoothGain(m_gainSmooth, targetGain, 1);

        /* Read delayed sample and apply gain */
        double delayed = m_delayBufferL[(m_delayPos + 1) % m_lookahead];
        output[i] = delayed * m_gainSmooth;

        double reduction = linearToDb(m_gainSmooth);
        peakReduction = qMin(peakReduction, reduction);

        m_delayPos = (m_delayPos + 1) % m_lookahead;
    }

    m_stats.totalSamples += n;
    m_stats.peakReductionDb = peakReduction;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1ULL, m_stats.totalSamples / qMax(1ULL, n > 0 ? n : 1));

    emit processingCompleted(n, peakReduction);
    return output;
}

/* ---- Process stereo (interleaved L/R) ---- */

QVector<double> Limiter3::processStereo(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n, 0.0);
    double peakReduction = 0.0;

    for (int i = 0; i < n; i += 2) {
        double left = (i < n) ? input[i] : 0.0;
        double right = (i + 1 < n) ? input[i + 1] : 0.0;

        m_delayBufferL[m_delayPos] = left;
        m_delayBufferR[m_delayPos] = right;

        /* Find peak across both channels in lookahead */
        double maxPeak = 0.0;
        for (int j = 0; j < m_lookahead; ++j) {
            int rp = (m_delayPos + 1 + j) % m_lookahead;
            maxPeak = qMax(maxPeak, qAbs(m_delayBufferL[rp]));
            maxPeak = qMax(maxPeak, qAbs(m_delayBufferR[rp]));
        }

        double targetGain = computeGainReduction(maxPeak);
        smoothGain(m_gainSmooth, targetGain, 1);

        int rp = (m_delayPos + 1) % m_lookahead;
        output[i] = m_delayBufferL[rp] * m_gainSmooth;
        if (i + 1 < n) output[i + 1] = m_delayBufferR[rp] * m_gainSmooth;

        peakReduction = qMin(peakReduction, linearToDb(m_gainSmooth));
        m_delayPos = (m_delayPos + 1) % m_lookahead;
    }

    m_stats.totalSamples += n;
    m_stats.peakReductionDb = peakReduction;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1ULL, m_stats.totalSamples / qMax(1ULL, n > 0 ? n : 1));

    emit processingCompleted(n, peakReduction);
    return output;
}

/* ---- Reset ---- */

void Limiter3::reset()
{
    m_delayBufferL.fill(0.0);
    m_delayBufferR.fill(0.0);
    m_delayPos = 0;
    m_gainSmooth = 1.0;
}

/* ---- Statistics ---- */

void Limiter3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file Limiter11.cpp
 * @brief Limiter11 实现
 *
 * 实现限幅器：前瞻缓冲与真峰值采样点间插值广播安全电平控制。
 */

#include "utils/dsp270/Limiter11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Limiter11::Limiter11(QObject *parent)
    : QObject(parent)
{
    initBuffer();
}

Limiter11::~Limiter11() = default;

/* ---- Configuration ---- */

void Limiter11::setCeiling(double ceilingDb)
{
    m_ceilingDb = qBound(-12.0, ceilingDb, 0.0);
}

void Limiter11::setLookaheadMs(double ms)
{
    m_lookaheadMs = qBound(0.0, ms, 50.0);
    initBuffer();
}

void Limiter11::setSampleRate(double rate)
{
    m_sampleRate = qBound(8000.0, rate, 192000.0);
    initBuffer();
}

void Limiter11::setReleaseMs(double ms)
{
    m_releaseMs = qBound(1.0, ms, 5000.0);
}

/* ---- Buffer init ---- */

void Limiter11::initBuffer()
{
    m_lookaheadSize = qMax(1, static_cast<int>(m_lookaheadMs * m_sampleRate / 1000.0));
    m_lookaheadBuf.resize(m_lookaheadSize);
    m_lookaheadBuf.fill(0.0);
    m_bufPos = 0;
}

/* ---- True-peak estimation via 4x sinc interpolation ---- */

double Limiter11::estimateTruePeak(double x0, double xm1, double xp1, double xp2) const
{
    // Simplified inter-sample peak detection using quadratic interpolation
    // For each pair of adjacent samples, check 3 intermediate points
    double maxVal = qAbs(x0);

    // Quadratic interpolation: peak at t = (xm1 - xp1) / (2*(xm1 - 2*x0 + xp1))
    double denom = 2.0 * (xm1 - 2.0 * x0 + xp1);
    if (qAbs(denom) > 1e-10) {
        double t = (xm1 - xp1) / denom;
        t = qBound(0.0, t, 1.0);
        double interpPeak = x0 + 0.5 * t * (xp1 - xm1 + t * denom);
        maxVal = qMax(maxVal, qAbs(interpPeak));
    }

    // Check point between x0 and xp1
    denom = 2.0 * (x0 - 2.0 * xp1 + xp2);
    if (qAbs(denom) > 1e-10) {
        double t = (x0 - xp2) / denom;
        t = qBound(0.0, t, 1.0);
        double interpPeak = xp1 + 0.5 * t * (xp2 - x0 + t * denom);
        maxVal = qMax(maxVal, qAbs(interpPeak));
    }

    return maxVal;
}

/* ---- Gain computation ---- */

double Limiter11::computeGain(double peakLinear) const
{
    double ceilingLinear = qPow(10.0, m_ceilingDb / 20.0);
    if (peakLinear <= ceilingLinear) return 1.0;
    return ceilingLinear / peakLinear;
}

/* ---- Main processing ---- */

QVector<double> Limiter11::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) return input;

    QVector<double> output(n, 0.0);
    double ceilingLin = qPow(10.0, m_ceilingDb / 20.0);
    double releaseCoeff = qExp(-1.0 / (m_releaseMs * m_sampleRate / 1000.0));

    m_truePeakDb = -120.0;
    m_maxGainReductionDb = 0.0;
    m_gain = 1.0;

    for (int i = 0; i < n; ++i) {
        // Get surrounding samples for true-peak estimation
        double xm1 = (i > 0) ? input[i - 1] : 0.0;
        double x0 = input[i];
        double xp1 = (i + 1 < n) ? input[i + 1] : 0.0;
        double xp2 = (i + 2 < n) ? input[i + 2] : 0.0;

        // Estimate true-peak
        double tp = estimateTruePeak(x0, xm1, xp1, xp2);

        // Track maximum true-peak in dBTP
        double tpDb = 20.0 * qLn(qMax(tp, 1e-10)) / qLn(10.0);
        if (tpDb > m_truePeakDb) m_truePeakDb = tpDb;

        // Compute target gain for this sample
        double targetGain = computeGain(tp);

        // Smooth gain: instant attack, exponential release
        if (targetGain < m_gain) {
            // Attack: instant
            m_gain = targetGain;
        } else {
            // Release: exponential
            m_gain = targetGain + (m_gain - targetGain) * releaseCoeff;
        }

        // Track gain reduction
        double grDb = 20.0 * qLn(qMax(m_gain, 1e-10) / qLn(10.0));
        if (grDb < m_maxGainReductionDb)
            m_maxGainReductionDb = grDb;

        // Lookahead: store in circular buffer, output delayed sample
        double delayedSample = m_lookaheadBuf[m_bufPos];
        m_lookaheadBuf[m_bufPos] = x0 * m_gain;
        m_bufPos = (m_bufPos + 1) % m_lookaheadSize;

        output[i] = delayedSample;
    }

    // Flush remaining lookahead buffer
    QVector<double> tail;
    for (int k = 0; k < m_lookaheadSize; ++k) {
        int idx = (m_bufPos + k) % m_lookaheadSize;
        tail.append(m_lookaheadBuf[idx]);
    }

    // Append tail to output
    for (int k = 0; k < tail.size() && n + k < n + m_lookaheadSize; ++k)
        output.append(tail[k]);

    double elapsed = timer.elapsed();
    m_stats.numSamples = n;
    m_stats.ceiling = m_ceilingDb;
    m_stats.truePeakDb = m_truePeakDb;
    m_stats.gainReductionDb = m_maxGainReductionDb;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit limitingApplied(m_maxGainReductionDb, m_truePeakDb, elapsed);

    return output;
}

/* ---- Accessors ---- */

double Limiter11::truePeakDb() const { return m_truePeakDb; }
double Limiter11::maxGainReductionDb() const { return m_maxGainReductionDb; }

/* ---- Reset ---- */

void Limiter11::resetStatistics()
{
    m_lookaheadBuf.clear();
    initBuffer();
    m_gain = 1.0;
    m_truePeakDb = -120.0;
    m_maxGainReductionDb = 0.0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

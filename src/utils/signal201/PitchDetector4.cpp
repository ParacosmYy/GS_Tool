/**
 * @file PitchDetector4.cpp
 * @brief PitchDetector4 实现
 *
 * 实现YIN基频检测：累积均值归一化差分函数、绝对阈值、抛物线插值。
 */

#include "utils/signal201/PitchDetector4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

PitchDetector4::PitchDetector4(QObject *parent) : QObject(parent) {}
PitchDetector4::~PitchDetector4() = default;

/* ---- Configuration ---- */

void PitchDetector4::setSampleRate(double rate) { m_sampleRate = qMax(1.0, rate); }
void PitchDetector4::setMinFrequency(double f) { m_minFreq = qMax(20.0, f); }
void PitchDetector4::setMaxFrequency(double f) { m_maxFreq = qMax(m_minFreq + 1.0, f); }
void PitchDetector4::setThreshold(double t) { m_threshold = qBound(0.01, t, 1.0); }

/* ---- Tau bounds ---- */

int PitchDetector4::tauMin() const
{
    return static_cast<int>(m_sampleRate / m_maxFreq) + 1;
}

int PitchDetector4::tauMax(int signalLen) const
{
    int tMax = static_cast<int>(m_sampleRate / m_minFreq);
    return qMin(tMax, signalLen / 2);
}

/* ---- Difference function ---- */

QVector<double> PitchDetector4::differenceFunction(const QVector<double>& signal, int tauMax) const
{
    int n = signal.size();
    int tMax = qMin(tauMax, n / 2);
    QVector<double> d(tMax + 1, 0.0);

    for (int tau = 0; tau <= tMax; ++tau) {
        double sum = 0.0;
        for (int j = 0; j < n - tau; ++j) {
            double diff = signal[j] - signal[j + tau];
            sum += diff * diff;
        }
        d[tau] = sum;
    }
    return d;
}

/* ---- Cumulative mean normalized difference ---- */

QVector<double> PitchDetector4::cumulativeMeanNormalized(const QVector<double>& diff) const
{
    int n = diff.size();
    QVector<double> cmndf(n, 1.0);
    cmndf[0] = 1.0;

    double cumSum = 0.0;
    for (int tau = 1; tau < n; ++tau) {
        cumSum += diff[tau];
        if (cumSum > 1e-12)
            cmndf[tau] = diff[tau] * static_cast<double>(tau) / cumSum;
        else
            cmndf[tau] = 1.0;
    }
    return cmndf;
}

/* ---- Absolute threshold ---- */

int PitchDetector4::absoluteThreshold(const QVector<double>& cmndf, double threshold) const
{
    int n = cmndf.size();
    int tMin = tauMin();

    // Find first tau below threshold
    for (int tau = tMin; tau < n - 1; ++tau) {
        if (cmndf[tau] < threshold) {
            // Find local minimum below threshold
            while (tau + 1 < n && cmndf[tau + 1] < cmndf[tau])
                tau++;
            return tau;
        }
    }

    // No dip below threshold: find global minimum
    int bestTau = tMin;
    double bestVal = cmndf[tMin];
    for (int tau = tMin + 1; tau < n; ++tau) {
        if (cmndf[tau] < bestVal) { bestVal = cmndf[tau]; bestTau = tau; }
    }
    return bestTau;
}

/* ---- Parabolic interpolation ---- */

double PitchDetector4::parabolicInterpolation(const QVector<double>& cmndf, int tau) const
{
    if (tau <= 0 || tau >= cmndf.size() - 1) return static_cast<double>(tau);

    double s0 = cmndf[tau - 1];
    double s1 = cmndf[tau];
    double s2 = cmndf[tau + 1];

    double denom = 2.0 * (2.0 * s1 - s0 - s2);
    if (qAbs(denom) < 1e-12) return static_cast<double>(tau);

    double offset = (s0 - s2) / denom;
    // Clamp offset to reasonable range
    return static_cast<double>(tau) + qBound(-0.5, offset, 0.5);
}

/* ---- Detect ---- */

PitchDetector4::PitchResult PitchDetector4::detect(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    PitchResult result;
    int n = signal.size();
    if (n < 64) {
        m_stats.totalDetections++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;
        return result;
    }

    int tMax = tauMax(n);
    auto diff = differenceFunction(signal, tMax);
    auto cmndf = cumulativeMeanNormalized(diff);

    int tau = absoluteThreshold(cmndf, m_threshold);
    double refinedTau = parabolicInterpolation(cmndf, tau);

    if (refinedTau > 0.0) {
        result.period = refinedTau;
        result.frequency = m_sampleRate / refinedTau;
        result.confidence = 1.0 - cmndf[qBound(0, tau, cmndf.size() - 1)];
        result.voiced = result.confidence > 0.5 && result.frequency >= m_minFreq;
    }

    m_stats.totalDetections++;
    m_stats.lastPitch = result.frequency;
    m_stats.lastConfidence = result.confidence;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit pitchDetected(result.frequency, result.confidence, timer.elapsed());
    return result;
}

/* ---- Detect frames ---- */

QVector<PitchDetector4::PitchResult> PitchDetector4::detectFrames(
    const QVector<double>& signal, int frameSize, int hopSize)
{
    QVector<PitchResult> results;
    int n = signal.size();
    frameSize = qMax(64, frameSize);
    hopSize = qMax(1, hopSize);

    for (int start = 0; start + frameSize <= n; start += hopSize) {
        QVector<double> frame(frameSize);
        for (int i = 0; i < frameSize; ++i)
            frame[i] = signal[start + i];
        results.append(detect(frame));
    }
    return results;
}

/* ---- Reset ---- */

void PitchDetector4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file PitchDetector9.cpp
 * @brief PitchDetector9 实现
 *
 * 实现基频检测器：自相关YIN算法与累积均值归一化差分鲁棒F0估计。
 */

#include "utils/signal273/PitchDetector9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

PitchDetector9::PitchDetector9(QObject *parent)
    : QObject(parent) {}

PitchDetector9::~PitchDetector9() = default;

/* ---- Configuration ---- */

void PitchDetector9::setSampleRate(double rate)
{
    m_sampleRate = qBound(8000.0, rate, 192000.0);
}

void PitchDetector9::setMinFrequency(double hz)
{
    m_minFreq = qBound(20.0, hz, 5000.0);
}

void PitchDetector9::setMaxFrequency(double hz)
{
    m_maxFreq = qBound(100.0, hz, 10000.0);
}

void PitchDetector9::setThreshold(double threshold)
{
    m_threshold = qBound(0.01, threshold, 1.0);
}

/* ---- YIN difference function ---- */

QVector<double> PitchDetector9::computeDifference(const QVector<double>& x,
                                                    int maxTau) const
{
    int n = x.size();
    int W = n / 2;  // integration window
    QVector<double> d(maxTau + 1, 0.0);

    // D(tau) = sum_{j=0}^{W-1} (x[j] - x[j+tau])^2
    for (int tau = 0; tau <= maxTau; ++tau) {
        double sum = 0.0;
        for (int j = 0; j < W && j + tau < n; ++j) {
            double diff = x[j] - x[j + tau];
            sum += diff * diff;
        }
        d[tau] = sum;
    }
    return d;
}

/* ---- Cumulative mean normalized difference ---- */

QVector<double> PitchDetector9::cumulativeMeanNorm(const QVector<double>& diff) const
{
    int n = diff.size();
    QVector<double> cmnd(n);
    cmnd[0] = 1.0;

    double cumSum = 0.0;
    for (int tau = 1; tau < n; ++tau) {
        cumSum += diff[tau];
        cmnd[tau] = (cumSum > 1e-30) ? diff[tau] * tau / cumSum : 1.0;
    }
    return cmnd;
}

/* ---- Find absolute minimum with threshold ---- */

int PitchDetector9::findAbsMin(const QVector<double>& cmnd) const
{
    int minTau = static_cast<int>(m_sampleRate / m_maxFreq);
    int maxTau = qMin(static_cast<int>(m_sampleRate / m_minFreq), cmnd.size() - 1);

    // Step 1: Find first tau where cmnd < threshold
    int firstBelow = -1;
    for (int tau = minTau; tau <= maxTau; ++tau) {
        if (cmnd[tau] < m_threshold) {
            firstBelow = tau;
            break;
        }
    }

    if (firstBelow < 0) {
        // No value below threshold; find global minimum
        double bestVal = std::numeric_limits<double>::max();
        for (int tau = minTau; tau <= maxTau; ++tau) {
            if (cmnd[tau] < bestVal) {
                bestVal = cmnd[tau];
                firstBelow = tau;
            }
        }
    }

    // Step 2: Find minimum within local neighborhood
    if (firstBelow >= 0) {
        double bestVal = cmnd[firstBelow];
        int bestTau = firstBelow;
        for (int tau = firstBelow; tau <= maxTau; ++tau) {
            if (cmnd[tau] < m_threshold) {
                if (cmnd[tau] < bestVal) {
                    bestVal = cmnd[tau];
                    bestTau = tau;
                }
            } else {
                // Stop searching once above threshold again
                break;
            }
        }
        return bestTau;
    }

    return -1;
}

/* ---- Parabolic interpolation ---- */

double PitchDetector9::parabolicInterpolation(const QVector<double>& cmnd,
                                                int tau) const
{
    if (tau <= 0 || tau >= cmnd.size() - 1) return static_cast<double>(tau);

    double s0 = cmnd[tau - 1];
    double s1 = cmnd[tau];
    double s2 = cmnd[tau + 1];

    double denom = 2.0 * (2.0 * s1 - s0 - s2);
    if (qAbs(denom) < 1e-30) return static_cast<double>(tau);

    double shift = (s0 - s2) / denom;
    return static_cast<double>(tau) + qBound(-0.5, shift, 0.5);
}

/* ---- Detect pitch ---- */

double PitchDetector9::detect(const QVector<double>& samples)
{
    auto [freq, conf] = detectWithConfidence(samples);
    return freq;
}

/* ---- Detect pitch with confidence ---- */

QPair<double, double> PitchDetector9::detectWithConfidence(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    int n = samples.size();
    if (n < 64) return {0.0, 0.0};

    int maxTau = qMin(n / 2, static_cast<int>(m_sampleRate / m_minFreq));

    // Step 1: Compute difference function
    QVector<double> diff = computeDifference(samples, maxTau);

    // Step 2: Cumulative mean normalization
    m_cmnDiff = cumulativeMeanNorm(diff);

    // Step 3: Find absolute minimum
    int tau = findAbsMin(m_cmnDiff);

    double freqHz = 0.0;
    double confidence = 0.0;

    if (tau > 0) {
        // Step 4: Parabolic interpolation for sub-sample precision
        double refinedTau = parabolicInterpolation(m_cmnDiff, tau);

        // Convert period to frequency
        freqHz = m_sampleRate / refinedTau;

        // Confidence: 1 - cmnd value (higher is more confident)
        confidence = 1.0 - qBound(0.0, m_cmnDiff[tau], 1.0);

        // Validate frequency range
        if (freqHz < m_minFreq || freqHz > m_maxFreq) {
            freqHz = 0.0;
            confidence = 0.0;
        }
    }

    double elapsed = timer.elapsed();
    m_stats.blockSize = n;
    m_stats.lastPitchHz = freqHz;
    m_stats.lastConfidence = confidence;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit pitchDetected(freqHz, confidence, elapsed);

    return {freqHz, confidence};
}

/* ---- Difference function accessor ---- */

QVector<double> PitchDetector9::differenceFunction() const
{
    return m_cmnDiff;
}

/* ---- Reset ---- */

void PitchDetector9::resetStatistics()
{
    m_cmnDiff.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

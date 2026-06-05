/**
 * @file YinPitchDetector.cpp
 * @brief YIN基频检测实现
 */

#include "YinPitchDetector.h"
#include <QElapsedTimer>
#include <cmath>

YinPitchDetector::YinPitchDetector(int bufferSize, QObject* parent)
    : QObject(parent)
    , m_bufferSize(bufferSize > 0 ? bufferSize : 2048)
    , m_minFreq(50.0)
    , m_maxFreq(2000.0)
    , m_threshold(0.3)
    , m_timeSum(0.0)
{
}

YinPitchDetector::PitchResult YinPitchDetector::detect(
    const QVector<double>& signal, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    PitchResult result;
    result.frequency = 0.0;
    result.confidence = 0.0;
    result.isVoiced = false;

    int N = qMin(signal.size(), m_bufferSize) / 2;
    if (N < 2) {
        m_stats.totalDetections++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;
        return result;
    }

    /* Step 1: 差分函数 */
    QVector<double> diff(N, 0.0);
    for (int tau = 0; tau < N; ++tau) {
        for (int i = 0; i < N; ++i) {
            double d = signal[i] - signal[i + tau];
            diff[tau] += d * d;
        }
    }

    /* Step 2: 累积均值归一化差分函数(CMNDF) */
    QVector<double> cmndf(N, 0.0);
    cmndf[0] = 1.0;
    double runningSum = 0.0;
    for (int tau = 1; tau < N; ++tau) {
        runningSum += diff[tau];
        cmndf[tau] = (runningSum > 1e-10) ? diff[tau] * tau / runningSum : 1.0;
    }

    /* Step 3: 绝对阈值找第一个谷 */
    int minTau = qMax(1, static_cast<int>(sampleRate / m_maxFreq));
    int maxTau = qMin(N - 1, static_cast<int>(sampleRate / m_minFreq));
    int bestTau = -1;

    for (int tau = minTau; tau <= maxTau; ++tau) {
        if (cmndf[tau] < m_threshold) {
            /* 找局部最小值 */
            while (tau + 1 <= maxTau && cmndf[tau + 1] < cmndf[tau])
                tau++;
            bestTau = tau;
            break;
        }
    }

    /* Step 4: 抛物线插值 */
    if (bestTau > 0) {
        double betterTau = bestTau;
        if (bestTau > 0 && bestTau < N - 1) {
            double s0 = cmndf[bestTau - 1];
            double s1 = cmndf[bestTau];
            double s2 = cmndf[bestTau + 1];
            double shift = (s0 - s2) / (2.0 * (s0 - 2.0 * s1 + s2));
            if (std::abs(shift) < 1.0) betterTau = bestTau + shift;
        }

        result.frequency = sampleRate / betterTau;
        result.confidence = 1.0 - cmndf[bestTau];
        result.isVoiced = true;
    }

    m_stats.totalDetections++;
    if (result.isVoiced) m_stats.totalVoiced++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit pitchDetected(result.frequency, result.confidence);
    return result;
}

void YinPitchDetector::setFrequencyRange(double minFreq, double maxFreq)
{
    m_minFreq = (minFreq > 0) ? minFreq : 50.0;
    m_maxFreq = (maxFreq > m_minFreq) ? maxFreq : 2000.0;
}

void YinPitchDetector::setConfidenceThreshold(double threshold)
{
    m_threshold = qBound(0.0, threshold, 1.0);
}

void YinPitchDetector::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

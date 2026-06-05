/**
 * @file PitchDetector.cpp
 * @brief 基频检测器实现 — 自相关/YIN/AMDF
 */

#include "utils/pitch/PitchDetector.h"

#include <QtMath>
#include <QElapsedTimer>
#include <algorithm>

PitchDetector::PitchDetector(QObject* parent)
    : QObject(parent), m_sampleRate(48000.0), m_timeSum(0.0) {}

void PitchDetector::setSampleRate(double rate) { m_sampleRate = qMax(1.0, rate); }

PitchDetector::Result PitchDetector::detect(const QVector<double>& data,
                                             Method method)
{
    QElapsedTimer timer;
    timer.start();

    Result result;
    switch (method) {
    case Method::Autocorrelation: result = detectAutocorrelation(data); break;
    case Method::Yin:             result = detectYin(data); break;
    case Method::Amdf:            result = detectAmdf(data); break;
    }

    m_stats.totalDetections++;
    m_stats.totalFramesProcessed++;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit detectionCompleted(result.frequency, result.confidence);
    return result;
}

PitchDetector::Result PitchDetector::detectAutocorrelation(const QVector<double>& data)
{
    Result result;
    int N = data.size();
    if (N < 64) return result;

    int minLag = static_cast<int>(m_sampleRate / 800.0);
    int maxLag = qMin(N / 2, static_cast<int>(m_sampleRate / 50.0));
    minLag = qMax(2, minLag);

    double bestCorr = 0.0;
    int bestLag = 0;
    double energy = 0.0;
    for (int i = 0; i < N; ++i) energy += data[i] * data[i];

    if (energy < 1e-10) return result;

    for (int lag = minLag; lag <= maxLag; ++lag) {
        double corr = 0.0;
        for (int i = 0; i < N - lag; ++i) {
            corr += data[i] * data[i + lag];
        }
        if (corr > bestCorr) { bestCorr = corr; bestLag = lag; }
    }

    if (bestLag > 0) {
        result.frequency = m_sampleRate / bestLag;
        result.confidence = bestCorr / energy;
        result.voiced = result.confidence > 0.3;
    }
    return result;
}

PitchDetector::Result PitchDetector::detectYin(const QVector<double>& data)
{
    Result result;
    int N = data.size();
    int halfN = N / 2;
    if (halfN < 32) return result;

    int minLag = static_cast<int>(m_sampleRate / 800.0);
    int maxLag = qMin(halfN, static_cast<int>(m_sampleRate / 50.0));
    minLag = qMax(2, minLag);

    /* 差分函数 */
    QVector<double> diff(maxLag + 1, 0.0);
    for (int lag = minLag; lag <= maxLag; ++lag) {
        for (int i = 0; i < halfN; ++i) {
            double d = data[i] - data[i + lag];
            diff[lag] += d * d;
        }
    }

    /* 累积均值归一化 */
    QVector<double> cmndf(maxLag + 1, 1.0);
    double runningSum = 0.0;
    for (int lag = minLag; lag <= maxLag; ++lag) {
        runningSum += diff[lag];
        cmndf[lag] = (runningSum > 0) ? diff[lag] * lag / runningSum : 1.0;
    }

    /* 找最小值 */
    double threshold = 0.15;
    int bestLag = minLag;
    double bestVal = cmndf[minLag];
    for (int lag = minLag; lag <= maxLag; ++lag) {
        if (cmndf[lag] < bestVal) { bestVal = cmndf[lag]; bestLag = lag; }
    }

    if (bestVal < threshold && bestLag > 0) {
        result.frequency = m_sampleRate / bestLag;
        result.confidence = 1.0 - bestVal;
        result.voiced = true;
    }
    return result;
}

PitchDetector::Result PitchDetector::detectAmdf(const QVector<double>& data)
{
    Result result;
    int N = data.size();
    if (N < 64) return result;

    int minLag = static_cast<int>(m_sampleRate / 800.0);
    int maxLag = qMin(N / 2, static_cast<int>(m_sampleRate / 50.0));
    minLag = qMax(2, minLag);

    double bestDiff = 1e30;
    int bestLag = minLag;
    double totalEnergy = 0.0;
    for (int i = 0; i < N; ++i) totalEnergy += qAbs(data[i]);

    for (int lag = minLag; lag <= maxLag; ++lag) {
        double diff = 0.0;
        for (int i = 0; i < N - lag; ++i) {
            diff += qAbs(data[i] - data[i + lag]);
        }
        diff /= (N - lag);
        if (diff < bestDiff) { bestDiff = diff; bestLag = lag; }
    }

    if (bestLag > 0 && totalEnergy > 0) {
        result.frequency = m_sampleRate / bestLag;
        result.confidence = 1.0 - bestDiff / (totalEnergy / N);
        result.voiced = result.confidence > 0.3;
    }
    return result;
}

void PitchDetector::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }

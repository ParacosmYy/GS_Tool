/**
 * @file SnrEstimator.cpp
 * @brief 信噪比估计器实现
 */

#include "utils/snr/SnrEstimator.h"

#include <QtMath>
#include <QElapsedTimer>
#include <algorithm>

SnrEstimator::SnrEstimator(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

SnrEstimator::Result SnrEstimator::estimateKnown(
    const QVector<double>& signal, const QVector<double>& noisy)
{
    Result r;
    int n = qMin(signal.size(), noisy.size());
    if (n == 0) return r;

    QElapsedTimer timer;
    timer.start();

    double sigPow = 0.0, noisePow = 0.0;
    for (int i = 0; i < n; ++i) {
        sigPow += signal[i] * signal[i];
        double noise = noisy[i] - signal[i];
        noisePow += noise * noise;
    }
    sigPow /= n;
    noisePow /= n;

    r.signalPower = sigPow;
    r.noisePower = noisePow;
    r.snrLinear = (noisePow > 1e-15) ? sigPow / noisePow : 1e15;
    r.snrDb = 10.0 * std::log10(qMax(r.snrLinear, 1e-15));

    m_stats.totalEstimations++;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalEstimations;

    emit estimationCompleted(r.snrDb);
    return r;
}

SnrEstimator::Result SnrEstimator::estimateSegmented(
    const QVector<double>& signalSegment, const QVector<double>& noiseSegment)
{
    Result r;
    if (signalSegment.isEmpty() || noiseSegment.isEmpty()) return r;

    QElapsedTimer timer;
    timer.start();

    double sigPow = 0.0;
    for (double v : signalSegment) sigPow += v * v;
    sigPow /= signalSegment.size();

    double noisePow = 0.0;
    for (double v : noiseSegment) noisePow += v * v;
    noisePow /= noiseSegment.size();

    r.signalPower = sigPow;
    r.noisePower = noisePow;
    r.snrLinear = (noisePow > 1e-15) ? sigPow / noisePow : 1e15;
    r.snrDb = 10.0 * std::log10(qMax(r.snrLinear, 1e-15));

    m_stats.totalEstimations++;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalEstimations;

    emit estimationCompleted(r.snrDb);
    return r;
}

SnrEstimator::Result SnrEstimator::estimateMinStat(
    const QVector<double>& data, int noiseEstLen)
{
    Result r;
    if (data.size() < noiseEstLen) return r;

    QElapsedTimer timer;
    timer.start();

    double totalPow = 0.0;
    for (double v : data) totalPow += v * v;
    totalPow /= data.size();

    QVector<double> sortedPow;
    sortedPow.reserve(data.size() - noiseEstLen + 1);
    for (int i = 0; i <= data.size() - noiseEstLen; ++i) {
        double segPow = 0.0;
        for (int j = i; j < i + noiseEstLen; ++j) segPow += data[j] * data[j];
        sortedPow.append(segPow / noiseEstLen);
    }
    std::sort(sortedPow.begin(), sortedPow.end());

    double noisePow = sortedPow[0];
    double sigPow = qMax(totalPow - noisePow, 0.0);

    r.signalPower = sigPow;
    r.noisePower = noisePow;
    r.snrLinear = (noisePow > 1e-15) ? sigPow / noisePow : 1e15;
    r.snrDb = 10.0 * std::log10(qMax(r.snrLinear, 1e-15));

    m_stats.totalEstimations++;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalEstimations;

    emit estimationCompleted(r.snrDb);
    return r;
}

void SnrEstimator::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }

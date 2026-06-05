/**
 * @file ZeroCrossingRate.cpp
 * @brief 过零率分析器实现
 */

#include "utils/zcr/ZeroCrossingRate.h"

#include <QElapsedTimer>

ZeroCrossingRate::ZeroCrossingRate(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

ZeroCrossingRate::Result ZeroCrossingRate::compute(
    const QVector<double>& data, double sampleRate)
{
    Result result;
    if (data.size() < 2) return result;

    QElapsedTimer timer;
    timer.start();

    int count = 0;
    for (int i = 1; i < data.size(); ++i) {
        if ((data[i - 1] >= 0.0 && data[i] < 0.0) ||
            (data[i - 1] < 0.0 && data[i] >= 0.0)) {
            ++count;
            result.crossingIndices.append(i);
        }
    }

    result.crossingCount = count;
    result.rate = static_cast<double>(count) / (data.size() - 1);
    result.estimatedFrequency = (sampleRate > 0 && count > 1)
        ? sampleRate * result.rate / 2.0 : 0.0;

    m_stats.totalAnalyses++;
    m_stats.totalCrossingsDetected += count;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalAnalyses;

    emit analysisCompleted(count, result.rate);
    return result;
}

QVector<double> ZeroCrossingRate::slidingWindowRate(
    const QVector<double>& data, int windowSize, int hopSize) const
{
    QVector<double> rates;
    if (data.size() < windowSize || windowSize < 2 || hopSize < 1) return rates;

    for (int start = 0; start + windowSize <= data.size(); start += hopSize) {
        int count = 0;
        for (int i = start + 1; i < start + windowSize; ++i) {
            if ((data[i - 1] >= 0.0 && data[i] < 0.0) ||
                (data[i - 1] < 0.0 && data[i] >= 0.0)) {
                ++count;
            }
        }
        rates.append(static_cast<double>(count) / (windowSize - 1));
    }
    return rates;
}

double ZeroCrossingRate::positiveRate(const QVector<double>& data) const
{
    if (data.size() < 2) return 0.0;

    int count = 0;
    for (int i = 1; i < data.size(); ++i) {
        if (data[i - 1] < 0.0 && data[i] >= 0.0) {
            ++count;
        }
    }
    return static_cast<double>(count) / (data.size() - 1);
}

void ZeroCrossingRate::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }

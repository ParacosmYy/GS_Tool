/**
 * @file StatisticalMoments.cpp
 * @brief 统计矩计算器实现
 */

#include "utils/moment/StatisticalMoments.h"

#include <QtMath>
#include <QElapsedTimer>
#include <cmath>

StatisticalMoments::StatisticalMoments(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

StatisticalMoments::Moments StatisticalMoments::compute(const QVector<double>& data)
{
    Moments m;
    int n = data.size();
    if (n < 2) return m;

    QElapsedTimer timer;
    timer.start();

    /* 均值 */
    double sum = 0.0;
    for (double v : data) sum += v;
    m.mean = sum / n;

    /* 方差 */
    double sumSq = 0.0;
    for (double v : data) {
        double diff = v - m.mean;
        sumSq += diff * diff;
    }
    m.variance = sumSq / (n - 1);
    m.stdDev = qSqrt(m.variance);

    /* 偏度和峰度 */
    double m3 = 0.0, m4 = 0.0;
    for (double v : data) {
        double diff = v - m.mean;
        double d2 = diff * diff;
        m3 += d2 * diff;
        m4 += d2 * d2;
    }
    m3 /= n;
    m4 /= n;

    double sigma3 = m.stdDev * m.stdDev * m.stdDev;
    double sigma4 = sigma3 * m.stdDev;

    m.skewness = (sigma3 > 1e-15) ? m3 / sigma3 : 0.0;
    m.kurtosis = (sigma4 > 1e-15) ? m4 / sigma4 : 0.0;
    m.excessKurtosis = m.kurtosis - 3.0;

    m_stats.totalComputations++;
    m_stats.totalSamplesProcessed += n;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationCompleted(n);
    return m;
}

QVector<StatisticalMoments::Moments> StatisticalMoments::slidingWindow(
    const QVector<double>& data, int windowSize)
{
    QVector<Moments> result;
    int n = data.size();
    if (n < windowSize || windowSize < 2) return result;

    result.reserve(n - windowSize + 1);

    for (int start = 0; start + windowSize <= n; ++start) {
        QVector<double> window(data.constBegin() + start,
                               data.constBegin() + start + windowSize);
        result.append(compute(window));
    }
    return result;
}

void StatisticalMoments::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }

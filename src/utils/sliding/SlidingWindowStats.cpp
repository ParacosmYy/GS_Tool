/**
 * @file SlidingWindowStats.cpp
 * @brief 滑动窗口统计实现
 */

#include "utils/sliding/SlidingWindowStats.h"
#include <QtMath>
#include <algorithm>

SlidingWindowStats::SlidingWindowStats(QObject* parent)
    : QObject(parent), m_windowSize(100), m_meanSum(0.0) {}

void SlidingWindowStats::setWindowSize(int size) { m_windowSize = qMax(2, size); }

void SlidingWindowStats::pushValue(double value)
{
    m_window.enqueue(value);
    while (m_window.size() > m_windowSize) {
        m_window.dequeue();
    }

    recalculate();

    ++m_stats.totalUpdates;
    ++m_stats.totalValuesPushed;
    if (m_window.size() > m_stats.peakWindowSize) {
        m_stats.peakWindowSize = m_window.size();
    }
    m_meanSum += m_current.mean;
    m_stats.averageMean = m_meanSum
        / static_cast<double>(m_stats.totalUpdates);

    emit windowUpdated(m_current);
}

SlidingWindowStats::WindowSummary SlidingWindowStats::currentSummary() const
{
    return m_current;
}

void SlidingWindowStats::reset()
{
    m_window.clear();
    m_current = WindowSummary{};
}

void SlidingWindowStats::resetStatistics()
{
    m_stats = Stats{};
    m_meanSum = 0.0;
}

void SlidingWindowStats::recalculate()
{
    if (m_window.isEmpty()) { m_current = {}; return; }

    int n = m_window.size();
    double sum = 0.0, minV = m_window.first(), maxV = m_window.first();
    for (double v : m_window) {
        sum += v;
        if (v < minV) minV = v;
        if (v > maxV) maxV = v;
    }
    double mean = sum / n;
    double sqSum = 0.0;
    for (double v : m_window) {
        double d = v - mean;
        sqSum += d * d;
    }

    m_current.sum = sum;
    m_current.mean = mean;
    m_current.min = minV;
    m_current.max = maxV;
    m_current.count = n;
    m_current.variance = (n > 1) ? sqSum / (n - 1) : 0.0;
    m_current.stddev = qSqrt(m_current.variance);
}

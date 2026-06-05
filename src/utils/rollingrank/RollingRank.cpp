/**
 * @file RollingRank.cpp
 * @brief 滚动排名计算器实现
 */

#include "utils/rollingrank/RollingRank.h"

#include <QElapsedTimer>
#include <algorithm>

RollingRank::RollingRank(int windowSize, QObject* parent)
    : QObject(parent), m_windowSize(qMax(2, windowSize)), m_timeSum(0.0) {}

void RollingRank::add(double value)
{
    QElapsedTimer timer;
    timer.start();

    if (m_window.size() >= m_windowSize) {
        m_window.removeFirst();
    }
    m_window.append(value);

    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalUpdates + m_stats.totalQueries, 1ULL);

    emit rankUpdated(value, percentileRank(value));
}

double RollingRank::percentileRank(double value) const
{
    if (m_window.isEmpty()) return 0.0;

    int count = 0;
    for (double v : m_window) {
        if (v <= value) ++count;
    }
    return 100.0 * count / m_window.size();
}

double RollingRank::percentile(double p) const
{
    if (m_window.isEmpty()) return 0.0;

    QVector<double> sorted = m_window;
    std::sort(sorted.begin(), sorted.end());

    double idx = (p / 100.0) * (sorted.size() - 1);
    int lo = static_cast<int>(qFloor(idx));
    int hi = static_cast<int>(qCeil(idx));
    if (lo == hi || hi >= sorted.size()) return sorted[lo];

    double frac = idx - lo;
    return sorted[lo] * (1.0 - frac) + sorted[hi] * frac;
}

double RollingRank::median() const
{
    return percentile(50.0);
}

int RollingRank::rank(double value) const
{
    int count = 0;
    for (double v : m_window) {
        if (v < value) ++count;
    }
    return count + 1;
}

void RollingRank::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

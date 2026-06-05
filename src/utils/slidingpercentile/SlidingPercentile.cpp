/**
 * @file SlidingPercentile.cpp
 * @brief 滑动百分位计算器实现
 */

#include "utils/slidingpercentile/SlidingPercentile.h"

#include <QElapsedTimer>
#include <QtMath>

SlidingPercentile::SlidingPercentile(int windowSize, QObject* parent)
    : QObject(parent), m_windowSize(qMax(2, windowSize)),
      m_pos(0), m_timeSum(0.0)
{
    m_buffer.resize(m_windowSize, 0.0);
}

void SlidingPercentile::add(double value)
{
    QElapsedTimer timer;
    timer.start();

    /* 如果窗口已满，移除最旧值 */
    if (m_pos >= m_windowSize) {
        double oldVal = m_buffer[m_pos % m_windowSize];
        auto it = m_sorted.find(oldVal);
        if (it != m_sorted.end()) {
            it.value()--;
            if (it.value() <= 0) m_sorted.erase(it);
        }
    }

    /* 添加新值 */
    m_buffer[m_pos % m_windowSize] = value;
    m_sorted[value]++;

    m_pos++;

    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalUpdates + m_stats.totalQueries, 1ULL);

    emit valueAdded(value, median());
}

double SlidingPercentile::query(double p) const
{
    int count = qMin(m_pos, m_windowSize);
    if (count == 0) return 0.0;

    double targetRank = (p / 100.0) * (count - 1);
    int cumulative = 0;

    for (auto it = m_sorted.constBegin(); it != m_sorted.constEnd(); ++it) {
        cumulative += it.value();
        if (cumulative - 1 >= static_cast<int>(targetRank)) {
            return it.key();
        }
    }

    return m_sorted.isEmpty() ? 0.0 : m_sorted.lastKey();
}

QVector<double> SlidingPercentile::queryBatch(
    const QVector<double>& percentiles) const
{
    QVector<double> results;
    results.reserve(percentiles.size());
    for (double p : percentiles) {
        results.append(query(p));
    }
    return results;
}

void SlidingPercentile::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

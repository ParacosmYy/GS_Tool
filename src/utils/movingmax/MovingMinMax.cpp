/**
 * @file MovingMinMax.cpp
 * @brief 滑动窗口最大/最小值实现
 */

#include "utils/movingmax/MovingMinMax.h"

#include <QElapsedTimer>
#include <QtMath>

MovingMinMax::MovingMinMax(int windowSize, QObject* parent)
    : QObject(parent), m_windowSize(qMax(2, windowSize)),
      m_pos(0), m_timeSum(0.0) {}

void MovingMinMax::add(double value)
{
    QElapsedTimer timer;
    timer.start();

    m_buffer.append(value);

    /* 维护最大值递减队列 */
    while (!m_maxDeque.empty() &&
           m_buffer[m_maxDeque.back()] <= value) {
        m_maxDeque.pop_back();
    }
    m_maxDeque.push_back(m_buffer.size() - 1);

    /* 维护最小值递增队列 */
    while (!m_minDeque.empty() &&
           m_buffer[m_minDeque.back()] >= value) {
        m_minDeque.pop_back();
    }
    m_minDeque.push_back(m_buffer.size() - 1);

    /* 移除超出窗口的索引 */
    int expiry = m_buffer.size() - m_windowSize;
    if (expiry >= 0) {
        while (!m_maxDeque.empty() && m_maxDeque.front() < expiry)
            m_maxDeque.pop_front();
        while (!m_minDeque.empty() && m_minDeque.front() < expiry)
            m_minDeque.pop_front();
    }

    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalUpdates + m_stats.totalQueries, 1ULL);

    emit windowUpdated(min(), max());
}

double MovingMinMax::max() const
{
    return m_maxDeque.empty() ? 0.0 : m_buffer[m_maxDeque.front()];
}

double MovingMinMax::min() const
{
    return m_minDeque.empty() ? 0.0 : m_buffer[m_minDeque.front()];
}

QVector<double> MovingMinMax::batchMax(const QVector<double>& values)
{
    reset();
    QVector<double> result;
    result.reserve(values.size());
    for (double v : values) {
        add(v);
        if (m_buffer.size() >= m_windowSize) {
            result.append(max());
        }
    }
    return result;
}

QVector<double> MovingMinMax::batchMin(const QVector<double>& values)
{
    reset();
    QVector<double> result;
    result.reserve(values.size());
    for (double v : values) {
        add(v);
        if (m_buffer.size() >= m_windowSize) {
            result.append(min());
        }
    }
    return result;
}

void MovingMinMax::reset()
{
    m_buffer.clear();
    m_maxDeque.clear();
    m_minDeque.clear();
}

void MovingMinMax::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

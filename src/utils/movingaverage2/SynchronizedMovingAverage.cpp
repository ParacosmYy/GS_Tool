/**
 * @file SynchronizedMovingAverage.cpp
 * @brief 线程安全移动平均 — QMutex保护的滑动窗口均值
 */

#include "SynchronizedMovingAverage.h"
#include <QMutexLocker>
#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

SynchronizedMovingAverage::SynchronizedMovingAverage(int windowSize,
                                                       QObject* parent)
    : QObject(parent)
    , m_windowSize(windowSize > 0 ? windowSize : 50)
    , m_buffer(m_windowSize, 0.0)
    , m_pos(0)
    , m_count(0)
    , m_sum(0.0)
    , m_timeSum(0.0)
{
}

void SynchronizedMovingAverage::add(double value)
{
    QMutexLocker locker(&m_mutex);
    QElapsedTimer timer;
    timer.start();

    if (m_count < m_windowSize) {
        m_buffer[m_pos] = value;
        m_sum += value;
        m_count++;
    } else {
        m_sum -= m_buffer[m_pos];
        m_buffer[m_pos] = value;
        m_sum += value;
    }

    m_pos = (m_pos + 1) % m_windowSize;

    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalUpdates;

    emit valueAdded(value, mean());
}

double SynchronizedMovingAverage::mean() const
{
    QMutexLocker locker(&m_mutex);
    if (m_count == 0) return 0.0;
    return m_sum / m_count;
}

double SynchronizedMovingAverage::variance() const
{
    QMutexLocker locker(&m_mutex);
    if (m_count < 2) return 0.0;

    double avg = m_sum / m_count;
    double varSum = 0.0;
    for (int i = 0; i < m_count; ++i) {
        double diff = m_buffer[i] - avg;
        varSum += diff * diff;
    }
    return varSum / (m_count - 1);
}

double SynchronizedMovingAverage::stdDev() const
{
    double v = variance();
    return v >= 0.0 ? std::sqrt(v) : 0.0;
}

double SynchronizedMovingAverage::median() const
{
    QMutexLocker locker(&m_mutex);
    if (m_count == 0) return 0.0;

    QVector<double> sorted(m_buffer.constBegin(), m_buffer.constBegin() + m_count);
    std::sort(sorted.begin(), sorted.end());

    int mid = m_count / 2;
    if (m_count % 2 == 0) {
        return (sorted[mid - 1] + sorted[mid]) / 2.0;
    }
    return sorted[mid];
}

int SynchronizedMovingAverage::count() const
{
    QMutexLocker locker(&m_mutex);
    return m_count;
}

void SynchronizedMovingAverage::reset()
{
    QMutexLocker locker(&m_mutex);
    m_buffer.fill(0.0);
    m_pos = 0;
    m_count = 0;
    m_sum = 0.0;
}

void SynchronizedMovingAverage::resetStatistics()
{
    QMutexLocker locker(&m_mutex);
    m_stats = Stats{};
    m_timeSum = 0.0;
}

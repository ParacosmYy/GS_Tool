/**
 * @file CircularStatsBuffer.cpp
 * @brief 统计环形缓冲区实现
 */

#include "utils/circbuf/CircularStatsBuffer.h"
#include <QtMath>
#include <limits>

CircularStatsBuffer::CircularStatsBuffer(int capacity, QObject* parent)
    : QObject(parent), m_head(0), m_count(0),
      m_sum(0.0), m_sqSum(0.0),
      m_min(std::numeric_limits<double>::max()),
      m_max(std::numeric_limits<double>::lowest())
{
    m_buffer.resize(qMax(1, capacity));
}

void CircularStatsBuffer::setCapacity(int capacity)
{
    clear();
    m_buffer.resize(qMax(1, capacity));
}

void CircularStatsBuffer::write(double value)
{
    if (m_count == m_buffer.size()) {
        /* 覆盖最旧数据 — 先从统计中移除 */
        double old = m_buffer[m_head];
        m_sum -= old;
        m_sqSum -= old * old;
        emit dataOverwritten(old);
        ++m_stats.totalOverwrites;
    } else {
        ++m_count;
    }

    m_buffer[m_head] = value;
    m_head = (m_head + 1) % m_buffer.size();

    m_sum += value;
    m_sqSum += value * value;
    if (value < m_min) m_min = value;
    if (value > m_max) m_max = value;

    ++m_stats.totalWrites;
    if (m_count > m_stats.peakUsage) m_stats.peakUsage = m_count;

    updateStats();

    if (m_count == m_buffer.size()) emit bufferFull();
}

void CircularStatsBuffer::writeBatch(const QVector<double>& values)
{
    for (double v : values) write(v);
}

QVector<double> CircularStatsBuffer::readAll() const
{
    QVector<double> result;
    result.reserve(m_count);
    int start = (m_head - m_count + m_buffer.size()) % m_buffer.size();
    for (int i = 0; i < m_count; ++i) {
        int idx = (start + i) % m_buffer.size();
        result.append(m_buffer[idx]);
    }
    return result;
}

double CircularStatsBuffer::readOldest() const
{
    if (m_count == 0) return 0.0;
    int idx = (m_head - m_count + m_buffer.size()) % m_buffer.size();
    return m_buffer[idx];
}

double CircularStatsBuffer::readNewest() const
{
    if (m_count == 0) return 0.0;
    int idx = (m_head - 1 + m_buffer.size()) % m_buffer.size();
    return m_buffer[idx];
}

int CircularStatsBuffer::size() const { return m_count; }
int CircularStatsBuffer::capacity() const { return m_buffer.size(); }
bool CircularStatsBuffer::isEmpty() const { return m_count == 0; }
bool CircularStatsBuffer::isFull() const { return m_count == m_buffer.size(); }

double CircularStatsBuffer::mean() const
{
    return (m_count > 0) ? m_sum / m_count : 0.0;
}

double CircularStatsBuffer::stddev() const
{
    if (m_count < 2) return 0.0;
    double m = mean();
    double variance = (m_sqSum / m_count) - m * m;
    return qSqrt(qMax(0.0, variance));
}

double CircularStatsBuffer::min() const { return (m_count > 0) ? m_min : 0.0; }
double CircularStatsBuffer::max() const { return (m_count > 0) ? m_max : 0.0; }
double CircularStatsBuffer::sum() const { return m_sum; }

void CircularStatsBuffer::clear()
{
    m_head = 0;
    m_count = 0;
    m_sum = 0.0;
    m_sqSum = 0.0;
    m_min = std::numeric_limits<double>::max();
    m_max = std::numeric_limits<double>::lowest();
}

void CircularStatsBuffer::updateStats()
{
    m_stats.currentMean = mean();
    m_stats.currentStddev = stddev();
    m_stats.currentMin = min();
    m_stats.currentMax = max();
    emit statsUpdated(m_stats.currentMean, m_stats.currentStddev);
}

void CircularStatsBuffer::resetStatistics()
{
    m_stats = Stats{};
    m_stats.currentMin = min();
    m_stats.currentMax = max();
    m_stats.currentMean = mean();
    m_stats.currentStddev = stddev();
}

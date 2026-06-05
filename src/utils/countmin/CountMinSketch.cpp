/**
 * @file CountMinSketch.cpp
 * @brief Count-Min Sketch实现
 */

#include "utils/countmin/CountMinSketch.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

CountMinSketch::CountMinSketch(int depth, int width, QObject* parent)
    : QObject(parent), m_depth(qMax(2, depth)),
      m_width(qMax(16, width)), m_timeSum(0.0)
{
    m_table.resize(m_depth);
    for (auto& row : m_table) row.resize(m_width, 0);
}

void CountMinSketch::update(const QByteArray& item, quint64 count)
{
    QElapsedTimer timer;
    timer.start();

    for (int d = 0; d < m_depth; ++d) {
        quint32 idx = hashItem(item, d) % m_width;
        m_table[d][idx] += count;
    }

    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalUpdates + m_stats.totalQueries, 1ULL);

    emit updated(item, count);
}

quint64 CountMinSketch::estimate(const QByteArray& item) const
{
    quint64 minVal = std::numeric_limits<quint64>::max();
    for (int d = 0; d < m_depth; ++d) {
        quint32 idx = hashItem(item, d) % m_width;
        minVal = qMin(minVal, m_table[d][idx]);
    }

    m_stats.totalQueries++;
    return minVal;
}

quint64 CountMinSketch::innerProduct(const CountMinSketch& other) const
{
    if (m_depth != other.m_depth || m_width != other.m_width) return 0;

    quint64 minSum = std::numeric_limits<quint64>::max();
    for (int d = 0; d < m_depth; ++d) {
        quint64 rowSum = 0;
        for (int w = 0; w < m_width; ++w) {
            rowSum += m_table[d][w] * other.m_table[d][w];
        }
        minSum = qMin(minSum, rowSum);
    }
    return minSum;
}

void CountMinSketch::merge(const CountMinSketch& other)
{
    if (m_depth != other.m_depth || m_width != other.m_width) return;
    for (int d = 0; d < m_depth; ++d) {
        for (int w = 0; w < m_width; ++w) {
            m_table[d][w] += other.m_table[d][w];
        }
    }
}

void CountMinSketch::reset()
{
    for (auto& row : m_table) row.fill(0);
}

quint32 CountMinSketch::hashItem(const QByteArray& item, int row) const
{
    quint32 h = static_cast<quint32>(row * 0x9e3779b9);
    for (int i = 0; i < item.size(); ++i) {
        h ^= static_cast<quint8>(item[i]);
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h;
}

void CountMinSketch::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

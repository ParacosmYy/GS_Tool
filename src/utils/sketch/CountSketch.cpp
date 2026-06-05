/**
 * @file CountSketch.cpp
 * @brief Count-Sketch实现
 */

#include "CountSketch.h"
#include <QElapsedTimer>
#include <algorithm>

CountSketch::CountSketch(int width, int depth, QObject* parent)
    : QObject(parent)
    , m_width(qMax(16, width))
    , m_depth(qMax(2, depth))
    , m_timeSum(0.0)
{
    m_counts.resize(m_depth);
    for (auto& row : m_counts)
        row.fill(0, m_width);
}

qint64 CountSketch::hash(const QByteArray& data, int row) const
{
    qint64 h = static_cast<qint64>(row) * 0x9E3779B97F4A7C15LL;
    for (char c : data) {
        h ^= static_cast<qint64>(static_cast<quint8>(c));
        h *= 0xBF58476D1CE4E5B9LL;
        h ^= h >> 31;
    }
    return h;
}

int CountSketch::signHash(const QByteArray& data, int row) const
{
    qint64 h = hash(data, row + 1000);
    return (h & 1) ? 1 : -1;
}

void CountSketch::update(const QByteArray& item, long long count)
{
    QElapsedTimer timer;
    timer.start();

    for (int d = 0; d < m_depth; ++d) {
        int w = static_cast<int>(static_cast<quint64>(hash(item, d)) % static_cast<quint64>(m_width));
        int sign = signHash(item, d);
        m_counts[d][w] += sign * count;
    }

    m_stats.totalUpdates++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalUpdates + m_stats.totalQueries;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit updated(item, count);
}

void CountSketch::updateString(const QString& item, long long count)
{
    update(item.toUtf8(), count);
}

long long CountSketch::estimate(const QByteArray& item) const
{
    QElapsedTimer timer;
    timer.start();

    long long minVal = std::numeric_limits<long long>::max();
    long long maxVal = std::numeric_limits<long long>::min();

    for (int d = 0; d < m_depth; ++d) {
        int w = static_cast<int>(static_cast<quint64>(hash(item, d)) % static_cast<quint64>(m_width));
        int sign = signHash(item, d);
        long long val = m_counts[d][w] * sign;
        minVal = qMin(minVal, val);
        maxVal = qMax(maxVal, val);
    }

    long long result = (std::abs(minVal) < std::abs(maxVal)) ? minVal : maxVal;

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalUpdates + m_stats.totalQueries;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return result;
}

long long CountSketch::estimateString(const QString& item) const
{
    return estimate(item.toUtf8());
}

void CountSketch::batchUpdate(const QVector<QByteArray>& items,
                                const QVector<long long>& counts)
{
    int n = qMin(items.size(), counts.size());
    for (int i = 0; i < n; ++i)
        update(items[i], counts[i]);
}

void CountSketch::reset()
{
    for (auto& row : m_counts)
        row.fill(0, m_width);
}

int CountSketch::width() const { return m_width; }
int CountSketch::depth() const { return m_depth; }
CountSketch::Stats CountSketch::stats() const { return m_stats; }

void CountSketch::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

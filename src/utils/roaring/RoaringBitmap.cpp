/**
 * @file RoaringBitmap.cpp
 * @brief Roaring位图实现
 */

#include "utils/roaring/RoaringBitmap.h"

#include <QElapsedTimer>
#include <algorithm>

RoaringBitmap::RoaringBitmap(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

void RoaringBitmap::add(quint32 value)
{
    QElapsedTimer timer;
    timer.start();

    m_data.insert(value);

    m_stats.totalAdds++;
    m_stats.cardinality = m_data.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalAdds + m_stats.totalRemoves + m_stats.totalSetOps, 1ULL);

    emit valueAdded(value);
}

void RoaringBitmap::addMany(const QVector<quint32>& values)
{
    QElapsedTimer timer;
    timer.start();

    for (quint32 v : values) {
        m_data.insert(v);
    }

    m_stats.totalAdds += values.size();
    m_stats.cardinality = m_data.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalAdds + m_stats.totalRemoves + m_stats.totalSetOps, 1ULL);
}

void RoaringBitmap::remove(quint32 value)
{
    QElapsedTimer timer;
    timer.start();

    m_data.remove(value);

    m_stats.totalRemoves++;
    m_stats.cardinality = m_data.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalAdds + m_stats.totalRemoves + m_stats.totalSetOps, 1ULL);

    emit valueRemoved(value);
}

bool RoaringBitmap::contains(quint32 value) const
{
    return m_data.contains(value);
}

quint64 RoaringBitmap::cardinality() const
{
    return m_data.size();
}

RoaringBitmap* RoaringBitmap::bitOr(const RoaringBitmap& other)
{
    QElapsedTimer timer;
    timer.start();

    RoaringBitmap* result = new RoaringBitmap(parent());
    /* 使用QSet的并集 */
    result->m_data = m_data | other.m_data;

    m_stats.totalSetOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalAdds + m_stats.totalRemoves + m_stats.totalSetOps, 1ULL);

    result->m_stats.cardinality = result->m_data.size();
    emit setOperationCompleted(QStringLiteral("OR"), result->m_data.size());
    return result;
}

RoaringBitmap* RoaringBitmap::bitAnd(const RoaringBitmap& other)
{
    QElapsedTimer timer;
    timer.start();

    RoaringBitmap* result = new RoaringBitmap(parent());
    result->m_data = m_data & other.m_data;

    m_stats.totalSetOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalAdds + m_stats.totalRemoves + m_stats.totalSetOps, 1ULL);

    result->m_stats.cardinality = result->m_data.size();
    emit setOperationCompleted(QStringLiteral("AND"), result->m_data.size());
    return result;
}

RoaringBitmap* RoaringBitmap::bitXor(const RoaringBitmap& other)
{
    QElapsedTimer timer;
    timer.start();

    RoaringBitmap* result = new RoaringBitmap(parent());
    /* 对称差 */
    QSet<quint32> unionSet = m_data | other.m_data;
    QSet<quint32> interSet = m_data & other.m_data;
    result->m_data = unionSet - interSet;

    m_stats.totalSetOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalAdds + m_stats.totalRemoves + m_stats.totalSetOps, 1ULL);

    result->m_stats.cardinality = result->m_data.size();
    emit setOperationCompleted(QStringLiteral("XOR"), result->m_data.size());
    return result;
}

QVector<quint32> RoaringBitmap::rangeQuery(quint32 minVal, quint32 maxVal) const
{
    QVector<quint32> result;
    for (quint32 v : m_data) {
        if (v >= minVal && v <= maxVal) {
            result.append(v);
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

QVector<quint32> RoaringBitmap::toVector() const
{
    QVector<quint32> result = m_data.values().toVector();
    std::sort(result.begin(), result.end());
    return result;
}

void RoaringBitmap::clear()
{
    m_data.clear();
    m_stats.cardinality = 0;
}

void RoaringBitmap::resetStatistics()
{
    m_stats = Stats{};
    m_stats.cardinality = m_data.size();
    m_timeSum = 0.0;
}

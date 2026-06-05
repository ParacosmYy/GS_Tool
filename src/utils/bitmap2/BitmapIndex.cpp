/**
 * @file BitmapIndex.cpp
 * @brief 位图索引实现
 */

#include "BitmapIndex.h"
#include <QElapsedTimer>

BitmapIndex::BitmapIndex(int size, QObject* parent)
    : QObject(parent)
    , m_size(qMax(1, size))
    , m_timeSum(0.0)
{
    int bytes = (m_size + 7) / 8;
    m_data.fill(0, bytes);
}

void BitmapIndex::set(int index)
{
    if (index < 0 || index >= m_size) return;
    m_data[index / 8] |= (1 << (index % 8));
}

void BitmapIndex::clear(int index)
{
    if (index < 0 || index >= m_size) return;
    m_data[index / 8] &= ~(1 << (index % 8));
}

bool BitmapIndex::test(int index) const
{
    if (index < 0 || index >= m_size) return false;
    return (m_data[index / 8] >> (index % 8)) & 1;
}

void BitmapIndex::flip(int index)
{
    if (index < 0 || index >= m_size) return;
    m_data[index / 8] ^= (1 << (index % 8));
}

BitmapIndex* BitmapIndex::operatorAnd(const BitmapIndex& other) const
{
    QElapsedTimer timer;
    timer.start();

    BitmapIndex* result = new BitmapIndex(qMin(m_size, other.m_size), parent());
    int bytes = qMin(m_data.size(), other.m_data.size());
    for (int i = 0; i < bytes; ++i)
        result->m_data[i] = m_data[i] & other.m_data[i];

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted(QStringLiteral("AND"), result->count());
    return result;
}

BitmapIndex* BitmapIndex::operatorOr(const BitmapIndex& other) const
{
    QElapsedTimer timer;
    timer.start();

    BitmapIndex* result = new BitmapIndex(qMax(m_size, other.m_size), parent());
    int bytes = qMin(m_data.size(), other.m_data.size());
    for (int i = 0; i < bytes; ++i)
        result->m_data[i] = m_data[i] | other.m_data[i];

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    return result;
}

BitmapIndex* BitmapIndex::operatorXor(const BitmapIndex& other) const
{
    QElapsedTimer timer;
    timer.start();

    BitmapIndex* result = new BitmapIndex(qMax(m_size, other.m_size), parent());
    int bytes = qMin(m_data.size(), other.m_data.size());
    for (int i = 0; i < bytes; ++i)
        result->m_data[i] = m_data[i] ^ other.m_data[i];

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    return result;
}

BitmapIndex* BitmapIndex::operatorNot() const
{
    QElapsedTimer timer;
    timer.start();

    BitmapIndex* result = new BitmapIndex(m_size, parent());
    for (int i = 0; i < m_data.size(); ++i)
        result->m_data[i] = ~m_data[i];

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    return result;
}

int BitmapIndex::count() const
{
    int c = 0;
    for (char byte : m_data) {
        quint8 b = static_cast<quint8>(byte);
        while (b) { c += b & 1; b >>= 1; }
    }
    return c;
}

bool BitmapIndex::isEmpty() const { return count() == 0; }

void BitmapIndex::setRange(int start, int end)
{
    for (int i = qMax(0, start); i < qMin(m_size, end); ++i)
        set(i);
}

void BitmapIndex::clearRange(int start, int end)
{
    for (int i = qMax(0, start); i < qMin(m_size, end); ++i)
        clear(i);
}

int BitmapIndex::size() const { return m_size; }
BitmapIndex::Stats BitmapIndex::stats() const { return m_stats; }

void BitmapIndex::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

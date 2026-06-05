/**
 * @file IndexedDeque.cpp
 * @brief 索引双端队列实现
 */

#include "IndexedDeque.h"
#include <QElapsedTimer>

IndexedDeque::IndexedDeque(int capacity, QObject* parent)
    : QObject(parent)
    , m_buffer(qMax(16, capacity))
    , m_front(0)
    , m_count(0)
    , m_capacity(qMax(16, capacity))
    , m_timeSum(0.0)
{
}

void IndexedDeque::ensureCapacity()
{
    if (m_count < m_capacity) return;

    int newCap = m_capacity * 2;
    QVector<QVariant> newBuffer(newCap);
    for (int i = 0; i < m_count; ++i)
        newBuffer[i] = m_buffer[physicalIndex(i)];

    m_buffer = newBuffer;
    m_front = 0;
    m_capacity = newCap;
}

int IndexedDeque::physicalIndex(int logicalIndex) const
{
    return (m_front + logicalIndex) % m_capacity;
}

void IndexedDeque::pushFront(const QVariant& value)
{
    QElapsedTimer timer;
    timer.start();

    ensureCapacity();
    m_front = (m_front - 1 + m_capacity) % m_capacity;
    m_buffer[m_front] = value;
    m_count++;

    m_stats.totalPushFront++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalPushFront + m_stats.totalPushBack
              + m_stats.totalPopFront + m_stats.totalPopBack;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit elementAdded(m_count);
}

void IndexedDeque::pushBack(const QVariant& value)
{
    QElapsedTimer timer;
    timer.start();

    ensureCapacity();
    int idx = (m_front + m_count) % m_capacity;
    m_buffer[idx] = value;
    m_count++;

    m_stats.totalPushBack++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalPushFront + m_stats.totalPushBack
              + m_stats.totalPopFront + m_stats.totalPopBack;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit elementAdded(m_count);
}

QVariant IndexedDeque::popFront()
{
    QElapsedTimer timer;
    timer.start();

    if (m_count == 0) return QVariant();

    QVariant val = m_buffer[m_front];
    m_front = (m_front + 1) % m_capacity;
    m_count--;

    m_stats.totalPopFront++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalPushFront + m_stats.totalPushBack
              + m_stats.totalPopFront + m_stats.totalPopBack;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return val;
}

QVariant IndexedDeque::popBack()
{
    QElapsedTimer timer;
    timer.start();

    if (m_count == 0) return QVariant();

    m_count--;
    QVariant val = m_buffer[physicalIndex(m_count)];

    m_stats.totalPopBack++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalPushFront + m_stats.totalPushBack
              + m_stats.totalPopFront + m_stats.totalPopBack;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return val;
}

QVariant IndexedDeque::at(int index) const
{
    if (index < 0 || index >= m_count) return QVariant();
    m_stats.totalAccess++;
    return m_buffer[physicalIndex(index)];
}

void IndexedDeque::setAt(int index, const QVariant& value)
{
    if (index < 0 || index >= m_count) return;
    m_buffer[physicalIndex(index)] = value;
}

QVariant IndexedDeque::front() const
{
    return m_count > 0 ? m_buffer[m_front] : QVariant();
}

QVariant IndexedDeque::back() const
{
    return m_count > 0 ? m_buffer[physicalIndex(m_count - 1)] : QVariant();
}

int IndexedDeque::size() const { return m_count; }
bool IndexedDeque::isEmpty() const { return m_count == 0; }

void IndexedDeque::clear()
{
    m_front = 0;
    m_count = 0;
}

QVector<QVariant> IndexedDeque::toVector() const
{
    QVector<QVariant> result;
    result.reserve(m_count);
    for (int i = 0; i < m_count; ++i)
        result.append(m_buffer[physicalIndex(i)]);
    return result;
}

IndexedDeque::Stats IndexedDeque::stats() const { return m_stats; }

void IndexedDeque::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

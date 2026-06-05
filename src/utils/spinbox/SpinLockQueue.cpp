/**
 * @file SpinLockQueue.cpp
 * @brief 自旋锁队列实现
 */

#include "utils/spinbox/SpinLockQueue.h"
#include <thread>

SpinLockQueue::SpinLockQueue(int capacity, QObject* parent)
    : QObject(parent), m_capacity(capacity)
{
    m_buffer.resize(capacity);
    m_head.store(0, std::memory_order_relaxed);
    m_tail.store(0, std::memory_order_relaxed);
    m_count.store(0, std::memory_order_relaxed);
    m_spinCount.store(0, std::memory_order_relaxed);
}

bool SpinLockQueue::push(const QByteArray& item)
{
    int maxSpins = 100;
    while (m_count.load(std::memory_order_acquire) >= m_capacity) {
        ++m_spinCount;
        if (--maxSpins <= 0) return false;
        std::this_thread::yield();
    }

    int tail = m_tail.load(std::memory_order_relaxed);
    m_buffer[tail] = item;
    m_tail.store((tail + 1) % m_capacity, std::memory_order_release);
    m_count.fetch_add(1, std::memory_order_release);

    ++m_stats.totalPushes;
    m_stats.totalBytesPushed += item.size();
    int sz = m_count.load(std::memory_order_relaxed);
    if (sz > m_stats.peakSize) m_stats.peakSize = sz;

    emit dataAvailable(sz);
    return true;
}

bool SpinLockQueue::push(const QVector<QByteArray>& items)
{
    for (const auto& item : items) {
        if (!push(item)) return false;
    }
    return true;
}

QByteArray SpinLockQueue::pop()
{
    if (m_count.load(std::memory_order_acquire) <= 0) return {};

    int head = m_head.load(std::memory_order_relaxed);
    QByteArray result = m_buffer[head];
    m_buffer[head].clear();
    m_head.store((head + 1) % m_capacity, std::memory_order_release);
    m_count.fetch_sub(1, std::memory_order_release);

    ++m_stats.totalPops;
    m_stats.contentionCount = m_spinCount.load(std::memory_order_relaxed);
    return result;
}

QVector<QByteArray> SpinLockQueue::popAll()
{
    QVector<QByteArray> result;
    int count = m_count.load(std::memory_order_acquire);
    result.reserve(count);

    while (m_count.load(std::memory_order_acquire) > 0) {
        QByteArray item = pop();
        if (item.isEmpty()) break;
        result.append(item);
    }
    return result;
}

int SpinLockQueue::size() const { return m_count.load(std::memory_order_relaxed); }
bool SpinLockQueue::isEmpty() const { return m_count.load(std::memory_order_relaxed) == 0; }

void SpinLockQueue::clear()
{
    m_head.store(0, std::memory_order_relaxed);
    m_tail.store(0, std::memory_order_relaxed);
    m_count.store(0, std::memory_order_relaxed);
}

void SpinLockQueue::resetStatistics() { m_stats = Stats{}; }

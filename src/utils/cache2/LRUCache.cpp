/**
 * @file LRUCache.cpp
 * @brief LRU缓存实现
 */

#include "LRUCache.h"
#include <QElapsedTimer>

LRUCache::LRUCache(int capacity, QObject* parent)
    : QObject(parent)
    , m_capacity(qMax(1, capacity))
    , m_timeSum(0.0)
{
}

bool LRUCache::get(const QString& key, QString& value)
{
    QElapsedTimer timer;
    timer.start();

    m_stats.totalGets++;

    if (m_index.contains(key)) {
        int idx = m_index[key];
        value = m_list[idx].second;
        moveToFront(idx);
        m_stats.totalHits++;
        m_timeSum += timer.elapsed();
        int total = m_stats.totalGets + m_stats.totalPuts;
        if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;
        return true;
    }

    m_stats.totalMisses++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalGets + m_stats.totalPuts;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;
    return false;
}

void LRUCache::put(const QString& key, const QString& value)
{
    QElapsedTimer timer;
    timer.start();

    m_stats.totalPuts++;

    if (m_index.contains(key)) {
        int idx = m_index[key];
        m_list[idx].second = value;
        moveToFront(idx);
    } else {
        /* 新条目: 添加到头部 */
        m_list.prepend({key, value});
        m_index[key] = 0;

        /* 更新所有索引(因为prepend移动了所有元素) */
        for (auto it = m_index.begin(); it != m_index.end(); ++it) {
            if (it.key() != key) it.value()++;
        }

        /* 淘汰 */
        while (m_list.size() > m_capacity) {
            QString evictedKey = m_list.last().first;
            m_index.remove(evictedKey);
            m_list.removeLast();
            m_stats.totalEvictions++;
            emit evicted(evictedKey);
        }
    }

    m_timeSum += timer.elapsed();
    int total = m_stats.totalGets + m_stats.totalPuts;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;
}

void LRUCache::remove(const QString& key)
{
    if (!m_index.contains(key)) return;

    int idx = m_index[key];
    m_list.removeAt(idx);
    m_index.remove(key);

    /* 重建索引 */
    m_index.clear();
    for (int i = 0; i < m_list.size(); ++i)
        m_index[m_list[i].first] = i;
}

bool LRUCache::contains(const QString& key) const
{
    return m_index.contains(key);
}

void LRUCache::clear()
{
    m_list.clear();
    m_index.clear();
}

int LRUCache::size() const
{
    return m_list.size();
}

int LRUCache::capacity() const
{
    return m_capacity;
}

void LRUCache::setCapacity(int capacity)
{
    m_capacity = qMax(1, capacity);
    while (m_list.size() > m_capacity) {
        QString evictedKey = m_list.last().first;
        m_index.remove(evictedKey);
        m_list.removeLast();
        m_stats.totalEvictions++;
        emit evicted(evictedKey);
    }
}

double LRUCache::hitRate() const
{
    if (m_stats.totalGets == 0) return 0.0;
    return static_cast<double>(m_stats.totalHits) / m_stats.totalGets;
}

void LRUCache::moveToFront(int idx)
{
    if (idx == 0) return;

    QPair<QString, QString> item = m_list.takeAt(idx);
    m_list.prepend(item);

    /* 重建索引 */
    m_index.clear();
    for (int i = 0; i < m_list.size(); ++i)
        m_index[m_list[i].first] = i;
}

LRUCache::Stats LRUCache::stats() const { return m_stats; }

void LRUCache::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

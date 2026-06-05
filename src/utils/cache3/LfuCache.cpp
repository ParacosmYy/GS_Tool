/**
 * @file LfuCache.cpp
 * @brief LFU缓存实现
 */

#include "LfuCache.h"
#include <QElapsedTimer>

LfuCache::LfuCache(int capacity, QObject* parent)
    : QObject(parent)
    , m_capacity(qMax(1, capacity))
    , m_minFreq(0)
    , m_count(0)
    , m_timeSum(0.0)
{
}

QVariant LfuCache::get(const QString& key)
{
    QElapsedTimer timer;
    timer.start();

    m_stats.totalGets++;

    auto it = m_keyMap.find(key);
    if (it == m_keyMap.end()) {
        m_timeSum += timer.elapsed();
        int total = m_stats.totalGets + m_stats.totalPuts;
        if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;
        return QVariant();
    }

    /* 提升频率 */
    auto entryIt = it.value();
    Entry entry = *entryIt;
    int oldFreq = entry.freq;

    m_freqMap[oldFreq].erase(entryIt);
    if (m_freqMap[oldFreq].isEmpty()) {
        m_freqMap.remove(oldFreq);
        if (m_minFreq == oldFreq) m_minFreq++;
    }

    entry.freq++;
    m_freqMap[entry.freq].push_front(entry);
    m_keyMap[key] = m_freqMap[entry.freq].begin();

    m_stats.totalHits++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalGets + m_stats.totalPuts;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return entry.value;
}

void LfuCache::put(const QString& key, const QVariant& value)
{
    QElapsedTimer timer;
    timer.start();

    m_stats.totalPuts++;

    auto it = m_keyMap.find(key);
    if (it != m_keyMap.end()) {
        /* 更新已有条目 */
        auto entryIt = it.value();
        Entry entry = *entryIt;
        int oldFreq = entry.freq;

        m_freqMap[oldFreq].erase(entryIt);
        if (m_freqMap[oldFreq].isEmpty()) {
            m_freqMap.remove(oldFreq);
            if (m_minFreq == oldFreq) m_minFreq++;
        }

        entry.value = value;
        entry.freq++;
        m_freqMap[entry.freq].push_front(entry);
        m_keyMap[key] = m_freqMap[entry.freq].begin();
    } else {
        /* 驱逐 */
        if (m_count >= m_capacity) {
            auto& minList = m_freqMap[m_minFreq];
            if (!minList.isEmpty()) {
                Entry evicted = minList.last();
                m_keyMap.remove(evicted.key);
                minList.removeLast();
                if (minList.isEmpty()) m_freqMap.remove(m_minFreq);
                m_count--;
                m_stats.totalEvictions++;
                emit this->evicted(evicted.key);
            }
        }

        Entry entry{key, value, 1};
        m_freqMap[1].push_front(entry);
        m_keyMap[key] = m_freqMap[1].begin();
        m_minFreq = 1;
        m_count++;
    }

    m_timeSum += timer.elapsed();
    int total = m_stats.totalGets + m_stats.totalPuts;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;
}

bool LfuCache::contains(const QString& key) const
{
    return m_keyMap.contains(key);
}

bool LfuCache::remove(const QString& key)
{
    auto it = m_keyMap.find(key);
    if (it == m_keyMap.end()) return false;

    auto entryIt = it.value();
    int freq = entryIt->freq;
    m_freqMap[freq].erase(entryIt);
    if (m_freqMap[freq].isEmpty()) m_freqMap.remove(freq);
    m_keyMap.erase(it);
    m_count--;
    return true;
}

void LfuCache::clear()
{
    m_keyMap.clear();
    m_freqMap.clear();
    m_minFreq = 0;
    m_count = 0;
}

int LfuCache::size() const { return m_count; }

double LfuCache::hitRate() const
{
    if (m_stats.totalGets == 0) return 0.0;
    return static_cast<double>(m_stats.totalHits) / m_stats.totalGets;
}

LfuCache::Stats LfuCache::stats() const { return m_stats; }

void LfuCache::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

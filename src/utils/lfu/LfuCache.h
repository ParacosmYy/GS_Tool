/**
 * @file LfuCache.h
 * @brief LFU缓存 — 最不经常使用淘汰策略
 */
#ifndef LFUCACHE_H
#define LFUCACHE_H

#include <QHash>
#include <QList>

template<typename Key, typename Value>
class LfuCache {
public:
    struct Stats {
        quint64 totalLookups = 0;
        quint64 totalHits = 0;
        quint64 totalMisses = 0;
        quint64 totalEvictions = 0;
    };

    explicit LfuCache(int capacity = 256) : m_capacity(qMax(1, capacity)) {}

    bool get(const Key& key, Value& value)
    {
        ++m_stats.totalLookups;
        auto it = m_cache.find(key);
        if (it == m_cache.end()) {
            ++m_stats.totalMisses;
            return false;
        }
        ++m_stats.totalHits;
        it.value().frequency++;
        value = it.value().data;
        return true;
    }

    void put(const Key& key, const Value& value)
    {
        auto it = m_cache.find(key);
        if (it != m_cache.end()) {
            it.value().data = value;
            it.value().frequency++;
            return;
        }

        if (m_cache.size() >= m_capacity) evict();

        m_cache[key] = {value, 1};
    }

    bool contains(const Key& key) const { return m_cache.contains(key); }
    int size() const { return m_cache.size(); }
    void clear() { m_cache.clear(); }

    const Stats& stats() const { return m_stats; }
    void resetStatistics() { m_stats = Stats{}; }

private:
    struct Entry { Value data; int frequency; };

    void evict()
    {
        auto it = m_cache.begin();
        auto minIt = it;
        int minFreq = it.value().frequency;
        for (++it; it != m_cache.end(); ++it) {
            if (it.value().frequency < minFreq) {
                minFreq = it.value().frequency;
                minIt = it;
            }
        }
        m_cache.erase(minIt);
        ++m_stats.totalEvictions;
    }

    QHash<Key, Entry> m_cache;
    int m_capacity;
    Stats m_stats;
};

#endif // LFUCACHE_H

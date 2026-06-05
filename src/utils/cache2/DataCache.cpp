/**
 * @file DataCache.cpp
 * @brief 数据缓存引擎实现
 */

#include "utils/cache2/DataCache.h"

DataCache::DataCache(QObject* parent)
    : QObject(parent), m_maxSize(100), m_policy(EvictionPolicy::LRU) {}

void DataCache::setMaxSize(int s) { m_maxSize = qMax(1, s); }
void DataCache::setPolicy(EvictionPolicy p) { m_policy = p; }

void DataCache::put(const QString& key, const QByteArray& value)
{
    if (m_data.contains(key)) {
        m_data[key].first = value;
        ++m_data[key].second;
        if (m_policy == EvictionPolicy::LRU) {
            m_order.removeOne(key);
            m_order.append(key);
        }
        return;
    }

    while (m_data.size() >= m_maxSize) evict();

    m_data[key] = {value, 1};
    m_order.append(key);
    if (m_data.size() > m_stats.peakSize) m_stats.peakSize = m_data.size();
}

QByteArray DataCache::get(const QString& key)
{
    if (!m_data.contains(key)) {
        ++m_stats.totalMisses;
        emit cacheMiss(key);
        updateHitRate();
        return {};
    }

    ++m_stats.totalHits;
    ++m_data[key].second;
    if (m_policy == EvictionPolicy::LRU) {
        m_order.removeOne(key);
        m_order.append(key);
    }
    emit cacheHit(key);
    updateHitRate();
    return m_data[key].first;
}

bool DataCache::contains(const QString& key) const { return m_data.contains(key); }

void DataCache::remove(const QString& key)
{
    m_data.remove(key);
    m_order.removeOne(key);
}

void DataCache::clear()
{
    m_data.clear();
    m_order.clear();
}

int DataCache::size() const { return m_data.size(); }

void DataCache::resetStatistics() { m_stats = Stats{}; }

void DataCache::evict()
{
    if (m_order.isEmpty()) return;
    QString victim;

    switch (m_policy) {
    case EvictionPolicy::LRU:
        victim = m_order.first();
        break;
    case EvictionPolicy::FIFO:
        victim = m_order.first();
        break;
    case EvictionPolicy::LFU: {
        int minFreq = std::numeric_limits<int>::max();
        for (const auto& k : m_order) {
            if (m_data[k].second < minFreq) {
                minFreq = m_data[k].second;
                victim = k;
            }
        }
        break;
    }
    }

    m_data.remove(victim);
    m_order.removeOne(victim);
    ++m_stats.totalEvictions;
    emit eviction(victim, m_data.size());
}

void DataCache::updateHitRate()
{
    quint64 total = m_stats.totalHits + m_stats.totalMisses;
    m_stats.hitRate = (total > 0) ? static_cast<double>(m_stats.totalHits) / total : 0.0;
}

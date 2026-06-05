/**
 * @file TwoWayMap.h
 * @brief 双向映射 — 键值双向查找(仅头文件)
 *
 * 功能: 双向映射容器，支持key→value和value→key的双向O(1)查找，
 *       支持批量插入/删除，统计操作次数。
 */
#ifndef TWOWAYMAP_H
#define TWOWAYMAP_H

#include <QMap>

/**
 * @class TwoWayMap
 * @brief 双向映射容器(无QObject，纯模板)
 */
template<typename K, typename V>
class TwoWayMap {
public:
    /** 统计 */
    struct Stats {
        quint64 totalInserts = 0;
        quint64 totalLookups = 0;
        quint64 totalDeletes = 0;
    };

    /** @brief 插入映射对 @param key 键 @param value 值 */
    void insert(const K& key, const V& value) {
        m_forward[key] = value;
        m_reverse[value] = key;
        m_stats.totalInserts++;
    }

    /** @brief 通过键查找值 @param key 键 @return 值(不存在返回默认) */
    V value(const K& key) const {
        m_stats.totalLookups++;
        return m_forward.value(key);
    }

    /** @brief 通过值查找键 @param value 值 @return 键(不存在返回默认) */
    K key(const V& value) const {
        m_stats.totalLookups++;
        return m_reverse.value(value);
    }

    /** @brief 通过键查找值(含检查) @param key 键 @param ok 是否找到 @return 值 */
    V value(const K& key, bool& ok) const {
        m_stats.totalLookups++;
        auto it = m_forward.find(key);
        if (it != m_forward.end()) { ok = true; return it.value(); }
        ok = false;
        return V();
    }

    /** @brief 通过值查找键(含检查) @param value 值 @param ok 是否找到 @return 键 */
    K key(const V& value, bool& ok) const {
        m_stats.totalLookups++;
        auto it = m_reverse.find(value);
        if (it != m_reverse.end()) { ok = true; return it.value(); }
        ok = false;
        return K();
    }

    /** @brief 删除键 @param key 键 @return 是否成功 */
    bool remove(const K& key) {
        auto it = m_forward.find(key);
        if (it == m_forward.end()) return false;
        V val = it.value();
        m_forward.erase(it);
        m_reverse.remove(val);
        m_stats.totalDeletes++;
        return true;
    }

    /** @brief 包含键 @param key 键 @return 是否存在 */
    bool containsKey(const K& key) const { return m_forward.contains(key); }

    /** @brief 包含值 @param value 值 @return 是否存在 */
    bool containsValue(const V& value) const { return m_reverse.contains(value); }

    int size() const { return m_forward.size(); }
    bool isEmpty() const { return m_forward.isEmpty(); }
    void clear() { m_forward.clear(); m_reverse.clear(); }

    QList<K> keys() const { return m_forward.keys(); }
    QList<V> values() const { return m_forward.values(); }

    const Stats& stats() const { return m_stats; }
    void resetStatistics() { m_stats = Stats{}; }

private:
    QMap<K, V> m_forward;
    QMap<V, K> m_reverse;
    mutable Stats m_stats;
};

#endif // TWOWAYMAP_H

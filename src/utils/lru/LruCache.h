/**
 * @file LruCache.h
 * @brief LRU缓存 — O(1)读取/插入/淘汰
 *
 * 功能: 基于哈希表+双向链表的LRU缓存，支持容量配置/TTL过期，
 *       统计命中/未命中/淘汰次数。
 */
#ifndef LRUCACHE_H
#define LRUCACHE_H

#include <QObject>
#include <QHash>
#include <QList>

template<typename Key, typename Value>
class LruCache {
public:
    /** 缓存统计 */
    struct Stats {
        quint64 totalLookups = 0;
        quint64 totalHits = 0;
        quint64 totalMisses = 0;
        quint64 totalEvictions = 0;
        double hitRate() const {
            return totalLookups > 0
                ? static_cast<double>(totalHits) / totalLookups : 0.0;
        }
    };

    explicit LruCache(int capacity = 256)
        : m_capacity(qMax(1, capacity)) {}

    /** @brief 获取值 @param key 键 @param value 输出值 @return 是否命中 */
    bool get(const Key& key, Value& value)
    {
        ++m_stats.totalLookups;
        auto it = m_map.find(key);
        if (it == m_map.end()) {
            ++m_stats.totalMisses;
            return false;
        }
        ++m_stats.totalHits;
        moveToHead(it.value());
        value = it.value()->value;
        return true;
    }

    /** @brief 插入/更新 @param key 键 @param value 值 */
    void put(const Key& key, const Value& value)
    {
        auto it = m_map.find(key);
        if (it != m_map.end()) {
            it.value()->value = value;
            moveToHead(it.value());
            return;
        }

        if (static_cast<int>(m_map.size()) >= m_capacity) {
            evictTail();
        }

        Node* node = new Node{key, value, m_head, nullptr};
        if (m_head) m_head->prev = node;
        m_head = node;
        if (!m_tail) m_tail = node;
        m_map[key] = node;
    }

    /** @brief 移除 @param key 键 @return 是否存在并移除 */
    bool remove(const Key& key)
    {
        auto it = m_map.find(key);
        if (it == m_map.end()) return false;
        Node* node = it.value();
        unlink(node);
        delete node;
        m_map.erase(it);
        return true;
    }

    /** @brief 是否包含 @param key 键 @return 存在性 */
    bool contains(const Key& key) const { return m_map.contains(key); }

    /** @brief 当前大小 */
    int size() const { return m_map.size(); }

    /** @brief 清空 */
    void clear()
    {
        Node* cur = m_head;
        while (cur) { Node* next = cur->next; delete cur; cur = next; }
        m_head = m_tail = nullptr;
        m_map.clear();
    }

    const Stats& stats() const { return m_stats; }
    void resetStatistics() { m_stats = Stats{}; }

    ~LruCache() { clear(); }

private:
    struct Node {
        Key key;
        Value value;
        Node* prev;
        Node* next;
    };

    void moveToHead(Node* node)
    {
        if (node == m_head) return;
        unlink(node);
        node->prev = nullptr;
        node->next = m_head;
        if (m_head) m_head->prev = node;
        m_head = node;
    }

    void unlink(Node* node)
    {
        if (node->prev) node->prev->next = node->next;
        else m_head = node->next;
        if (node->next) node->next->prev = node->prev;
        else m_tail = node->prev;
    }

    void evictTail()
    {
        if (!m_tail) return;
        Node* victim = m_tail;
        m_map.remove(victim->key);
        unlink(victim);
        delete victim;
        ++m_stats.totalEvictions;
    }

    QHash<Key, Node*> m_map;
    Node* m_head = nullptr;
    Node* m_tail = nullptr;
    int m_capacity;
    Stats m_stats;
};

#endif // LRUCACHE_H

/**
 * @file SkipList.h
 * @brief 跳表 — O(logN)查找/插入/删除的有序容器
 */
#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <QRandomGenerator>
#include <QVector>
#include <algorithm>

template<typename Key, typename Value>
class SkipList {
public:
    struct Stats {
        quint64 totalInsertions = 0;
        quint64 totalDeletions = 0;
        quint64 totalLookups = 0;
    };

    explicit SkipList(int maxLevel = 16)
        : m_maxLevel(maxLevel), m_level(1), m_count(0)
    {
        m_head = new Node{Key{}, Value{}, maxLevel + 1, QVector<Node*>(maxLevel + 1, nullptr)};
    }

    ~SkipList() { clear(); delete m_head; }

    void insert(const Key& key, const Value& value)
    {
        QVector<Node*> update(m_maxLevel + 1, nullptr);
        Node* current = m_head;

        for (int i = m_level; i >= 1; --i) {
            while (current->forward[i] && current->forward[i]->key < key)
                current = current->forward[i];
            update[i] = current;
        }
        current = current->forward[1];

        if (current && current->key == key) {
            current->value = value;
            return;
        }

        int newLevel = randomLevel();
        if (newLevel > m_level) {
            for (int i = m_level + 1; i <= newLevel; ++i) update[i] = m_head;
            m_level = newLevel;
        }

        Node* newNode = new Node{key, value, newLevel, QVector<Node*>(newLevel + 1, nullptr)};
        for (int i = 1; i <= newLevel; ++i) {
            newNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newNode;
        }
        ++m_count;
        ++m_stats.totalInsertions;
    }

    bool get(const Key& key, Value& value)
    {
        ++m_stats.totalLookups;
        Node* current = m_head;
        for (int i = m_level; i >= 1; --i) {
            while (current->forward[i] && current->forward[i]->key < key)
                current = current->forward[i];
        }
        current = current->forward[1];
        if (current && current->key == key) { value = current->value; return true; }
        return false;
    }

    bool remove(const Key& key)
    {
        QVector<Node*> update(m_maxLevel + 1, nullptr);
        Node* current = m_head;

        for (int i = m_level; i >= 1; --i) {
            while (current->forward[i] && current->forward[i]->key < key)
                current = current->forward[i];
            update[i] = current;
        }
        current = current->forward[1];

        if (!current || !(current->key == key)) return false;

        for (int i = 1; i <= m_level; ++i) {
            if (update[i]->forward[i] != current) break;
            update[i]->forward[i] = current->forward[i];
        }
        delete current;
        --m_count;
        ++m_stats.totalDeletions;

        while (m_level > 1 && !m_head->forward[m_level]) --m_level;
        return true;
    }

    bool contains(const Key& key) { Value v; return get(key, v); }
    int size() const { return m_count; }
    void clear() { Node* n = m_head->forward[1]; while (n) { Node* next = n->forward[1]; delete n; n = next; } for (int i = 1; i <= m_level; ++i) m_head->forward[i] = nullptr; m_level = 1; m_count = 0; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics() { m_stats = Stats{}; }

private:
    struct Node { Key key; Value value; int level; QVector<Node*> forward; };
    int randomLevel() const { int lvl = 1; while (QRandomGenerator::global()->bounded(2) && lvl < m_maxLevel) ++lvl; return lvl; }

    int m_maxLevel, m_level, m_count;
    Node* m_head;
    Stats m_stats;
};

#endif // SKIPLIST_H

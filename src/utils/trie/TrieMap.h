/**
 * @file TrieMap.h
 * @brief Trie前缀树 — 字符串前缀匹配/自动补全
 */
#ifndef TRIEMAP_H
#define TRIEMAP_H

#include <QChar>
#include <QHash>
#include <QVector>
#include <QString>

template<typename Value>
class TrieMap {
public:
    struct Stats {
        quint64 totalInsertions = 0;
        quint64 totalLookups = 0;
        quint64 totalPrefixSearches = 0;
    };

    TrieMap() = default;

    void insert(const QString& key, const Value& value)
    {
        Node* node = &m_root;
        for (int i = 0; i < key.length(); ++i) {
            QChar c = key[i];
            if (!node->children.contains(c))
                node->children[c] = Node{};
            node = &node->children[c];
        }
        node->isEnd = true;
        node->value = value;
        ++m_stats.totalInsertions;
    }

    bool contains(const QString& key) const
    {
        const Node* node = findNode(key);
        return node && node->isEnd;
    }

    bool get(const QString& key, Value& value) const
    {
        const Node* node = findNode(key);
        if (node && node->isEnd) { value = node->value; return true; }
        return false;
    }

    QVector<QString> startsWith(const QString& prefix) const
    {
        QVector<QString> results;
        const Node* node = findNode(prefix);
        if (!node) return results;

        collectAll(node, prefix, results);
        ++m_stats.totalPrefixSearches;
        return results;
    }

    void clear() { m_root = Node{}; }
    int nodeCount() const { return countNodes(&m_root) - 1; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics() { m_stats = Stats{}; }

private:
    struct Node {
        QHash<QChar, Node> children;
        bool isEnd = false;
        Value value{};
    };

    const Node* findNode(const QString& key) const
    {
        const Node* node = &m_root;
        for (int i = 0; i < key.length(); ++i) {
            auto it = node->children.find(key[i]);
            if (it == node->children.end()) return nullptr;
            node = &it.value();
        }
        return node;
    }

    void collectAll(const Node* node, const QString& prefix,
                    QVector<QString>& results) const
    {
        if (node->isEnd) results.append(prefix);
        for (auto it = node->children.constBegin();
             it != node->children.constEnd(); ++it) {
            collectAll(&it.value(), prefix + it.key(), results);
        }
    }

    int countNodes(const Node* node) const
    {
        int count = 1;
        for (auto it = node->children.constBegin();
             it != node->children.constEnd(); ++it) {
            count += countNodes(&it.value());
        }
        return count;
    }

    Node m_root;
    Stats m_stats;
};

#endif // TRIEMAP_H

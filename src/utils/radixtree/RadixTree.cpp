/**
 * @file RadixTree.cpp
 * @brief 基数树(Patricia Trie)实现
 */

#include "utils/radixtree/RadixTree.h"

#include <QElapsedTimer>
#include <algorithm>

RadixTree::RadixTree(QObject* parent)
    : QObject(parent), m_size(0), m_timeSum(0.0) {}

RadixTree::~RadixTree() { clearNode(&m_root); }

void RadixTree::clearNode(Node* node)
{
    for (auto it = node->children.begin(); it != node->children.end(); ++it)
        clearNode(it.value());
    node->children.clear();
}

void RadixTree::insert(const QString& key, int value)
{
    QElapsedTimer timer;
    timer.start();

    Node* cur = &m_root;
    int i = 0;

    while (i < key.length()) {
        QChar c = key[i];
        auto it = cur->children.find(c);

        if (it == cur->children.end()) {
            /* 无匹配子节点，创建新分支 */
            auto* child = new Node();
            child->edgeLabel = key.mid(i);
            child->value = value;
            child->isTerminal = true;
            child->parent = cur;
            cur->children[c] = child;
            ++m_size;

            m_stats.totalInserts++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum /
                qMax(m_stats.totalInserts + m_stats.totalRemoves +
                     m_stats.totalLookups, 1ULL);
            emit entryInserted(key);
            return;
        }

        Node* child = it.value();
        int j = 0;
        int edgeLen = child->edgeLabel.length();

        /* 找公共前缀长度 */
        while (j < edgeLen && i < key.length() &&
               child->edgeLabel[j] == key[i]) {
            ++j; ++i;
        }

        if (j == edgeLen) {
            /* 完全匹配边标签，继续向下 */
            cur = child;
        } else {
            /* 分裂节点 */
            auto* split = new Node();
            split->edgeLabel = child->edgeLabel.mid(0, j);
            split->parent = cur;

            child->edgeLabel = child->edgeLabel.mid(j);
            child->parent = split;

            cur->children[c] = split;
            split->children[child->edgeLabel[0]] = child;

            if (i == key.length()) {
                /* 键在分裂点结束 */
                split->value = value;
                split->isTerminal = true;
                ++m_size;
            } else {
                /* 剩余键创建新节点 */
                auto* leaf = new Node();
                leaf->edgeLabel = key.mid(i);
                leaf->value = value;
                leaf->isTerminal = true;
                leaf->parent = split;
                split->children[key[i]] = leaf;
                ++m_size;
            }

            m_stats.totalInserts++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum /
                qMax(m_stats.totalInserts + m_stats.totalRemoves +
                     m_stats.totalLookups, 1ULL);
            emit entryInserted(key);
            return;
        }
    }

    /* 到达已有节点 */
    if (!cur->isTerminal) ++m_size;
    cur->value = value;
    cur->isTerminal = true;

    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalInserts + m_stats.totalRemoves +
             m_stats.totalLookups, 1ULL);
    emit entryInserted(key);
}

bool RadixTree::remove(const QString& key)
{
    QElapsedTimer timer;
    timer.start();

    Node* cur = &m_root;
    int i = 0;

    while (i < key.length()) {
        QChar c = key[i];
        auto it = cur->children.find(c);
        if (it == cur->children.end()) {
            m_stats.totalRemoves++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum /
                qMax(m_stats.totalInserts + m_stats.totalRemoves +
                     m_stats.totalLookups, 1ULL);
            return false;
        }

        Node* child = it.value();
        int edgeLen = child->edgeLabel.length();

        /* 检查键是否匹配边标签 */
        if (i + edgeLen > key.length()) {
            m_stats.totalRemoves++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum /
                qMax(m_stats.totalInserts + m_stats.totalRemoves +
                     m_stats.totalLookups, 1ULL);
            return false;
        }
        for (int j = 0; j < edgeLen; ++j) {
            if (child->edgeLabel[j] != key[i + j]) {
                m_stats.totalRemoves++;
                m_timeSum += timer.elapsed();
                m_stats.avgProcessingTimeMs = m_timeSum /
                    qMax(m_stats.totalInserts + m_stats.totalRemoves +
                         m_stats.totalLookups, 1ULL);
                return false;
            }
        }
        i += edgeLen;
        cur = child;
    }

    if (!cur->isTerminal) {
        m_stats.totalRemoves++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum /
            qMax(m_stats.totalInserts + m_stats.totalRemoves +
                 m_stats.totalLookups, 1ULL);
        return false;
    }

    cur->isTerminal = false;
    --m_size;

    /* 合并: 若非终端节点只有一个子节点，合并边 */
    if (cur->children.size() == 1 && !cur->isTerminal) {
        auto childIt = cur->children.begin();
        Node* grandchild = childIt.value();
        cur->edgeLabel += grandchild->edgeLabel;
        cur->value = grandchild->value;
        cur->isTerminal = grandchild->isTerminal;
        cur->children = grandchild->children;
        for (auto& gc : cur->children) gc->parent = cur;
        grandchild->children.clear();
        delete grandchild;
    } else if (cur->children.isEmpty() && !cur->isTerminal) {
        /* 叶子节点: 从父节点移除 */
        if (cur->parent) {
            for (auto it = cur->parent->children.begin();
                 it != cur->parent->children.end(); ++it) {
                if (it.value() == cur) {
                    cur->parent->children.erase(it);
                    break;
                }
            }
            delete cur;
        }
    }

    m_stats.totalRemoves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalInserts + m_stats.totalRemoves +
             m_stats.totalLookups, 1ULL);
    return true;
}

bool RadixTree::lookup(const QString& key, int& value)
{
    QElapsedTimer timer;
    timer.start();

    Node* cur = &m_root;
    int i = 0;

    while (i < key.length()) {
        QChar c = key[i];
        auto it = cur->children.find(c);
        if (it == cur->children.end()) {
            m_stats.totalLookups++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum /
                qMax(m_stats.totalInserts + m_stats.totalRemoves +
                     m_stats.totalLookups, 1ULL);
            return false;
        }

        Node* child = it.value();
        int edgeLen = child->edgeLabel.length();
        if (i + edgeLen > key.length()) {
            m_stats.totalLookups++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum /
                qMax(m_stats.totalInserts + m_stats.totalRemoves +
                     m_stats.totalLookups, 1ULL);
            return false;
        }
        for (int j = 0; j < edgeLen; ++j) {
            if (child->edgeLabel[j] != key[i + j]) {
                m_stats.totalLookups++;
                m_timeSum += timer.elapsed();
                m_stats.avgProcessingTimeMs = m_timeSum /
                    qMax(m_stats.totalInserts + m_stats.totalRemoves +
                         m_stats.totalLookups, 1ULL);
                return false;
            }
        }
        i += edgeLen;
        cur = child;
    }

    if (cur->isTerminal) {
        value = cur->value;
        m_stats.totalLookups++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum /
            qMax(m_stats.totalInserts + m_stats.totalRemoves +
                 m_stats.totalLookups, 1ULL);
        return true;
    }

    m_stats.totalLookups++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalInserts + m_stats.totalRemoves +
             m_stats.totalLookups, 1ULL);
    return false;
}

QVector<QPair<QString, int>> RadixTree::startsWith(const QString& prefix)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<QString, int>> results;
    Node* cur = &m_root;
    int i = 0;

    /* 沿前缀向下查找 */
    while (i < prefix.length()) {
        QChar c = prefix[i];
        auto it = cur->children.find(c);
        if (it == cur->children.end()) {
            m_stats.totalLookups++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum /
                qMax(m_stats.totalInserts + m_stats.totalRemoves +
                     m_stats.totalLookups, 1ULL);
            return results;
        }
        Node* child = it.value();
        int edgeLen = child->edgeLabel.length();
        for (int j = 0; j < edgeLen && i < prefix.length(); ++j, ++i) {
            if (child->edgeLabel[j] != prefix[i]) {
                m_stats.totalLookups++;
                m_timeSum += timer.elapsed();
                m_stats.avgProcessingTimeMs = m_timeSum /
                    qMax(m_stats.totalInserts + m_stats.totalRemoves +
                         m_stats.totalLookups, 1ULL);
                return results;
            }
        }
        if (i >= prefix.length()) { cur = child; break; }
        cur = child;
    }

    /* 收集所有匹配 */
    collectAll(cur, prefix, results);

    m_stats.totalLookups++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalInserts + m_stats.totalRemoves +
             m_stats.totalLookups, 1ULL);
    return results;
}

void RadixTree::collectAll(Node* node, const QString& prefix,
                           QVector<QPair<QString, int>>& results)
{
    if (node->isTerminal)
        results.append({prefix, node->value});
    for (auto it = node->children.begin(); it != node->children.end(); ++it)
        collectAll(it.value(), prefix + it.value()->edgeLabel, results);
}

void RadixTree::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

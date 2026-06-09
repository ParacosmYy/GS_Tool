/**
 * @file Treap12.cpp
 * @brief Treap12 实现
 *
 * 实现树堆：哈希确定性优先级与分裂合并隐式键索引。
 */

#include "utils/tree243/Treap12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Treap12::Treap12(QObject *parent) : QObject(parent) {}
Treap12::~Treap12() = default;

/* ---- Hash-based deterministic priority ---- */

quint64 Treap12::hashPriority(double key, int value)
{
    // FNV-1a style hash combining key bits and value
    quint64 h = 14695981039346656037ULL;
    quint64 keyBits = 0;
    memcpy(&keyBits, &key, qMin(sizeof(keyBits), sizeof(key)));
    h ^= keyBits & 0xFF;   h *= 1099511628211ULL;
    h ^= (keyBits >> 8);   h *= 1099511628211ULL;
    h ^= (keyBits >> 16);  h *= 1099511628211ULL;
    h ^= (keyBits >> 24);  h *= 1099511628211ULL;
    h ^= (keyBits >> 32);  h *= 1099511628211ULL;
    h ^= static_cast<quint64>(value) & 0xFF; h *= 1099511628211ULL;
    h ^= static_cast<quint64>(value) >> 8;   h *= 1099511628211ULL;
    return h;
}

/* ---- Allocate node ---- */

int Treap12::allocNode(double key, int value)
{
    Node n;
    n.key = key;
    n.value = value;
    n.priority = hashPriority(key, value);
    n.left = -1;
    n.right = -1;
    n.size = 1;
    m_nodes.append(n);
    return m_nodes.size() - 1;
}

/* ---- Update / Get size ---- */

void Treap12::updateSize(int idx)
{
    if (idx < 0) return;
    m_nodes[idx].size = 1 + getSize(m_nodes[idx].left) + getSize(m_nodes[idx].right);
}

int Treap12::getSize(int idx) const
{
    return (idx >= 0) ? m_nodes[idx].size : 0;
}

/* ---- Split by implicit position ---- */

void Treap12::split(int idx, int pos, int& left, int& right)
{
    if (idx < 0) { left = -1; right = -1; return; }

    int leftSize = getSize(m_nodes[idx].left);
    if (leftSize < pos) {
        // Split point is in right subtree
        split(m_nodes[idx].right, pos - leftSize - 1, m_nodes[idx].right, right);
        left = idx;
    } else {
        // Split point is in left subtree
        split(m_nodes[idx].left, pos, left, m_nodes[idx].left);
        right = idx;
    }
    updateSize(idx);
    m_stats.numSplits++;
}

/* ---- Merge by priority ---- */

int Treap12::merge(int left, int right)
{
    if (left < 0) return right;
    if (right < 0) return left;

    m_stats.numMerges++;

    if (m_nodes[left].priority > m_nodes[right].priority) {
        m_nodes[left].right = merge(m_nodes[left].right, right);
        updateSize(left);
        return left;
    } else {
        m_nodes[right].left = merge(left, m_nodes[right].left);
        updateSize(right);
        return right;
    }
}

/* ---- Insert at implicit position ---- */

void Treap12::insertAt(int pos, double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    int nodeIdx = allocNode(key, value);
    int left = -1, right = -1;
    split(m_root, pos, left, right);
    m_root = merge(merge(left, nodeIdx), right);

    m_stats.treeSize = getSize(m_root);
    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit insertCompleted(key, m_stats.treeSize, timer.elapsed());
    emit splitMergePerformed(m_stats.treeSize, m_stats.treeHeight);
}

/* ---- Insert maintaining key order ---- */

void Treap12::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    int nodeIdx = allocNode(key, value);

    // Find position by key order
    int pos = 0;
    int cur = m_root;
    while (cur >= 0) {
        if (key < m_nodes[cur].key) {
            cur = m_nodes[cur].left;
        } else {
            pos += getSize(m_nodes[cur].left) + 1;
            cur = m_nodes[cur].right;
        }
    }

    int left = -1, right = -1;
    split(m_root, pos, left, right);
    m_root = merge(merge(left, nodeIdx), right);

    m_stats.treeSize = getSize(m_root);
    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit insertCompleted(key, m_stats.treeSize, timer.elapsed());
}

/* ---- Remove at implicit position ---- */

bool Treap12::removeAt(int pos)
{
    if (pos < 0 || pos >= getSize(m_root)) return false;

    int left = -1, mid = -1, right = -1;
    split(m_root, pos, left, right);
    split(right, 1, mid, right);
    m_root = merge(left, right);

    m_stats.treeSize = getSize(m_root);
    m_stats.treeHeight = height();
    m_stats.totalOps++;
    return true;
}

/* ---- Remove by key ---- */

bool Treap12::remove(double key)
{
    int pos = rankOf(key);
    if (pos < 0) return false;
    return removeAt(pos);
}

/* ---- Search by key ---- */

int Treap12::search(double key) const
{
    int cur = m_root;
    while (cur >= 0) {
        if (key < m_nodes[cur].key) cur = m_nodes[cur].left;
        else if (key > m_nodes[cur].key) cur = m_nodes[cur].right;
        else return m_nodes[cur].value;
    }
    return -1;
}

/* ---- Access at implicit index ---- */

QPair<double, int> Treap12::atIndex(int idx) const
{
    if (idx < 0 || idx >= getSize(m_root)) return {0.0, -1};

    int cur = m_root;
    while (cur >= 0) {
        int leftSize = getSize(m_nodes[cur].left);
        if (idx < leftSize) {
            cur = m_nodes[cur].left;
        } else if (idx == leftSize) {
            return {m_nodes[cur].key, m_nodes[cur].value};
        } else {
            idx -= leftSize + 1;
            cur = m_nodes[cur].right;
        }
    }
    return {0.0, -1};
}

/* ---- Rank of key ---- */

int Treap12::rankOf(double key) const
{
    int rank = 0;
    int cur = m_root;
    while (cur >= 0) {
        if (key < m_nodes[cur].key) {
            cur = m_nodes[cur].left;
        } else if (key > m_nodes[cur].key) {
            rank += getSize(m_nodes[cur].left) + 1;
            cur = m_nodes[cur].right;
        } else {
            rank += getSize(m_nodes[cur].left);
            return rank;
        }
    }
    return -1;
}

/* ---- In-order traversal ---- */

void Treap12::collectInOrder(int idx, QVector<int>& indices) const
{
    if (idx < 0) return;
    collectInOrder(m_nodes[idx].left, indices);
    indices.append(idx);
    collectInOrder(m_nodes[idx].right, indices);
}

QVector<QPair<double, int>> Treap12::inOrder() const
{
    QVector<int> indices;
    collectInOrder(m_root, indices);
    QVector<QPair<double, int>> result;
    result.reserve(indices.size());
    for (int i : indices) result.append({m_nodes[i].key, m_nodes[i].value});
    return result;
}

/* ---- Size / Height ---- */

int Treap12::size() const { return getSize(m_root); }

int Treap12::computeHeight(int idx) const
{
    if (idx < 0) return 0;
    return 1 + qMax(computeHeight(m_nodes[idx].left), computeHeight(m_nodes[idx].right));
}

int Treap12::height() const { return computeHeight(m_root); }

/* ---- Reset ---- */

void Treap12::resetStatistics()
{
    m_nodes.clear();
    m_root = -1;
    m_stats = Stats{}; m_timeSum = 0.0;
}

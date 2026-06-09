/**
 * @file WeightBalancedTree8.cpp
 * @brief WeightBalancedTree8 实现
 *
 * 实现权重平衡树：秩平衡重平衡与部分重建O(log n)最坏情况操作。
 */

#include "utils/tree242/WeightBalancedTree8.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WeightBalancedTree8::WeightBalancedTree8(QObject *parent) : QObject(parent) {}
WeightBalancedTree8::~WeightBalancedTree8() = default;

/* ---- Allocate node ---- */

int WeightBalancedTree8::allocNode(double key, int value)
{
    Node n;
    n.key = key;
    n.value = value;
    n.left = -1;
    n.right = -1;
    n.weight = 1;
    m_nodes.append(n);
    return m_nodes.size() - 1;
}

/* ---- Update weight ---- */

void WeightBalancedTree8::updateWeight(int idx)
{
    if (idx < 0) return;
    int w = 1;
    if (m_nodes[idx].left >= 0) w += m_nodes[m_nodes[idx].left].weight;
    if (m_nodes[idx].right >= 0) w += m_nodes[m_nodes[idx].right].weight;
    m_nodes[idx].weight = w;
}

/* ---- Check balance ---- */

bool WeightBalancedTree8::isBalanced(int idx) const
{
    if (idx < 0) return true;
    int lw = (m_nodes[idx].left >= 0) ? m_nodes[m_nodes[idx].left].weight : 0;
    int rw = (m_nodes[idx].right >= 0) ? m_nodes[m_nodes[idx].right].weight : 0;
    int total = lw + rw + 1;
    if (total <= 2) return true;
    // Balance condition: no child weight > (1 - alpha) * total
    int threshold = static_cast<int>((1.0 - kAlpha) * total);
    return lw <= threshold && rw <= threshold;
}

/* ---- Collect in-order ---- */

void WeightBalancedTree8::collectInOrder(int idx, QVector<int>& indices) const
{
    if (idx < 0) return;
    collectInOrder(m_nodes[idx].left, indices);
    indices.append(idx);
    collectInOrder(m_nodes[idx].right, indices);
}

/* ---- Build balanced from sorted ---- */

int WeightBalancedTree8::buildBalanced(const QVector<int>& indices, int lo, int hi)
{
    if (lo > hi) return -1;
    int mid = (lo + hi) / 2;
    int idx = indices[mid];
    m_nodes[idx].left = buildBalanced(indices, lo, mid - 1);
    m_nodes[idx].right = buildBalanced(indices, mid + 1, hi);
    updateWeight(idx);
    return idx;
}

/* ---- Partial rebuild ---- */

int WeightBalancedTree8::partialRebuild(int idx)
{
    QVector<int> indices;
    collectInOrder(idx, indices);
    m_stats.numPartialRebuilds++;
    emit rebalancePerformed(indices.size(), 0);
    return buildBalanced(indices, 0, indices.size() - 1);
}

/* ---- Rebalance ---- */

int WeightBalancedTree8::rebalance(int idx)
{
    if (!isBalanced(idx)) {
        idx = partialRebuild(idx);
        m_stats.numRebalances++;
    }
    return idx;
}

/* ---- Insert helper ---- */

int WeightBalancedTree8::insertHelper(int idx, double key, int value, bool& inserted)
{
    if (idx < 0) {
        inserted = true;
        return allocNode(key, value);
    }
    if (key < m_nodes[idx].key) {
        m_nodes[idx].left = insertHelper(m_nodes[idx].left, key, value, inserted);
    } else if (key > m_nodes[idx].key) {
        m_nodes[idx].right = insertHelper(m_nodes[idx].right, key, value, inserted);
    } else {
        m_nodes[idx].value = value;
        inserted = false;
        return idx;
    }
    updateWeight(idx);
    return rebalance(idx);
}

/* ---- Find min ---- */

int WeightBalancedTree8::findMin(int idx) const
{
    while (idx >= 0 && m_nodes[idx].left >= 0)
        idx = m_nodes[idx].left;
    return idx;
}

/* ---- Remove helper ---- */

int WeightBalancedTree8::removeHelper(int idx, double key, bool& removed)
{
    if (idx < 0) { removed = false; return -1; }
    if (key < m_nodes[idx].key) {
        m_nodes[idx].left = removeHelper(m_nodes[idx].left, key, removed);
    } else if (key > m_nodes[idx].key) {
        m_nodes[idx].right = removeHelper(m_nodes[idx].right, key, removed);
    } else {
        removed = true;
        if (m_nodes[idx].left < 0) return m_nodes[idx].right;
        if (m_nodes[idx].right < 0) return m_nodes[idx].left;
        int succ = findMin(m_nodes[idx].right);
        m_nodes[idx].key = m_nodes[succ].key;
        m_nodes[idx].value = m_nodes[succ].value;
        bool dummy = false;
        m_nodes[idx].right = removeHelper(m_nodes[idx].right, m_nodes[succ].key, dummy);
    }
    updateWeight(idx);
    return rebalance(idx);
}

/* ---- Public: Insert ---- */

void WeightBalancedTree8::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    bool inserted = false;
    m_root = insertHelper(m_root, key, value, inserted);

    m_stats.treeSize = (m_root >= 0) ? m_nodes[m_root].weight : 0;
    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit insertCompleted(key, m_stats.treeHeight, timer.elapsed());
}

/* ---- Public: Remove ---- */

bool WeightBalancedTree8::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    bool removed = false;
    m_root = removeHelper(m_root, key, removed);

    m_stats.treeSize = (m_root >= 0) ? m_nodes[m_root].weight : 0;
    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return removed;
}

/* ---- Public: Search ---- */

int WeightBalancedTree8::search(double key) const
{
    int idx = m_root;
    while (idx >= 0) {
        if (key < m_nodes[idx].key) idx = m_nodes[idx].left;
        else if (key > m_nodes[idx].key) idx = m_nodes[idx].right;
        else return m_nodes[idx].value;
    }
    return -1;
}

/* ---- In-order keys ---- */

QVector<double> WeightBalancedTree8::inOrderKeys() const
{
    QVector<int> indices;
    collectInOrder(m_root, indices);
    QVector<double> keys;
    keys.reserve(indices.size());
    for (int i : indices) keys.append(m_nodes[i].key);
    return keys;
}

/* ---- Compute height ---- */

int WeightBalancedTree8::computeHeight(int idx) const
{
    if (idx < 0) return 0;
    int lh = computeHeight(m_nodes[idx].left);
    int rh = computeHeight(m_nodes[idx].right);
    return 1 + qMax(lh, rh);
}

int WeightBalancedTree8::height() const
{
    return computeHeight(m_root);
}

/* ---- Reset ---- */

void WeightBalancedTree8::resetStatistics()
{
    m_nodes.clear();
    m_root = -1;
    m_stats = Stats{}; m_timeSum = 0.0;
}

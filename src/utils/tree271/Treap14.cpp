/**
 * @file Treap14.cpp
 * @brief Treap14 实现
 *
 * 实现树堆：随机堆优先级与分裂合并操作高效有序集范围查询。
 */

#include "utils/tree271/Treap14.h"

#include <QElapsedTimer>
#include <QtGlobal>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Treap14::Treap14(QObject *parent)
    : QObject(parent) {}

Treap14::~Treap14() = default;

/* ---- Node allocation ---- */

int Treap14::allocNode(double key, int value)
{
    int idx;
    if (!m_freeList.isEmpty()) {
        idx = m_freeList.takeLast();
        m_nodes[idx] = {key, value, qrand(), 1, -1, -1};
    } else {
        idx = m_nodes.size();
        m_nodes.append({key, value, qrand(), 1, -1, -1});
    }
    return idx;
}

void Treap14::freeNode(int idx)
{
    m_freeList.append(idx);
}

/* ---- Subtree size helpers ---- */

int Treap14::nodeSize(int idx) const
{
    return (idx >= 0) ? m_nodes[idx].size : 0;
}

void Treap14::updateSize(int idx)
{
    if (idx < 0) return;
    m_nodes[idx].size = 1 + nodeSize(m_nodes[idx].left) + nodeSize(m_nodes[idx].right);
}

/* ---- Split: keys < key go left, keys >= key go right ---- */

QPair<int, int> Treap14::split(int idx, double key)
{
    if (idx < 0) return {-1, -1};

    if (m_nodes[idx].key < key) {
        // Current node and its left subtree go to left part
        auto [lr, rr] = split(m_nodes[idx].right, key);
        m_nodes[idx].right = lr;
        updateSize(idx);
        return {idx, rr};
    } else {
        // Current node and its right subtree go to right part
        auto [lr, rr] = split(m_nodes[idx].left, key);
        m_nodes[idx].left = rr;
        updateSize(idx);
        return {lr, idx};
    }
}

/* ---- Merge: all keys in a < all keys in b ---- */

int Treap14::merge(int a, int b)
{
    if (a < 0) return b;
    if (b < 0) return a;

    // Heap property: higher priority at root
    if (m_nodes[a].priority > m_nodes[b].priority) {
        m_nodes[a].right = merge(m_nodes[a].right, b);
        updateSize(a);
        return a;
    } else {
        m_nodes[b].left = merge(a, m_nodes[b].left);
        updateSize(b);
        return b;
    }
}

/* ---- Height ---- */

int Treap14::heightHelper(int idx) const
{
    if (idx < 0) return 0;
    return 1 + qMax(heightHelper(m_nodes[idx].left), heightHelper(m_nodes[idx].right));
}

/* ---- In-order traversal ---- */

void Treap14::inOrderHelper(int idx, QVector<double>& result) const
{
    if (idx < 0) return;
    inOrderHelper(m_nodes[idx].left, result);
    result.append(m_nodes[idx].key);
    inOrderHelper(m_nodes[idx].right, result);
}

/* ---- Rank helper ---- */

int Treap14::rankHelper(int idx, double key) const
{
    if (idx < 0) return 0;
    if (m_nodes[idx].key < key)
        return 1 + nodeSize(m_nodes[idx].left) + rankHelper(m_nodes[idx].right, key);
    return rankHelper(m_nodes[idx].left, key);
}

/* ---- Public: Insert ---- */

void Treap14::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    // Split by key, then merge: left + new_node + right_ge
    auto [left, right] = split(m_root, key);

    // Check if key already exists in right subtree (first key >= key)
    int existing = right;
    while (existing >= 0 && m_nodes[existing].key == key) {
        // Update existing value
        m_nodes[existing].value = value;
        m_root = merge(left, right);
        return;
    }

    int newNode = allocNode(key, value);
    m_root = merge(merge(left, newNode), right);

    double elapsed = timer.elapsed();
    m_stats.treeSize = size();
    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_stats.treeSize, m_stats.treeHeight, elapsed);
}

/* ---- Public: Remove ---- */

bool Treap14::remove(double key)
{
    QElapsedTimer timer;
    timer.start();
    int oldSize = size();

    // Split into: (< key), (>= key)
    auto [left, midRight] = split(m_root, key);
    // Split midRight into: ([key]), (> key)
    auto [mid, right] = split(midRight, key + 1e-15);

    // Free all nodes in mid (keys in [key, key+eps))
    int cur = mid;
    QVector<int> toFree;
    while (cur >= 0) {
        if (m_nodes[cur].key == key)
            toFree.append(cur);
        cur = m_nodes[cur].right;
    }
    // Remove exact key node from mid
    if (mid >= 0) {
        // Find and remove the exact match
        auto [exactLeft, exactRight] = split(mid, key);
        if (exactRight >= 0 && m_nodes[exactRight].key == key) {
            freeNode(exactRight);
            mid = merge(exactLeft, m_nodes[exactRight].right);
        } else {
            mid = merge(exactLeft, exactRight);
        }
    }

    // Merge all remaining pieces
    // Collect mid's remaining nodes back
    m_root = merge(left, merge(mid, right));

    // Actually simpler approach: just rebuild from split + skip exact
    // Re-do with cleaner approach
    auto [l1, r1] = split(m_root, key);
    auto [l2, r2] = split(r1, key + 1e-15);
    // l2 contains nodes with keys in [key, key+eps)
    // Remove exact match from l2
    bool found = (l2 >= 0);
    if (found) freeNode(l2);
    m_root = merge(l1, merge(l2 >= 0 ? -1 : l2, r2));

    // Simpler: just do the merge skipping the mid tree
    // Restore from original splits
    m_root = merge(left, right);

    int newSize = size();

    double elapsed = timer.elapsed();
    m_stats.treeSize = newSize;
    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_stats.treeSize, m_stats.treeHeight, elapsed);

    return newSize < oldSize;
}

/* ---- Public: Search ---- */

int Treap14::search(double key) const
{
    int idx = m_root;
    while (idx >= 0) {
        if (key < m_nodes[idx].key)
            idx = m_nodes[idx].left;
        else if (key > m_nodes[idx].key)
            idx = m_nodes[idx].right;
        else
            return m_nodes[idx].value;
    }
    return -1;
}

/* ---- Public: K-th smallest ---- */

double Treap14::kth(int k) const
{
    int idx = m_root;
    while (idx >= 0) {
        int leftSz = nodeSize(m_nodes[idx].left);
        if (k < leftSz) {
            idx = m_nodes[idx].left;
        } else if (k == leftSz) {
            return m_nodes[idx].key;
        } else {
            k -= leftSz + 1;
            idx = m_nodes[idx].right;
        }
    }
    return 0.0; // not found
}

/* ---- Public: Rank ---- */

int Treap14::rank(double key) const
{
    return rankHelper(m_root, key);
}

/* ---- Public: Range count ---- */

int Treap14::rangeCount(double lo, double hi) const
{
    // Count keys in [lo, hi] = rank(hi+eps) - rank(lo)
    return rankHelper(m_root, hi + 1e-15) - rankHelper(m_root, lo);
}

/* ---- Public: In-order ---- */

QVector<double> Treap14::inOrderKeys() const
{
    QVector<double> result;
    inOrderHelper(m_root, result);
    return result;
}

/* ---- Public: Height / Size ---- */

int Treap14::height() const { return heightHelper(m_root); }
int Treap14::size() const { return nodeSize(m_root); }

/* ---- Reset ---- */

void Treap14::resetStatistics()
{
    m_root = -1;
    m_nodes.clear();
    m_freeList.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

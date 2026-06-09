/**
 * @file Treap13.cpp
 * @brief Treap13 实现
 *
 * 实现树堆：Zip树优先级几何分布与合并-分裂区间操作。
 */

#include "utils/tree257/Treap13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/* ---- Construction / Destruction ---- */

Treap13::Treap13(QObject *parent)
    : QObject(parent) {}
Treap13::~Treap13() = default;

/* ---- Configuration ---- */

void Treap13::setGeoParam(double p) { m_geoP = qBound(0.1, p, 0.9); }

/* ---- Generate geometric-distribution priority ---- */

int Treap13::generatePriority() const
{
    // Geometric distribution: number of failures before first success
    // P(X=k) = (1-p)^k * p, using inverse CDF
    static std::mt19937 rng(42);
    std::geometric_distribution<int> dist(m_geoP);
    return dist(rng);
}

/* ---- Node allocation ---- */

int Treap13::allocNode(int key, int priority)
{
    int idx;
    if (m_freeList >= 0) {
        idx = m_freeList;
        m_freeList = m_nodes[idx].right; // Reuse right as next-free link
    } else {
        idx = m_nodes.size();
        m_nodes.append(Node());
    }
    m_nodes[idx].key = key;
    m_nodes[idx].priority = priority;
    m_nodes[idx].size = 1;
    m_nodes[idx].left = -1;
    m_nodes[idx].right = -1;
    return idx;
}

/* ---- Free node ---- */

void Treap13::freeNode(int idx)
{
    m_nodes[idx].right = m_freeList;
    m_freeList = idx;
}

/* ---- Node size (safe) ---- */

int Treap13::nodeSize(int idx) const
{
    return (idx >= 0 && idx < m_nodes.size()) ? m_nodes[idx].size : 0;
}

/* ---- Update subtree size ---- */

void Treap13::updateSize(int idx)
{
    if (idx < 0) return;
    m_nodes[idx].size = 1 + nodeSize(m_nodes[idx].left)
                            + nodeSize(m_nodes[idx].right);
}

/* ---- Merge two treaps (all keys in a < all keys in b) ---- */

int Treap13::merge(int a, int b)
{
    if (a < 0) return b;
    if (b < 0) return a;

    // Zip-tree merge: higher priority becomes root
    if (m_nodes[a].priority >= m_nodes[b].priority) {
        m_nodes[a].right = merge(m_nodes[a].right, b);
        updateSize(a);
        return a;
    } else {
        m_nodes[b].left = merge(a, m_nodes[b].left);
        updateSize(b);
        return b;
    }
}

/* ---- Split at key ---- */

void Treap13::split(int root, int key, int& left, int& right)
{
    if (root < 0) {
        left = -1;
        right = -1;
        return;
    }

    if (key <= m_nodes[root].key) {
        split(m_nodes[root].left, key, left, m_nodes[root].left);
        right = root;
        updateSize(right);
    } else {
        split(m_nodes[root].right, key, m_nodes[root].right, right);
        left = root;
        updateSize(left);
    }
}

/* ---- Insert ---- */

void Treap13::insert(int key)
{
    QElapsedTimer timer;
    timer.start();

    int priority = generatePriority();
    int newIdx = allocNode(key, priority);

    // Split tree at key
    int left, right;
    split(m_root, key, left, right);

    // Merge left + new node
    int merged = merge(left, newIdx);

    // Merge result + right
    m_root = merge(merged, right);

    m_stats.numInserts++;
    m_stats.numNodes = nodeSize(m_root);
    m_stats.totalOps++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    int h = 0;
    if (m_root >= 0) h = heightHelper(m_root);
    emit treeUpdated(m_stats.numNodes, h, elapsed);
}

/* ---- Remove ---- */

void Treap13::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    // Split into <key and >=key
    int left, right;
    split(m_root, key, left, right);

    // Split right into ==key and >key
    int mid, gt;
    split(right, key + 1, mid, gt);

    // Skip the first node in mid (the one with key)
    if (mid >= 0) {
        // Merge left and right subtrees of mid (skip root)
        int midChildren = merge(m_nodes[mid].left, m_nodes[mid].right);
        freeNode(mid);
        mid = midChildren;
    }

    // Merge all parts
    m_root = merge(merge(left, mid), gt);

    m_stats.numDeletes++;
    m_stats.numNodes = nodeSize(m_root);
    m_stats.totalOps++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    int h = 0;
    if (m_root >= 0) h = heightHelper(m_root);
    emit treeUpdated(m_stats.numNodes, h, elapsed);
}

/* ---- Contains ---- */

bool Treap13::contains(int key) const
{
    return find(key) >= 0;
}

/* ---- Find key ---- */

int Treap13::find(int key) const
{
    int idx = m_root;
    while (idx >= 0) {
        if (key == m_nodes[idx].key) return idx;
        idx = (key < m_nodes[idx].key) ? m_nodes[idx].left : m_nodes[idx].right;
    }
    return -1;
}

/* ---- Range query ---- */

void Treap13::rangeQuery(int idx, int lo, int hi, QVector<int>& result) const
{
    if (idx < 0) return;
    if (m_nodes[idx].key > lo)
        rangeQuery(m_nodes[idx].left, lo, hi, result);
    if (m_nodes[idx].key >= lo && m_nodes[idx].key <= hi)
        result.append(m_nodes[idx].key);
    if (m_nodes[idx].key < hi)
        rangeQuery(m_nodes[idx].right, lo, hi, result);
}

Treap13::IntervalResult Treap13::queryRange(int lo, int hi) const
{
    IntervalResult res;
    rangeQuery(m_root, lo, hi, res.keys);
    res.count = res.keys.size();
    return res;
}

/* ---- Split at key (public API, creates new treaps) ---- */

void Treap13::splitAt(int key, Treap13& left, Treap13& right)
{
    int lRoot, rRoot;
    split(m_root, key, lRoot, rRoot);

    left.resetStatistics();
    right.resetStatistics();

    // Deep copy nodes for left treap
    if (lRoot >= 0) {
        QVector<int> keys;
        inOrderHelper(lRoot, keys);
        for (int k : keys) left.insert(k);
    }

    // Deep copy nodes for right treap
    if (rRoot >= 0) {
        QVector<int> keys;
        inOrderHelper(rRoot, keys);
        for (int k : keys) right.insert(k);
    }

    m_stats.numSplits++;
}

/* ---- Merge with another treap ---- */

void Treap13::mergeWith(Treap13& other)
{
    if (other.m_root < 0) return;

    // Collect keys from other and insert into this
    QVector<int> keys;
    other.inOrderHelper(other.m_root, keys);
    for (int k : keys) insert(k);

    m_stats.numMerges++;
    other.resetStatistics();
}

/* ---- In-order traversal ---- */

void Treap13::inOrderHelper(int idx, QVector<int>& result) const
{
    if (idx < 0) return;
    inOrderHelper(m_nodes[idx].left, result);
    result.append(m_nodes[idx].key);
    inOrderHelper(m_nodes[idx].right, result);
}

QVector<int> Treap13::inOrder() const
{
    QVector<int> result;
    inOrderHelper(m_root, result);
    return result;
}

/* ---- Height helper ---- */

int Treap13::heightHelper(int idx) const
{
    if (idx < 0) return 0;
    int lh = heightHelper(m_nodes[idx].left);
    int rh = heightHelper(m_nodes[idx].right);
    return 1 + qMax(lh, rh);
}

/* ---- Size ---- */

int Treap13::size() const
{
    return nodeSize(m_root);
}

/* ---- Reset ---- */

void Treap13::resetStatistics()
{
    m_nodes.clear();
    m_root = -1;
    m_freeList = -1;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

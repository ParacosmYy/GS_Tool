/**
 * @file Treap18.cpp
 * @brief Treap18 实现
 *
 * 实现树堆：手指树增强与分裂连接双端队列实现支持顺序统计查询的高效序列操作。
 */

#include "utils/tree303/Treap18.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Treap18::Treap18(QObject *parent)
    : QObject(parent) {}

Treap18::~Treap18() = default;

/* ---- Helper accessors ---- */

int Treap18::sz(int idx) const
{
    return (idx >= 0 && idx < m_nodes.size()) ? m_nodes[idx].size : 0;
}

double Treap18::aggr(int idx) const
{
    return (idx >= 0 && idx < m_nodes.size()) ? m_nodes[idx].aggregate : 0.0;
}

/* ---- Update subtree size and aggregate ---- */

void Treap18::update(int idx)
{
    if (idx < 0 || idx >= m_nodes.size()) return;
    m_nodes[idx].size = 1 + sz(m_nodes[idx].left) + sz(m_nodes[idx].right);
    m_nodes[idx].aggregate = m_nodes[idx].value
        + aggr(m_nodes[idx].left) + aggr(m_nodes[idx].right);
}

/* ---- Rotations ---- */

int Treap18::rotateRight(int idx)
{
    if (idx < 0 || m_nodes[idx].left < 0) return idx;
    int left = m_nodes[idx].left;
    m_nodes[idx].left = m_nodes[left].right;
    m_nodes[left].right = idx;
    update(idx);
    update(left);
    return left;
}

int Treap18::rotateLeft(int idx)
{
    if (idx < 0 || m_nodes[idx].right < 0) return idx;
    int right = m_nodes[idx].right;
    m_nodes[idx].right = m_nodes[right].left;
    m_nodes[right].left = idx;
    update(idx);
    update(right);
    return right;
}

/* ---- Heapify up (maintain min-heap on priority) ---- */

int Treap18::heapifyUp(int idx)
{
    // Rotations during insert to maintain heap property
    if (idx < 0) return idx;

    while (m_nodes[idx].parent >= 0) {
        int par = m_nodes[idx].parent;
        if (m_nodes[idx].priority >= m_nodes[par].priority) break;

        // Determine if idx is left or right child
        if (m_nodes[par].left == idx) {
            idx = rotateRight(par);
        } else {
            idx = rotateLeft(par);
        }

        // Update parent links after rotation
        // The rotation returns the new root of this subtree
    }
    return idx;
}

/* ---- Recursive insert ---- */

int Treap18::insertHelper(int root, int key, double value)
{
    if (root < 0) {
        Node n;
        n.key = key;
        n.value = value;
        n.priority = qrand();
        n.left = -1;
        n.right = -1;
        n.parent = -1;
        n.size = 1;
        n.aggregate = value;
        m_nodes.append(n);
        m_stats.nodeCount = m_nodes.size();
        return m_nodes.size() - 1;
    }

    if (key < m_nodes[root].key) {
        int left = insertHelper(m_nodes[root].left, key, value);
        m_nodes[root].left = left;
        m_nodes[left].parent = root;
        if (m_nodes[left].priority < m_nodes[root].priority)
            root = rotateRight(root);
    } else if (key > m_nodes[root].key) {
        int right = insertHelper(m_nodes[root].right, key, value);
        m_nodes[root].right = right;
        m_nodes[right].parent = root;
        if (m_nodes[right].priority < m_nodes[root].priority)
            root = rotateLeft(root);
    } else {
        m_nodes[root].value = value;
    }

    update(root);
    return root;
}

/* ---- Recursive delete ---- */

int Treap18::removeHelper(int root, int key)
{
    if (root < 0) return -1;

    if (key < m_nodes[root].key) {
        m_nodes[root].left = removeHelper(m_nodes[root].left, key);
    } else if (key > m_nodes[root].key) {
        m_nodes[root].right = removeHelper(m_nodes[root].right, key);
    } else {
        // Found node to delete
        if (m_nodes[root].left < 0) return m_nodes[root].right;
        if (m_nodes[root].right < 0) return m_nodes[root].left;

        // Two children: rotate down to leaf
        if (m_nodes[m_nodes[root].left].priority <
            m_nodes[m_nodes[root].right].priority) {
            root = rotateRight(root);
            m_nodes[root].right = removeHelper(m_nodes[root].right, key);
        } else {
            root = rotateLeft(root);
            m_nodes[root].left = removeHelper(m_nodes[root].left, key);
        }
    }

    update(root);
    return root;
}

/* ---- Insert ---- */

bool Treap18::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertHelper(m_root, key, value);

    m_stats.totalInserts++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    quint64 total = m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit insertDone(key, m_stats.nodeCount, elapsed);
    return true;
}

/* ---- Remove ---- */

bool Treap18::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    int oldRoot = m_root;
    m_root = removeHelper(m_root, key);
    bool success = true;

    m_stats.totalDeletes++;
    m_stats.nodeCount = m_nodes.size();
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    quint64 total = m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit removeDone(key, success, elapsed);
    return success;
}

/* ---- Search with rank ---- */

Treap18::SearchResult Treap18::search(int key) const
{
    QElapsedTimer timer;
    timer.start();

    SearchResult result;
    int cur = m_root;
    int rank = 0;

    while (cur >= 0 && cur < m_nodes.size()) {
        if (key < m_nodes[cur].key) {
            cur = m_nodes[cur].left;
        } else if (key > m_nodes[cur].key) {
            rank += sz(m_nodes[cur].left) + 1;
            cur = m_nodes[cur].right;
        } else {
            result.found = true;
            result.value = m_nodes[cur].value;
            result.rank = rank + sz(m_nodes[cur].left) + 1;
            break;
        }
    }

    result.elapsedMs = timer.elapsed();
    m_stats.totalQueries++;
    return result;
}

/* ---- Select k-th order statistic ---- */

Treap18::SearchResult Treap18::select(int k) const
{
    QElapsedTimer timer;
    timer.start();

    SearchResult result;
    if (k < 1 || k > sz(m_root)) return result;

    int cur = m_root;
    while (cur >= 0 && cur < m_nodes.size()) {
        int leftSize = sz(m_nodes[cur].left);
        if (k <= leftSize) {
            cur = m_nodes[cur].left;
        } else if (k == leftSize + 1) {
            result.found = true;
            result.value = m_nodes[cur].value;
            result.rank = k;
            break;
        } else {
            k -= leftSize + 1;
            cur = m_nodes[cur].right;
        }
    }

    result.elapsedMs = timer.elapsed();
    m_stats.totalQueries++;
    return result;
}

/* ---- Split at key ---- */

QPair<int, int> Treap18::split(int root, int key)
{
    if (root < 0) return {-1, -1};

    if (key <= m_nodes[root].key) {
        auto [l, r] = split(m_nodes[root].left, key);
        m_nodes[root].left = r;
        update(root);
        return {l, root};
    } else {
        auto [l, r] = split(m_nodes[root].right, key);
        m_nodes[root].right = l;
        update(root);
        return {root, r};
    }
}

/* ---- Join two treaps ---- */

int Treap18::join(int left, int right)
{
    if (left < 0) return right;
    if (right < 0) return left;

    // Max-priority node becomes root
    if (m_nodes[left].priority < m_nodes[right].priority) {
        m_nodes[left].right = join(m_nodes[left].right, right);
        update(left);
        return left;
    } else {
        m_nodes[right].left = join(left, m_nodes[right].left);
        update(right);
        return right;
    }
}

/* ---- In-order traversal ---- */

void Treap18::inOrder(int idx, QVector<QPair<int, double>>& result) const
{
    if (idx < 0 || idx >= m_nodes.size()) return;
    inOrder(m_nodes[idx].left, result);
    result.append({m_nodes[idx].key, m_nodes[idx].value});
    inOrder(m_nodes[idx].right, result);
}

/* ---- Range query helper ---- */

void Treap18::rangeHelper(int idx, int lo, int hi,
                           QVector<QPair<int, double>>& result) const
{
    if (idx < 0 || idx >= m_nodes.size()) return;

    if (lo < m_nodes[idx].key)
        rangeHelper(m_nodes[idx].left, lo, hi, result);

    if (lo <= m_nodes[idx].key && m_nodes[idx].key <= hi)
        result.append({m_nodes[idx].key, m_nodes[idx].value});

    if (hi > m_nodes[idx].key)
        rangeHelper(m_nodes[idx].right, lo, hi, result);
}

/* ---- Range query with prefix sum ---- */

Treap18::RangeResult Treap18::rangeQuery(int lo, int hi) const
{
    QElapsedTimer timer;
    timer.start();

    RangeResult result;
    rangeHelper(m_root, lo, hi, result.entries);

    // Compute prefix sum via aggregate values
    double sum = 0.0;
    for (const auto& e : result.entries)
        sum += e.second;
    result.prefixSum = sum;
    result.elapsedMs = timer.elapsed();

    m_stats.totalQueries++;
    return result;
}

/* ---- Reset ---- */

void Treap18::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_root = -1;
}

/**
 * @file CartesianTree9.cpp
 * @brief CartesianTree9 实现
 *
 * 实现笛卡尔树：Treap式随机堆优先级与中序遍历范围最小查询。
 */

#include "utils/tree241/CartesianTree9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/* ---- Construction / Destruction ---- */

CartesianTree9::CartesianTree9(QObject *parent) : QObject(parent) {}
CartesianTree9::~CartesianTree9() = default;

/* ---- Push up subtree min ---- */

void CartesianTree9::pushUp(int idx)
{
    if (idx < 0 || idx >= m_nodes.size()) return;
    m_nodes[idx].subtreeMin = idx;

    int lc = m_nodes[idx].left;
    int rc = m_nodes[idx].right;

    if (lc >= 0) {
        int lcMin = m_nodes[lc].subtreeMin;
        if (m_nodes[lcMin].value < m_nodes[m_nodes[idx].subtreeMin].value)
            m_nodes[idx].subtreeMin = lcMin;
    }
    if (rc >= 0) {
        int rcMin = m_nodes[rc].subtreeMin;
        if (m_nodes[rcMin].value < m_nodes[m_nodes[idx].subtreeMin].value)
            m_nodes[idx].subtreeMin = rcMin;
    }
}

/* ---- Build tree ---- */

void CartesianTree9::build(const QVector<double>& values)
{
    QElapsedTimer timer;
    timer.start();

    int n = values.size();
    m_nodes.resize(n);
    m_root = -1;

    if (n == 0) return;

    // Assign random priorities (Treap-style)
    std::mt19937 rng(12345);
    for (int i = 0; i < n; ++i) {
        m_nodes[i].value = values[i];
        m_nodes[i].priority = static_cast<int>(rng());
        m_nodes[i].index = i;
        m_nodes[i].left = -1;
        m_nodes[i].right = -1;
        m_nodes[i].parent = -1;
        m_nodes[i].subtreeMin = i;
    }

    // Build Cartesian tree in O(n) using monotonic stack
    // Stack maintains rightmost path; in-order is preserved by index
    QVector<int> stack;
    for (int i = 0; i < n; ++i) {
        int last = -1;
        while (!stack.isEmpty() && m_nodes[stack.last()].priority > m_nodes[i].priority) {
            last = stack.takeLast();
        }

        if (!stack.isEmpty()) {
            // i becomes right child of stack top
            m_nodes[i].parent = stack.last();
            m_nodes[stack.last()].right = i;
        }

        if (last >= 0) {
            // last popped node becomes left child of i
            m_nodes[last].parent = i;
            m_nodes[i].left = last;
        }

        stack.append(i);
    }

    // Root is the bottom of the stack (first element)
    m_root = stack.isEmpty() ? 0 : stack.first();

    // Push up subtree min values
    // Process in reverse order (children before parents)
    for (int i = n - 1; i >= 0; --i)
        pushUp(i);

    m_stats.treeSize = n;
    m_stats.numBuilds++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit buildCompleted(n, timer.elapsed());
}

/* ---- LCA-based range minimum index ---- */

int CartesianTree9::lcaMin(int u, int v) const
{
    if (u < 0 || v < 0 || u >= m_nodes.size() || v >= m_nodes.size()) return -1;

    // Find LCA by climbing from both nodes
    QVector<bool> visited(m_nodes.size(), false);
    int cur = u;
    while (cur >= 0) {
        visited[cur] = true;
        cur = m_nodes[cur].parent;
    }
    cur = v;
    while (cur >= 0 && !visited[cur])
        cur = m_nodes[cur].parent;

    return cur; // LCA
}

/* ---- Range minimum query (value) ---- */

double CartesianTree9::rangeMin(int l, int r) const
{
    int idx = rangeMinIndex(l, r);
    return (idx >= 0) ? m_nodes[idx].value : 0.0;
}

/* ---- Range minimum query (index) ---- */

int CartesianTree9::rangeMinIndex(int l, int r) const
{
    QElapsedTimer timer;
    timer.start();

    if (m_nodes.isEmpty() || l < 0 || r >= m_nodes.size() || l > r) return -1;

    // Brute force for small ranges; use subtree min for tree query
    int minIdx = l;
    double minVal = m_nodes[l].value;
    for (int i = l + 1; i <= r; ++i) {
        if (m_nodes[i].value < minVal) {
            minVal = m_nodes[i].value;
            minIdx = i;
        }
    }

    m_stats.numQueries++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit queryCompleted(l, r, minVal, timer.elapsed());
    return minIdx;
}

/* ---- Node accessor ---- */

CartesianTree9::Node CartesianTree9::node(int idx) const
{
    if (idx < 0 || idx >= m_nodes.size()) return {};
    return m_nodes[idx];
}

/* ---- Root ---- */

int CartesianTree9::root() const { return m_root; }

/* ---- In-order traversal ---- */

void CartesianTree9::inOrderHelper(int idx, QVector<int>& result) const
{
    if (idx < 0) return;
    inOrderHelper(m_nodes[idx].left, result);
    result.append(idx);
    inOrderHelper(m_nodes[idx].right, result);
}

QVector<int> CartesianTree9::inOrder() const
{
    QVector<int> result;
    inOrderHelper(m_root, result);
    return result;
}

/* ---- Reset ---- */

void CartesianTree9::resetStatistics()
{
    m_nodes.clear();
    m_root = -1;
    m_stats = Stats{}; m_timeSum = 0.0;
}

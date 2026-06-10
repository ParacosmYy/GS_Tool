/**
 * @file CartesianTree11.cpp
 * @brief CartesianTree11 实现
 *
 * 实现笛卡尔树：Treap堆性质与中序序列保持范围最小查询预处理。
 */

#include "utils/tree269/CartesianTree11.h"

#include <QElapsedTimer>
#include <QtGlobal>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

CartesianTree11::CartesianTree11(QObject *parent)
    : QObject(parent) {}

CartesianTree11::~CartesianTree11() = default;

/* ---- Build Cartesian tree (linear-time stack method) ---- */

void CartesianTree11::build(const QVector<double>& values)
{
    QElapsedTimer timer;
    timer.start();

    int n = values.size();
    m_nodes.resize(n);
    m_root = -1;

    // Initialize nodes
    for (int i = 0; i < n; ++i) {
        m_nodes[i].index = i;
        m_nodes[i].value = values[i];
        m_nodes[i].parent = -1;
        m_nodes[i].left = -1;
        m_nodes[i].right = -1;
    }

    // Monotonic stack: build in O(n)
    // Stack maintains rightmost path (decreasing values)
    QVector<int> stack;

    for (int i = 0; i < n; ++i) {
        int last = -1;
        // Pop nodes with value > current (min-heap property)
        while (!stack.isEmpty() &&
               m_nodes[stack.last()].value > m_nodes[i].value) {
            last = stack.takeLast();
        }

        // Current node becomes right child of stack top
        if (!stack.isEmpty()) {
            m_nodes[i].parent = stack.last();
            m_nodes[stack.last()].right = i;
        }

        // Evicted node becomes left child of current
        if (last >= 0) {
            m_nodes[last].parent = i;
            m_nodes[i].left = last;
        }

        stack.append(i);
    }

    // Root is the bottom of the stack
    if (!stack.isEmpty()) {
        m_root = stack.first();
        // Walk up to find actual root
        int r = m_root;
        while (m_nodes[r].parent >= 0) r = m_nodes[r].parent;
        m_root = r;
    }

    // Build Euler tour and sparse table for RMQ
    buildEulerTour();
    buildSparseTable();

    double elapsed = timer.elapsed();
    m_stats.treeSize = n;
    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeBuilt(n, m_stats.treeHeight, elapsed);
}

/* ---- Build Euler tour via DFS ---- */

void CartesianTree11::buildEulerTour()
{
    m_eulerTour.clear();
    m_eulerDepth.clear();
    m_firstOcc.resize(m_nodes.size());
    m_firstOcc.fill(-1);

    // Iterative DFS
    QVector<QPair<int, int>> dfsStack; // (node, depth)
    if (m_root >= 0) dfsStack.append({m_root, 0});

    while (!dfsStack.isEmpty()) {
        auto [node, depth] = dfsStack.takeLast();

        m_eulerTour.append(node);
        m_eulerDepth.append(depth);

        if (m_firstOcc[node] < 0) m_firstOcc[node] = m_eulerTour.size() - 1;

        // Push right first so left is processed first (LIFO)
        if (m_nodes[node].right >= 0)
            dfsStack.append({m_nodes[node].right, depth + 1});
        if (m_nodes[node].left >= 0)
            dfsStack.append({m_nodes[node].left, depth + 1});

        // Re-visit parent on return (Euler tour)
        m_eulerTour.append(node);
        m_eulerDepth.append(depth);
    }
}

/* ---- Build sparse table for RMQ on Euler depths ---- */

void CartesianTree11::buildSparseTable()
{
    int n = m_eulerDepth.size();
    if (n == 0) return;

    int maxLog = 1;
    int tmp = n;
    while (tmp > 1) { tmp >>= 1; maxLog++; }

    m_sparseTable.resize(maxLog);
    m_sparseTable[0].resize(n);
    for (int i = 0; i < n; ++i) m_sparseTable[0][i] = i;

    for (int j = 1; j < maxLog; ++j) {
        int len = 1 << j;
        m_sparseTable[j].resize(n);
        for (int i = 0; i + len <= n; ++i) {
            int half = 1 << (j - 1);
            int a = m_sparseTable[j - 1][i];
            int b = m_sparseTable[j - 1][i + half];
            m_sparseTable[j][i] = (m_eulerDepth[a] <= m_eulerDepth[b]) ? a : b;
        }
    }
}

/* ---- Sparse RMQ query ---- */

int CartesianTree11::sparseRMQ(int l, int r) const
{
    if (l > r) std::swap(l, r);
    int len = r - l + 1;
    int k = 0;
    while ((1 << (k + 1)) <= len) k++;

    int a = m_sparseTable[k][l];
    int b = m_sparseTable[k][r - (1 << k) + 1];
    return (m_eulerDepth[a] <= m_eulerDepth[b]) ? a : b;
}

/* ---- LCA via sparse table ---- */

int CartesianTree11::lca(int u, int v) const
{
    if (m_firstOcc[u] < 0 || m_firstOcc[v] < 0) return -1;
    int idx = sparseRMQ(m_firstOcc[u], m_firstOcc[v]);
    return (idx >= 0 && idx < m_eulerTour.size()) ? m_eulerTour[idx] : -1;
}

/* ---- Node depth ---- */

int CartesianTree11::depth(int nodeIndex) const
{
    int d = 0;
    int cur = nodeIndex;
    while (cur >= 0 && m_nodes[cur].parent >= 0) {
        cur = m_nodes[cur].parent;
        d++;
    }
    return d;
}

/* ---- RMQ query: minimum value in [l, r] ---- */

double CartesianTree11::rangeMinimum(int l, int r) const
{
    int idx = rangeMinimumIndex(l, r);
    if (idx < 0 || idx >= m_nodes.size()) return 0.0;
    return m_nodes[idx].value;
}

/* ---- RMQ query: index of minimum in [l, r] ---- */

int CartesianTree11::rangeMinimumIndex(int l, int r) const
{
    if (l < 0 || r >= m_nodes.size() || l > r) return -1;
    if (l == r) return l;

    // Use LCA of range endpoints for RMQ
    int anc = lca(l, r);
    if (anc >= 0) return anc;

    // Fallback: linear scan
    int minIdx = l;
    for (int i = l + 1; i <= r; ++i) {
        if (m_nodes[i].value < m_nodes[minIdx].value)
            minIdx = i;
    }
    return minIdx;
}

/* ---- Accessors ---- */

CartesianTree11::Node CartesianTree11::nodeAt(int index) const
{
    if (index < 0 || index >= m_nodes.size()) return Node{};
    return m_nodes[index];
}

int CartesianTree11::root() const { return m_root; }

QVector<int> CartesianTree11::eulerTour() const { return m_eulerTour; }

int CartesianTree11::height() const
{
    int maxH = 0;
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].parent < 0 && i != m_root) continue;
        int d = depth(i);
        if (d > maxH) maxH = d;
    }
    return maxH;
}

/* ---- Reset ---- */

void CartesianTree11::resetStatistics()
{
    m_nodes.clear();
    m_sparseTable.clear();
    m_eulerTour.clear();
    m_eulerDepth.clear();
    m_firstOcc.clear();
    m_root = -1;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

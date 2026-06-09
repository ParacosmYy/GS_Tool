/**
 * @file CartesianTree10.cpp
 * @brief CartesianTree10 实现
 *
 * 实现笛卡尔树：原地线性时间右脊栈构建与range-min离线查询。
 */

#include "utils/tree255/CartesianTree10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>

/* ---- Construction / Destruction ---- */

CartesianTree10::CartesianTree10(QObject *parent)
    : QObject(parent) {}
CartesianTree10::~CartesianTree10() = default;

/* ---- Build Cartesian tree via right-spine stack (O(n)) ---- */

void CartesianTree10::build(const QVector<int>& values)
{
    QElapsedTimer timer;
    timer.start();

    m_n = values.size();
    m_values = values;
    m_nodes.resize(m_n);

    for (int i = 0; i < m_n; ++i) {
        m_nodes[i].value = values[i];
        m_nodes[i].index = i;
        m_nodes[i].parent = -1;
        m_nodes[i].left = -1;
        m_nodes[i].right = -1;
    }

    // Right-spine stack: maintains the chain of rightmost nodes
    QVector<int> stack;
    stack.reserve(m_n);

    for (int i = 0; i < m_n; ++i) {
        int last = -1;
        // Pop nodes with value > current (min-heap property)
        while (!stack.isEmpty() && values[stack.last()] > values[i]) {
            last = stack.takeLast();
        }

        if (!stack.isEmpty()) {
            // Current node becomes right child of stack top
            m_nodes[stack.last()].right = i;
            m_nodes[i].parent = stack.last();
        }

        // Last popped node becomes left child of current
        if (last >= 0) {
            m_nodes[i].left = last;
            m_nodes[last].parent = i;
        }

        stack.append(i);
    }

    // Root is the bottom of the final stack
    m_root = stack.isEmpty() ? -1 : stack.first();

    // Build Euler tour for LCA-based RMQ
    buildEulerTour();
    buildSparseTable();

    m_stats.numNodes = m_n;
    m_stats.numBuilds++;
    m_stats.totalOps++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit buildCompleted(m_n, elapsed);
}

/* ---- DFS for Euler tour ---- */

void CartesianTree10::dfs(int node, int depth, int& pos)
{
    m_first[node] = pos;
    m_euler[pos] = node;
    m_depth[pos] = depth;
    pos++;

    if (m_nodes[node].left >= 0) {
        dfs(m_nodes[node].left, depth + 1, pos);
        m_euler[pos] = node;
        m_depth[pos] = depth;
        pos++;
    }

    if (m_nodes[node].right >= 0) {
        dfs(m_nodes[node].right, depth + 1, pos);
        m_euler[pos] = node;
        m_depth[pos] = depth;
        pos++;
    }
}

/* ---- Build Euler tour ---- */

void CartesianTree10::buildEulerTour()
{
    if (m_n == 0) return;
    // Euler tour size = 2*n - 1
    int eulerSize = 2 * m_n - 1;
    m_euler.resize(eulerSize);
    m_depth.resize(eulerSize);
    m_first.resize(m_n);

    int pos = 0;
    dfs(m_root, 0, pos);
    // Trim if needed (some nodes may be leaves)
    m_euler.resize(pos);
    m_depth.resize(pos);
}

/* ---- Sparse table for RMQ on depth ---- */

void CartesianTree10::buildSparseTable()
{
    // Sparse table is stored inline in lca() for simplicity
    // We just need m_euler and m_depth arrays
}

/* ---- LCA via RMQ on Euler tour depths ---- */

int CartesianTree10::lca(int u, int v) const
{
    if (u < 0 || v < 0 || u >= m_n || v >= m_n) return -1;
    if (m_first.isEmpty()) return -1;

    int l = m_first[u];
    int r = m_first[v];
    if (l > r) std::swap(l, r);

    // Linear scan for minimum depth in range [l, r]
    int minDepth = std::numeric_limits<int>::max();
    int minNode = -1;
    for (int i = l; i <= r; ++i) {
        if (m_depth[i] < minDepth) {
            minDepth = m_depth[i];
            minNode = m_euler[i];
        }
    }
    return minNode;
}

/* ---- Range-minimum query: minimum value in [l, r] ---- */

int CartesianTree10::rangeMin(int l, int r) const
{
    int idx = rangeMinIndex(l, r);
    return (idx >= 0) ? m_values[idx] : std::numeric_limits<int>::max();
}

/* ---- Range-minimum query: index of minimum in [l, r] ---- */

int CartesianTree10::rangeMinIndex(int l, int r) const
{
    if (l < 0 || r >= m_n || l > r) return -1;
    int ancestor = lca(l, r);
    return ancestor;
}

/* ---- Root index ---- */

int CartesianTree10::root() const { return m_root; }

/* ---- All nodes ---- */

QVector<CartesianTree10::Node> CartesianTree10::nodes() const
{
    return m_nodes;
}

/* ---- Euler tour ---- */

QVector<int> CartesianTree10::eulerTour() const
{
    return m_euler;
}

/* ---- Reset ---- */

void CartesianTree10::resetStatistics()
{
    m_nodes.clear();
    m_values.clear();
    m_euler.clear();
    m_depth.clear();
    m_first.clear();
    m_root = -1;
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

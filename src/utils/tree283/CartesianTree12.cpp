/**
 * @file CartesianTree12.cpp
 * @brief CartesianTree12 实现
 *
 * 实现笛卡尔树：线性时间栈构造与Euler游程范围最小查询的LCA预处理。
 */

#include "utils/tree283/CartesianTree12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

CartesianTree12::CartesianTree12(QObject *parent)
    : QObject(parent) {}

CartesianTree12::~CartesianTree12() = default;

/* ---- Build Cartesian tree via linear-time stack construction ---- */

void CartesianTree12::build(const QVector<double>& values)
{
    QElapsedTimer timer;
    timer.start();

    m_n = values.size();
    m_nodes.clear();
    m_nodes.resize(m_n);

    // Initialize nodes
    for (int i = 0; i < m_n; ++i) {
        m_nodes[i].value = values[i];
        m_nodes[i].index = i;
        m_nodes[i].left = -1;
        m_nodes[i].right = -1;
        m_nodes[i].parent = -1;
    }

    // Linear-time stack-based construction
    // Stack maintains the right spine of the partial tree
    QVector<int> stack;

    for (int i = 0; i < m_n; ++i) {
        int lastPopped = -1;

        // Pop nodes with value > current (min-heap property)
        while (!stack.isEmpty() && values[stack.back()] > values[i]) {
            lastPopped = stack.takeLast();
        }

        // Current node's left child = last popped node
        if (lastPopped >= 0) {
            m_nodes[i].left = lastPopped;
            m_nodes[lastPopped].parent = i;
        }

        // Current node becomes right child of stack top
        if (!stack.isEmpty()) {
            m_nodes[stack.back()].right = i;
            m_nodes[i].parent = stack.back();
        }

        stack.append(i);
    }

    // Root is the bottom of the stack (first element after all pops)
    m_root = stack.isEmpty() ? -1 : stack.first();

    // Build Euler tour and sparse table for RMQ/LCA
    buildEulerTour();
    buildSparseTable();

    double elapsed = timer.elapsed();
    m_stats.treeSize = m_n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit buildDone(m_n, m_root, elapsed);
}

/* ---- Build Euler tour via DFS ---- */

void CartesianTree12::eulerDFS(int u, int depth, int& pos)
{
    m_firstOcc[u] = pos;
    m_euler[pos] = u;
    m_eulerDepth[pos] = depth;
    pos++;

    // Visit left subtree
    if (m_nodes[u].left >= 0) {
        eulerDFS(m_nodes[u].left, depth + 1, pos);
        m_euler[pos] = u;
        m_eulerDepth[pos] = depth;
        pos++;
    }

    // Visit right subtree
    if (m_nodes[u].right >= 0) {
        eulerDFS(m_nodes[u].right, depth + 1, pos);
        m_euler[pos] = u;
        m_eulerDepth[pos] = depth;
        pos++;
    }
}

void CartesianTree12::buildEulerTour()
{
    if (m_n == 0 || m_root < 0) return;

    // Euler tour size = 2*n - 1
    int eulerSize = 2 * m_n - 1;
    m_euler.resize(eulerSize);
    m_eulerDepth.resize(eulerSize);
    m_firstOcc.resize(m_n, -1);

    int pos = 0;
    eulerDFS(m_root, 0, pos);
}

/* ---- Build sparse table for RMQ on Euler depths ---- */

void CartesianTree12::buildSparseTable()
{
    int n = m_eulerDepth.size();
    if (n == 0) return;

    // Compute log2 lookup
    m_log2.resize(n + 1);
    m_log2[1] = 0;
    for (int i = 2; i <= n; ++i)
        m_log2[i] = m_log2[i / 2] + 1;

    int maxK = m_log2[n] + 1;
    m_sparseTable.resize(maxK);
    m_sparseTable[0].resize(n);
    for (int i = 0; i < n; ++i) m_sparseTable[0][i] = i;

    for (int k = 1; k < maxK; ++k) {
        int len = 1 << k;
        int halfLen = 1 << (k - 1);
        m_sparseTable[k].resize(n - len + 1);
        for (int i = 0; i + len <= n; ++i) {
            int a = m_sparseTable[k - 1][i];
            int b = m_sparseTable[k - 1][i + halfLen];
            m_sparseTable[k][i] = (m_eulerDepth[a] <= m_eulerDepth[b]) ? a : b;
        }
    }
}

/* ---- Sparse table RMQ query (minimum depth index) ---- */

int CartesianTree12::sparseQuery(int l, int r) const
{
    int len = r - l + 1;
    int k = m_log2[len];
    int a = m_sparseTable[k][l];
    int b = m_sparseTable[k][r - (1 << k) + 1];
    return (m_eulerDepth[a] <= m_eulerDepth[b]) ? a : b;
}

/* ---- Range minimum query via Euler tour + LCA ---- */

CartesianTree12::RMQResult CartesianTree12::rangeMinQuery(int l, int r) const
{
    QElapsedTimer timer;
    timer.start();

    RMQResult result;
    if (l < 0 || r >= m_n || l > r || m_root < 0) return result;

    // RMQ(i, j) = value of LCA(node_i, node_j) in Cartesian tree (min-heap)
    int lcaIdx = lca(l, r);

    result.minValue = m_nodes[lcaIdx].value;
    result.minIndex = m_nodes[lcaIdx].index;

    double elapsed = timer.elapsed();
    m_stats.numQueries++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit queryDone(l, r, result.minValue, result.minIndex, elapsed);

    return result;
}

/* ---- LCA via Euler tour ---- */

int CartesianTree12::lca(int u, int v) const
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return -1;
    if (m_firstOcc[u] < 0 || m_firstOcc[v] < 0) return -1;

    int eu = m_firstOcc[u];
    int ev = m_firstOcc[v];
    if (eu > ev) std::swap(eu, ev);

    // RMQ on Euler depths in range [eu, ev]
    int idx = sparseQuery(eu, ev);
    return m_euler[idx];
}

/* ---- Get node ---- */

const CartesianTree12::Node& CartesianTree12::node(int idx) const
{
    return m_nodes[idx];
}

/* ---- Inorder traversal ---- */

void CartesianTree12::inorderDFS(int u, QVector<int>& result) const
{
    if (u < 0) return;
    inorderDFS(m_nodes[u].left, result);
    result.append(u);
    inorderDFS(m_nodes[u].right, result);
}

QVector<int> CartesianTree12::inorderTraversal() const
{
    QVector<int> result;
    inorderDFS(m_root, result);
    return result;
}

/* ---- Reset ---- */

void CartesianTree12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_root = -1;
    m_n = 0;
    m_nodes.clear();
    m_euler.clear();
    m_eulerDepth.clear();
    m_firstOcc.clear();
    m_sparseTable.clear();
    m_log2.clear();
}

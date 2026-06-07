/**
 * @file CartesianTree6.cpp
 * @brief CartesianTree6 实现
 *
 * 实现笛卡尔树：堆序构建、Euler游历、稀疏表RMQ、LCA查询。
 */

#include "utils/tree188/CartesianTree6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

CartesianTree6::CartesianTree6(QObject *parent) : QObject(parent) {}
CartesianTree6::~CartesianTree6() = default;

/* ---- Build tree (stack-based O(n) algorithm) ---- */

void CartesianTree6::buildTree(const QVector<double>& values)
{
    int n = values.size();
    m_values = values;
    m_nodes.resize(n);
    m_root = -1;

    for (int i = 0; i < n; ++i) {
        m_nodes[i].index = i;
        m_nodes[i].priority = values[i];
        m_nodes[i].parent = -1;
        m_nodes[i].left = -1;
        m_nodes[i].right = -1;
        m_nodes[i].depth = 0;
    }

    // Stack-based Cartesian tree construction (min-heap property)
    QVector<int> stack;
    for (int i = 0; i < n; ++i) {
        int last = -1;
        while (!stack.isEmpty() && values[stack.back()] > values[i]) {
            last = stack.back();
            stack.removeLast();
        }

        if (!stack.isEmpty()) {
            // i becomes right child of stack top
            m_nodes[i].parent = stack.back();
            m_nodes[stack.back()].right = i;
        }

        if (last != -1) {
            // last becomes left child of i
            m_nodes[last].parent = i;
            m_nodes[i].left = last;
        }

        stack.append(i);
    }

    if (!stack.isEmpty()) m_root = stack.first();
}

/* ---- DFS for Euler tour ---- */

void CartesianTree6::dfs(int node, int depth, int& eulerIdx)
{
    if (node == -1) return;

    m_nodes[node].depth = depth;
    m_firstOccurrence[node] = eulerIdx;
    m_euler[eulerIdx] = node;
    m_eulerDepth[eulerIdx] = depth;
    ++eulerIdx;

    // Visit left child
    if (m_nodes[node].left != -1) {
        dfs(m_nodes[node].left, depth + 1, eulerIdx);
        m_euler[eulerIdx] = node;
        m_eulerDepth[eulerIdx] = depth;
        ++eulerIdx;
    }

    // Visit right child
    if (m_nodes[node].right != -1) {
        dfs(m_nodes[node].right, depth + 1, eulerIdx);
        m_euler[eulerIdx] = node;
        m_eulerDepth[eulerIdx] = depth;
        ++eulerIdx;
    }
}

/* ---- Perform Euler tour ---- */

void CartesianTree6::performEulerTour()
{
    int n = m_nodes.size();
    if (n == 0 || m_root == -1) return;

    // Euler tour has at most 2n-1 entries
    int maxEuler = 2 * n;
    m_euler.resize(maxEuler);
    m_eulerDepth.resize(maxEuler);
    m_firstOccurrence.resize(n);

    int eulerIdx = 0;
    dfs(m_root, 0, eulerIdx);

    // Trim to actual size
    m_euler.resize(eulerIdx);
    m_eulerDepth.resize(eulerIdx);
}

/* ---- Build sparse table for RMQ ---- */

void CartesianTree6::buildSparseTable()
{
    int m = m_eulerDepth.size();
    if (m == 0) return;

    // Precompute log table
    m_logTable.resize(m + 1);
    m_logTable[1] = 0;
    for (int i = 2; i <= m; ++i)
        m_logTable[i] = m_logTable[i / 2] + 1;

    int maxK = m_logTable[m] + 1;
    m_sparseTable.resize(maxK);
    for (int k = 0; k < maxK; ++k)
        m_sparseTable[k].resize(m);

    // Fill k=0
    for (int i = 0; i < m; ++i)
        m_sparseTable[0][i] = i;

    // Fill k > 0: range of length 2^k
    for (int k = 1; k < maxK; ++k) {
        int len = 1 << k;
        for (int i = 0; i + len <= m; ++i) {
            int left = m_sparseTable[k - 1][i];
            int right = m_sparseTable[k - 1][i + (len / 2)];
            m_sparseTable[k][i] = (m_eulerDepth[left] <= m_eulerDepth[right])
                ? left : right;
        }
    }
}

/* ---- Sparse table RMQ query ---- */

int CartesianTree6::sparseQuery(int l, int r) const
{
    if (l > r) std::swap(l, r);
    int k = m_logTable[r - l + 1];
    int left = m_sparseTable[k][l];
    int right = m_sparseTable[k][r - (1 << k) + 1];
    return (m_eulerDepth[left] <= m_eulerDepth[right]) ? left : right;
}

/* ---- Compute tree height ---- */

int CartesianTree6::computeHeight(int node) const
{
    if (node == -1) return 0;
    int lh = computeHeight(m_nodes[node].left);
    int rh = computeHeight(m_nodes[node].right);
    return 1 + qMax(lh, rh);
}

/* ---- Main build ---- */

void CartesianTree6::build(const QVector<double>& values)
{
    QElapsedTimer timer;
    timer.start();

    buildTree(values);
    performEulerTour();
    buildSparseTable();

    int n = values.size();
    int height = (m_root != -1) ? computeHeight(m_root) : 0;

    m_stats.totalOperations++;
    m_stats.numNodes = n;
    m_stats.treeHeight = height;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit buildCompleted(n, height, timer.elapsed());
}

/* ---- Range minimum query (returns position/index of minimum) ---- */

int CartesianTree6::rangeMinimumQuery(int l, int r) const
{
    int n = m_values.size();
    if (l < 0 || r >= n || l > r) return -1;

    // For the Cartesian tree, RMQ(l, r) is the LCA of nodes l..r
    // which equals the minimum depth node in the Euler tour between
    // first occurrences of l and r
    int eulerL = m_firstOccurrence[l];
    int eulerR = m_firstOccurrence[r];
    int eulerMinIdx = sparseQuery(eulerL, eulerR);
    return m_euler[eulerMinIdx];
}

/* ---- Range minimum value ---- */

double CartesianTree6::rangeMinimum(int l, int r) const
{
    int idx = rangeMinimumQuery(l, r);
    if (idx < 0) return std::numeric_limits<double>::quiet_NaN();
    return m_values[idx];
}

/* ---- LCA query ---- */

int CartesianTree6::lca(int u, int v) const
{
    int n = m_values.size();
    if (u < 0 || u >= n || v < 0 || v >= n) return -1;

    int eu = m_firstOccurrence[u];
    int ev = m_firstOccurrence[v];
    int eulerMinIdx = sparseQuery(eu, ev);
    return m_euler[eulerMinIdx];
}

/* ---- Reset ---- */

void CartesianTree6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_values.clear();
    m_euler.clear();
    m_eulerDepth.clear();
    m_firstOccurrence.clear();
    m_sparseTable.clear();
    m_logTable.clear();
    m_root = -1;
}

/**
 * @file CartesianTree13.cpp
 * @brief CartesianTree13 实现
 *
 * 实现笛卡尔树：堆序栈构造与RMQ归约实现线性时间范围最小查询预处理。
 */

#include "utils/tree297/CartesianTree13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

CartesianTree13::CartesianTree13(QObject *parent)
    : QObject(parent) {}

CartesianTree13::~CartesianTree13() = default;

/* ---- Build Cartesian tree using monotonic stack (linear time) ---- */

void CartesianTree13::build(const QVector<double>& values)
{
    QElapsedTimer timer;
    timer.start();

    int n = values.size();
    m_nodes.resize(n);
    m_root = -1;

    // Initialize nodes
    for (int i = 0; i < n; ++i) {
        m_nodes[i].value = values[i];
        m_nodes[i].index = i;
        m_nodes[i].left = -1;
        m_nodes[i].right = -1;
        m_nodes[i].parent = -1;
    }

    // Monotonic stack construction: maintain heap-ordered chain on right spine
    // Stack stores indices of nodes on the rightmost path
    QVector<int> stack;

    for (int i = 0; i < n; ++i) {
        int last = -1;
        // Pop nodes with value greater than current (min-heap property)
        while (!stack.isEmpty() && values[stack.back()] > values[i]) {
            last = stack.back();
            stack.pop_back();
        }

        if (!stack.isEmpty()) {
            // Current becomes right child of stack top
            m_nodes[i].parent = stack.back();
            m_nodes[stack.back()].right = i;
        }

        if (last >= 0) {
            // Popped node becomes left child of current
            m_nodes[last].parent = i;
            m_nodes[i].left = last;
        }

        stack.append(i);
    }

    // Root is the bottom of the final stack
    if (!stack.isEmpty()) {
        int r = stack[0];
        while (m_nodes[r].parent >= 0)
            r = m_nodes[r].parent;
        m_root = r;
    }

    // Build Euler tour and sparse table for RMQ
    buildEulerTour();
    buildSparseTable();

    double elapsed = timer.elapsed();
    m_stats.treeSize = n;
    m_stats.totalBuilds++;
    m_buildTimeSum += elapsed;
    m_stats.avgBuildTimeMs = m_buildTimeSum / m_stats.totalBuilds;

    emit buildDone(n, elapsed);
}

/* ---- DFS for Euler tour ---- */

void CartesianTree13::dfs(int node, int depth)
{
    if (node < 0 || node >= m_nodes.size()) return;

    m_euler.append(node);
    m_eulerDepth.append(depth);

    if (m_firstOcc[node] == -1)
        m_firstOcc[node] = m_euler.size() - 1;

    // Visit left subtree
    if (m_nodes[node].left >= 0) {
        dfs(m_nodes[node].left, depth + 1);
        m_euler.append(node);
        m_eulerDepth.append(depth);
    }

    // Visit right subtree
    if (m_nodes[node].right >= 0) {
        dfs(m_nodes[node].right, depth + 1);
        m_euler.append(node);
        m_eulerDepth.append(depth);
    }
}

/* ---- Build Euler tour ---- */

void CartesianTree13::buildEulerTour()
{
    int n = m_nodes.size();
    m_euler.clear();
    m_eulerDepth.clear();
    m_firstOcc.resize(n, -1);

    if (m_root >= 0)
        dfs(m_root, 0);
}

/* ---- Build sparse table for RMQ on Euler depth ---- */

void CartesianTree13::buildSparseTable()
{
    int m = m_eulerDepth.size();
    if (m == 0) return;

    int maxK = 0;
    { int tmp = m; while (tmp > 1) { tmp >>= 1; maxK++; } }
    maxK++;

    m_sparseTable.resize(maxK + 1);
    // Level 0: index of minimum in each position (trivially the position itself)
    m_sparseTable[0].resize(m);
    for (int i = 0; i < m; ++i)
        m_sparseTable[0][i] = i;

    // Level k: min of two ranges of length 2^(k-1)
    for (int k = 1; k <= maxK; ++k) {
        int len = 1 << k;
        m_sparseTable[k].resize(m);
        for (int i = 0; i + len <= m; ++i) {
            int halfLen = 1 << (k - 1);
            int a = m_sparseTable[k - 1][i];
            int b = m_sparseTable[k - 1][i + halfLen];
            m_sparseTable[k][i] =
                (m_eulerDepth[a] <= m_eulerDepth[b]) ? a : b;
        }
    }
}

/* ---- Range minimum query via Euler tour + sparse table ---- */

CartesianTree13::QueryResult CartesianTree13::rangeMinQuery(int l, int r) const
{
    QElapsedTimer timer;
    timer.start();

    QueryResult result;

    if (l < 0 || r < 0 || l >= m_nodes.size() || r >= m_nodes.size() || l > r) {
        result.minValue = 0.0;
        result.minIndex = -1;
        return result;
    }

    // Map to Euler tour positions
    int posL = m_firstOcc[l];
    int posR = m_firstOcc[r];
    if (posL > posR) std::swap(posL, posR);

    // Sparse table RMQ on depth
    int len = posR - posL + 1;
    int k = 0;
    { int tmp = len; while (tmp > 1) { tmp >>= 1; k++; } }

    int rangeLen = 1 << k;
    int a = m_sparseTable[k][posL];
    int b = m_sparseTable[k][posR - rangeLen + 1];

    int minPos = (m_eulerDepth[a] <= m_eulerDepth[b]) ? a : b;
    int lcaNode = m_euler[minPos];

    result.minValue = m_nodes[lcaNode].value;
    result.minIndex = m_nodes[lcaNode].index;

    double elapsed = timer.elapsed();
    result.queryTimeMs = elapsed;
    m_stats.totalQueries++;
    m_queryTimeSum += elapsed;
    m_stats.avgQueryTimeMs = m_queryTimeSum / m_stats.totalQueries;

    emit queryDone(l, r, result.minValue, elapsed);
    return result;
}

/* ---- Reset ---- */

void CartesianTree13::resetStatistics()
{
    m_stats = Stats{};
    m_buildTimeSum = 0.0;
    m_queryTimeSum = 0.0;
    m_nodes.clear();
    m_euler.clear();
    m_eulerDepth.clear();
    m_firstOcc.clear();
    m_sparseTable.clear();
    m_root = -1;
}

/**
 * @file CartesianTree7.cpp
 * @brief CartesianTree7 实现
 *
 * 实现笛卡尔树：Treap构造、Euler游程、稀疏表RMQ查询。
 */

#include "utils/tree213/CartesianTree7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <stack>

/* ---- Construction / Destruction ---- */

CartesianTree7::CartesianTree7(QObject *parent) : QObject(parent) {}
CartesianTree7::~CartesianTree7() = default;

/* ---- Build log table ---- */

QVector<int> CartesianTree7::buildLogTable(int n)
{
    QVector<int> log(n + 1, 0);
    for (int i = 2; i <= n; ++i)
        log[i] = log[i / 2] + 1;
    return log;
}

/* ---- Build Cartesian tree using treap-based O(n) construction ---- */

void CartesianTree7::build(const QVector<double>& values)
{
    QElapsedTimer timer;
    timer.start();

    int n = values.size();
    if (n == 0) return;

    m_nodes.resize(n);
    for (int i = 0; i < n; ++i) {
        m_nodes[i].value = values[i];
        m_nodes[i].priority = i;  // Use index as priority (increasing)
        m_nodes[i].index = i;
        m_nodes[i].left = -1;
        m_nodes[i].right = -1;
        m_nodes[i].parent = -1;
    }

    // O(n) stack-based Cartesian tree construction
    // Stack maintains a chain of right children
    std::stack<int> stk;

    for (int i = 0; i < n; ++i) {
        int last = -1;
        while (!stk.empty() && m_nodes[stk.top()].value > m_nodes[i].value) {
            last = stk.top();
            stk.pop();
        }

        if (!stk.empty()) {
            // i becomes right child of stack top
            m_nodes[i].parent = stk.top();
            m_nodes[stk.top()].right = i;
        }

        if (last >= 0) {
            // last becomes left child of i
            m_nodes[last].parent = i;
            m_nodes[i].left = last;
        }

        stk.push(i);
    }

    // Find root: the bottom of the stack
    m_root = -1;
    while (!stk.empty()) {
        m_root = stk.top();
        stk.pop();
    }

    // Build Euler tour and sparse table for RMQ
    buildEulerTour();
    buildSparseTable();

    m_stats.treeSize = n;
    m_stats.eulerTourLength = m_euler.size();
    m_stats.treeHeight = computeHeight();
    m_stats.totalQueries = 0;
    m_timeSum += timer.elapsed();
    emit treeBuilt(n, m_stats.treeHeight, timer.elapsed());
}

/* ---- Build with explicit priorities ---- */

void CartesianTree7::buildWithPriorities(const QVector<QPair<double, int>>& entries)
{
    QElapsedTimer timer;
    timer.start();

    int n = entries.size();
    if (n == 0) return;

    m_nodes.resize(n);
    for (int i = 0; i < n; ++i) {
        m_nodes[i].value = entries[i].first;
        m_nodes[i].priority = entries[i].second;
        m_nodes[i].index = i;
        m_nodes[i].left = -1;
        m_nodes[i].right = -1;
        m_nodes[i].parent = -1;
    }

    // Treap insertion: value is BST key, priority is heap key
    // Sort by priority (max-heap: highest priority at root)
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [this](int a, int b) {
        return m_nodes[a].priority > m_nodes[b].priority;
    });

    // Insert in priority order using BST property on values
    m_root = order[0];
    for (int idx = 1; idx < n; ++idx) {
        int cur = order[idx];
        int node = m_root;
        int parent = -1;
        bool goLeft = false;

        while (node >= 0) {
            parent = node;
            if (m_nodes[cur].value < m_nodes[node].value) {
                goLeft = true;
                node = m_nodes[node].left;
            } else {
                goLeft = false;
                node = m_nodes[node].right;
            }
        }

        m_nodes[cur].parent = parent;
        if (parent >= 0) {
            if (goLeft) m_nodes[parent].left = cur;
            else m_nodes[parent].right = cur;
        }
    }

    buildEulerTour();
    buildSparseTable();

    m_stats.treeSize = n;
    m_stats.eulerTourLength = m_euler.size();
    m_stats.treeHeight = computeHeight();
    m_timeSum += timer.elapsed();
    emit treeBuilt(n, m_stats.treeHeight, timer.elapsed());
}

/* ---- Build Euler tour via DFS ---- */

void CartesianTree7::buildEulerTour()
{
    m_euler.clear();
    m_eulerDepth.clear();
    m_firstOcc.fill(-1, m_nodes.size());

    // Iterative DFS
    std::stack<QPair<int, int>> stk;  // (node, depth)
    stk.push({m_root, 0});

    while (!stk.empty()) {
        auto [node, depth] = stk.top();
        stk.pop();

        if (node < 0) continue;

        m_euler.append(node);
        m_eulerDepth.append(depth);
        if (m_firstOcc[node] < 0)
            m_firstOcc[node] = m_euler.size() - 1;

        // Push right first so left is processed first
        if (m_nodes[node].right >= 0)
            stk.push({m_nodes[node].right, depth + 1});
        if (m_nodes[node].left >= 0)
            stk.push({m_nodes[node].left, depth + 1});

        // Re-visit parent on way back (for proper Euler tour)
        if (m_nodes[node].left >= 0 || m_nodes[node].right >= 0) {
            m_euler.append(node);
            m_eulerDepth.append(depth);
        }
    }
}

/* ---- Build sparse table for RMQ on Euler depths ---- */

void CartesianTree7::buildSparseTable()
{
    int n = m_eulerDepth.size();
    if (n == 0) return;

    m_logTable = buildLogTable(n);

    int maxLog = m_logTable[n] + 1;
    m_sparseTable.resize(maxLog);
    m_sparseTable[0].resize(n);
    for (int i = 0; i < n; ++i)
        m_sparseTable[0][i] = i;

    for (int k = 1; k < maxLog; ++k) {
        int len = 1 << k;
        m_sparseTable[k].resize(n);
        for (int i = 0; i + len <= n; ++i) {
            int left = m_sparseTable[k - 1][i];
            int right = m_sparseTable[k - 1][i + (len >> 1)];
            m_sparseTable[k][i] = (m_eulerDepth[left] <= m_eulerDepth[right])
                ? left : right;
        }
    }
}

/* ---- Range minimum query using Euler tour + sparse table ---- */

QPair<double, int> CartesianTree7::rangeMinimumQuery(int l, int r) const
{
    QElapsedTimer timer;
    timer.start();

    if (m_nodes.isEmpty() || l < 0 || r >= m_nodes.size() || l > r)
        return {0.0, -1};

    // Find LCA via Euler tour RMQ
    int eulerL = m_firstOcc[l];
    int eulerR = m_firstOcc[r];
    if (eulerL > eulerR) std::swap(eulerL, eulerR);

    int len = eulerR - eulerL + 1;
    int k = m_logTable[len];
    int leftIdx = m_sparseTable[k][eulerL];
    int rightIdx = m_sparseTable[k][eulerR - (1 << k) + 1];
    int minIdx = (m_eulerDepth[leftIdx] <= m_eulerDepth[rightIdx])
        ? leftIdx : rightIdx;
    int lcaNode = m_euler[minIdx];

    // LCA value is the minimum in range [l..r]
    QPair<double, int> result = {m_nodes[lcaNode].value, lcaNode};

    auto self = const_cast<CartesianTree7*>(this);
    self->m_stats.totalQueries++;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, m_stats.totalQueries);
    self->emit queryCompleted(l, r, result.first, timer.elapsed());

    return result;
}

/* ---- Subtree height ---- */

int CartesianTree7::subtreeHeight(int nodeIdx) const
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return 0;
    int leftH = subtreeHeight(m_nodes[nodeIdx].left);
    int rightH = subtreeHeight(m_nodes[nodeIdx].right);
    return 1 + qMax(leftH, rightH);
}

int CartesianTree7::computeHeight() const
{
    return subtreeHeight(m_root);
}

/* ---- Getters ---- */

QVector<CartesianTree7::Node> CartesianTree7::nodes() const { return m_nodes; }
int CartesianTree7::root() const { return m_root; }
QVector<int> CartesianTree7::eulerTour() const { return m_euler; }
QVector<int> CartesianTree7::eulerDepths() const { return m_eulerDepth; }

/* ---- Reset ---- */

void CartesianTree7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_euler.clear();
    m_eulerDepth.clear();
    m_firstOcc.clear();
    m_sparseTable.clear();
    m_logTable.clear();
    m_root = -1;
}

/**
 * @file CartesianTree8.cpp
 * @brief CartesianTree8 实现
 *
 * 实现笛卡尔树分治构建、二进制提升LCA与O(1)稀疏表RMQ。
 */

#include "utils/tree227/CartesianTree8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <functional>

/* ---- Construction / Destruction ---- */

CartesianTree8::CartesianTree8(QObject *parent) : QObject(parent) {}
CartesianTree8::~CartesianTree8() = default;

/* ---- Divide-and-conquer build ---- */

int CartesianTree8::buildDC(const QVector<int>& values, int lo, int hi)
{
    if (lo > hi) return -1;
    if (lo == hi) {
        int nodeId = m_treeSize++;
        m_nodes[nodeId].value = values[lo];
        m_nodes[nodeId].index = lo;
        m_nodes[nodeId].parent = -1;
        m_nodes[nodeId].left = -1;
        m_nodes[nodeId].right = -1;
        m_indexToNode[lo] = nodeId;
        return nodeId;
    }

    // Find minimum in [lo, hi]
    int minIdx = lo;
    for (int i = lo + 1; i <= hi; ++i)
        if (values[i] < values[minIdx]) minIdx = i;

    int nodeId = m_treeSize++;
    m_nodes[nodeId].value = values[minIdx];
    m_nodes[nodeId].index = minIdx;
    m_nodes[nodeId].parent = -1;

    // Recursively build left and right subtrees
    int leftChild = buildDC(values, lo, minIdx - 1);
    int rightChild = buildDC(values, minIdx + 1, hi);

    m_nodes[nodeId].left = leftChild;
    m_nodes[nodeId].right = rightChild;
    if (leftChild >= 0) m_nodes[leftChild].parent = nodeId;
    if (rightChild >= 0) m_nodes[rightChild].parent = nodeId;

    m_indexToNode[minIdx] = nodeId;
    return nodeId;
}

/* ---- Build binary lifting table ---- */

void CartesianTree8::buildLifting()
{
    if (m_treeSize == 0) return;
    m_maxLog = 0;
    int tmp = m_treeSize;
    while (tmp > 1) { tmp >>= 1; m_maxLog++; }
    m_maxLog++;

    m_up.resize(m_treeSize, QVector<int>(m_maxLog, -1));
    m_depth.resize(m_treeSize, 0);

    // BFS to set depth and first ancestor
    QVector<int> queue;
    queue.append(m_root);
    m_depth[m_root] = 0;
    m_up[m_root][0] = -1;

    int head = 0;
    while (head < queue.size()) {
        int u = queue[head++];
        for (int k = 1; k < m_maxLog; ++k) {
            if (m_up[u][k - 1] >= 0)
                m_up[u][k] = m_up[m_up[u][k - 1]][k - 1];
        }
        for (int child : {m_nodes[u].left, m_nodes[u].right}) {
            if (child >= 0) {
                m_depth[child] = m_depth[u] + 1;
                m_up[child][0] = u;
                queue.append(child);
            }
        }
    }
}

/* ---- Euler DFS ---- */

void CartesianTree8::eulerDFS(int node, int depth)
{
    m_euler.append(node);
    m_eulerDepth.append(depth);
    m_firstOccurrence[node] = m_euler.size() - 1;

    for (int child : {m_nodes[node].left, m_nodes[node].right}) {
        if (child >= 0) {
            eulerDFS(child, depth + 1);
            m_euler.append(node);
            m_eulerDepth.append(depth);
        }
    }
}

/* ---- Build Euler tour sparse table ---- */

void CartesianTree8::buildEulerSparse()
{
    if (m_treeSize == 0) return;
    m_euler.clear();
    m_eulerDepth.clear();
    m_firstOccurrence.resize(m_treeSize, -1);
    eulerDFS(m_root, 0);

    int n = m_euler.size();
    if (n == 0) return;

    // Build log table
    m_logTable.resize(n + 1, 0);
    for (int i = 2; i <= n; ++i)
        m_logTable[i] = m_logTable[i / 2] + 1;

    // Sparse table stores index of minimum depth
    int maxK = m_logTable[n] + 1;
    m_sparseTable.resize(n, QVector<int>(maxK));

    for (int i = 0; i < n; ++i) m_sparseTable[i][0] = i;
    for (int k = 1; k < maxK; ++k) {
        for (int i = 0; i + (1 << k) <= n; ++i) {
            int a = m_sparseTable[i][k - 1];
            int b = m_sparseTable[i + (1 << (k - 1))][k - 1];
            m_sparseTable[i][k] = (m_eulerDepth[a] <= m_eulerDepth[b]) ? a : b;
        }
    }
}

/* ---- RMQ on Euler depth sparse table ---- */

int CartesianTree8::rmqSparse(int l, int r) const
{
    int k = m_logTable[r - l + 1];
    int a = m_sparseTable[l][k];
    int b = m_sparseTable[r - (1 << k) + 1][k];
    return (m_eulerDepth[a] <= m_eulerDepth[b]) ? a : b;
}

/* ---- Build tree ---- */

bool CartesianTree8::build(const QVector<int>& values)
{
    QElapsedTimer timer;
    timer.start();

    int n = values.size();
    if (n == 0) return false;

    m_nodes.resize(n);
    m_indexToNode.resize(n, -1);
    m_treeSize = 0;

    m_root = buildDC(values, 0, n - 1);

    // Build LCA structures
    buildLifting();
    buildEulerSparse();

    int h = height();
    m_stats.treeSize = n;
    m_stats.treeHeight = h;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeBuilt(n, h, timer.elapsed());
    return true;
}

/* ---- LCA via binary lifting ---- */

int CartesianTree8::lcaLifting(int u, int v) const
{
    if (m_depth[u] < m_depth[v]) std::swap(u, v);
    int diff = m_depth[u] - m_depth[v];
    for (int k = 0; k < m_maxLog; ++k)
        if ((diff >> k) & 1) u = m_up[u][k];
    if (u == v) return u;
    for (int k = m_maxLog - 1; k >= 0; --k) {
        if (m_up[u][k] != m_up[v][k]) {
            u = m_up[u][k];
            v = m_up[v][k];
        }
    }
    return m_up[u][0];
}

/* ---- LCA query ---- */

CartesianTree8::LCAResult CartesianTree8::lca(int indexA, int indexB) const
{
    QElapsedTimer timer;
    timer.start();

    LCAResult result;
    if (indexA < 0 || indexA >= m_indexToNode.size() ||
        indexB < 0 || indexB >= m_indexToNode.size()) return result;

    int nodeA = m_indexToNode[indexA];
    int nodeB = m_indexToNode[indexB];
    if (nodeA < 0 || nodeB < 0) return result;

    // Use Euler tour + sparse table for O(1) LCA
    int firstA = m_firstOccurrence[nodeA];
    int firstB = m_firstOccurrence[nodeB];
    if (firstA > firstB) std::swap(firstA, firstB);
    int rmqIdx = rmqSparse(firstA, firstB);
    int lcaNode = m_euler[rmqIdx];

    result.lcaNode = lcaNode;
    result.lcaIndex = m_nodes[lcaNode].index;
    result.lcaValue = m_nodes[lcaNode].value;

    const_cast<CartesianTree8*>(this)->m_stats.numLCAQueries++;
    const_cast<CartesianTree8*>(this)->m_timeSum += timer.elapsed();
    const_cast<CartesianTree8*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / (++const_cast<CartesianTree8*>(this)->m_stats.totalOps);
    emit const_cast<CartesianTree8*>(this)->lcaQueried(indexA, indexB, lcaNode, timer.elapsed());
    return result;
}

/* ---- Range minimum via Cartesian tree ---- */

CartesianTree8::LCAResult CartesianTree8::rangeMinimum(int l, int r) const
{
    return lca(l, r);
}

/* ---- Tree height ---- */

int CartesianTree8::height() const
{
    if (m_treeSize == 0) return 0;
    int maxDepth = 0;
    for (int i = 0; i < m_treeSize; ++i)
        maxDepth = qMax(maxDepth, m_depth[i]);
    return maxDepth + 1;
}

/* ---- Inorder traversal ---- */

QVector<int> CartesianTree8::inorder() const
{
    QVector<int> result;
    if (m_root < 0) return result;

    // Inorder: left, node, right (yields sorted by index)
    QVector<int> stack;
    int cur = m_root;
    while (cur >= 0 || !stack.isEmpty()) {
        while (cur >= 0) {
            stack.append(cur);
            cur = m_nodes[cur].left;
        }
        cur = stack.takeLast();
        result.append(m_nodes[cur].index);
        cur = m_nodes[cur].right;
    }
    return result;
}

/* ---- Reset ---- */

void CartesianTree8::resetStatistics()
{
    m_nodes.clear();
    m_indexToNode.clear();
    m_up.clear();
    m_depth.clear();
    m_euler.clear();
    m_eulerDepth.clear();
    m_firstOccurrence.clear();
    m_sparseTable.clear();
    m_logTable.clear();
    m_treeSize = 0;
    m_root = -1;
    m_maxLog = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

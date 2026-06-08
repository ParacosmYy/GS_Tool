/**
 * @file WeightBalancedTree6.cpp
 * @brief WeightBalancedTree6 实现
 *
 * 实现权重平衡树：Alpha平衡检查、替罪羊批量重建、插入/删除/搜索。
 */

#include "utils/tree214/WeightBalancedTree6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WeightBalancedTree6::WeightBalancedTree6(QObject *parent) : QObject(parent) {}
WeightBalancedTree6::~WeightBalancedTree6() = default;

/* ---- Configuration ---- */

void WeightBalancedTree6::setAlpha(double alpha)
{
    m_alpha = qBound(0.51, alpha, 0.99);
}

/* ---- Allocate node ---- */

int WeightBalancedTree6::allocateNode(double key, int value)
{
    Node n;
    n.key = key;
    n.value = value;
    n.left = -1;
    n.right = -1;
    n.weight = 1;
    m_nodes.append(n);
    return m_nodes.size() - 1;
}

/* ---- Update weight ---- */

void WeightBalancedTree6::updateWeight(int node)
{
    if (node < 0 || node >= m_nodes.size()) return;
    int w = 1;
    if (m_nodes[node].left >= 0)
        w += m_nodes[m_nodes[node].left].weight;
    if (m_nodes[node].right >= 0)
        w += m_nodes[m_nodes[node].right].weight;
    m_nodes[node].weight = w;
}

/* ---- Alpha-balance check ---- */

bool WeightBalancedTree6::isAlphaBalanced(int node) const
{
    if (node < 0 || node >= m_nodes.size()) return true;
    int leftW = (m_nodes[node].left >= 0) ? m_nodes[m_nodes[node].left].weight : 0;
    int rightW = (m_nodes[node].right >= 0) ? m_nodes[m_nodes[node].right].weight : 0;
    int total = leftW + rightW + 1;
    if (total <= 2) return true;
    return leftW <= m_alpha * total && rightW <= m_alpha * total;
}

/* ---- Flatten subtree ---- */

void WeightBalancedTree6::flatten(int node, QVector<int>& out) const
{
    if (node < 0 || node >= m_nodes.size()) return;
    flatten(m_nodes[node].left, out);
    out.append(node);
    flatten(m_nodes[node].right, out);
}

/* ---- Build balanced from sorted node indices ---- */

int WeightBalancedTree6::buildBalanced(const QVector<int>& sorted,
                                        int start, int end)
{
    if (start > end) return -1;
    int mid = (start + end) / 2;
    int node = sorted[mid];

    m_nodes[node].left = buildBalanced(sorted, start, mid - 1);
    m_nodes[node].right = buildBalanced(sorted, mid + 1, end);
    updateWeight(node);
    return node;
}

/* ---- Rebuild subtree ---- */

int WeightBalancedTree6::rebuild(int node)
{
    QVector<int> flat;
    flatten(node, flat);
    // flat is already sorted by key due to in-order traversal
    int newRoot = buildBalanced(flat, 0, flat.size() - 1);
    m_stats.rebuildCount++;
    return newRoot;
}

/* ---- Find scapegoat ---- */

int WeightBalancedTree6::findScapegoat(int node, double key) const
{
    if (node < 0) return -1;
    int child = (key < m_nodes[node].key) ? m_nodes[node].left : m_nodes[node].right;
    if (child < 0 || isAlphaBalanced(node))
        return node;
    return findScapegoat(child, key);
}

/* ---- Recursive insert ---- */

int WeightBalancedTree6::insertRec(int node, double key, int value, bool& rebuilt)
{
    if (node < 0) return allocateNode(key, value);

    if (key < m_nodes[node].key)
        m_nodes[node].left = insertRec(m_nodes[node].left, key, value, rebuilt);
    else if (key > m_nodes[node].key)
        m_nodes[node].right = insertRec(m_nodes[node].right, key, value, rebuilt);
    else {
        m_nodes[node].value = value;  // Update existing
        return node;
    }

    updateWeight(node);

    // Check alpha-balance and rebuild if needed (scapegoat strategy)
    if (!rebuilt && !isAlphaBalanced(node)) {
        node = rebuild(node);
        rebuilt = true;
    }
    return node;
}

/* ---- Insert ---- */

void WeightBalancedTree6::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    bool rebuilt = false;
    m_root = insertRec(m_root, key, value, rebuilt);

    m_stats.totalOps++;
    m_stats.treeSize = (m_root >= 0) ? m_nodes[m_root].weight : 0;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.alpha = m_alpha;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_stats.treeSize, m_stats.treeHeight, timer.elapsed());
}

/* ---- Remove ---- */

void WeightBalancedTree6::remove(double key)
{
    // Simplified: rebuild tree without the key
    QVector<QPair<double, int>> flat;
    inOrderRec(m_root, flat);

    QVector<QPair<double, int>> filtered;
    filtered.reserve(flat.size());
    for (const auto& [k, v] : flat) {
        if (k != key) filtered.append({k, v});
    }

    m_nodes.clear();
    m_root = -1;
    if (!filtered.isEmpty()) {
        // Build balanced tree from sorted data
        for (const auto& [k, v] : filtered)
            allocateNode(k, v);
        QVector<int> indices(m_nodes.size());
        for (int i = 0; i < m_nodes.size(); ++i) indices[i] = i;
        m_root = buildBalanced(indices, 0, indices.size() - 1);
    }

    m_stats.totalOps++;
    m_stats.treeSize = (m_root >= 0) ? m_nodes[m_root].weight : 0;
    m_stats.treeHeight = computeHeight(m_root);
}

/* ---- Search ---- */

int WeightBalancedTree6::search(double key) const
{
    int node = m_root;
    while (node >= 0 && node < m_nodes.size()) {
        if (key < m_nodes[node].key)
            node = m_nodes[node].left;
        else if (key > m_nodes[node].key)
            node = m_nodes[node].right;
        else
            return m_nodes[node].value;
    }
    return -1;
}

/* ---- In-order traversal ---- */

void WeightBalancedTree6::inOrderRec(int node,
                                      QVector<QPair<double, int>>& result) const
{
    if (node < 0 || node >= m_nodes.size()) return;
    inOrderRec(m_nodes[node].left, result);
    result.append({m_nodes[node].key, m_nodes[node].value});
    inOrderRec(m_nodes[node].right, result);
}

QVector<QPair<double, int>> WeightBalancedTree6::inOrder() const
{
    QVector<QPair<double, int>> result;
    result.reserve(m_stats.treeSize);
    inOrderRec(m_root, result);
    return result;
}

/* ---- Balance check ---- */

bool WeightBalancedTree6::isBalanced() const
{
    return isAlphaBalanced(m_root);
}

/* ---- Bulk rebuild ---- */

void WeightBalancedTree6::bulkRebuild(const QVector<QPair<double, int>>& sortedData)
{
    QElapsedTimer timer;
    timer.start();

    m_nodes.clear();
    m_root = -1;
    if (sortedData.isEmpty()) return;

    for (const auto& [k, v] : sortedData)
        allocateNode(k, v);

    QVector<int> indices(m_nodes.size());
    for (int i = 0; i < m_nodes.size(); ++i) indices[i] = i;
    m_root = buildBalanced(indices, 0, indices.size() - 1);

    m_stats.rebuildCount++;
    m_stats.treeSize = m_nodes[m_root].weight;
    m_stats.treeHeight = computeHeight(m_root);
    emit rebuildTriggered(m_stats.treeSize, timer.elapsed());
}

/* ---- Compute height ---- */

int WeightBalancedTree6::computeHeight(int node) const
{
    if (node < 0 || node >= m_nodes.size()) return 0;
    int lh = computeHeight(m_nodes[node].left);
    int rh = computeHeight(m_nodes[node].right);
    return 1 + qMax(lh, rh);
}

/* ---- Reset ---- */

void WeightBalancedTree6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_root = -1;
}

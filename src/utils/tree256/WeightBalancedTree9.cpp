/**
 * @file WeightBalancedTree9.cpp
 * @brief WeightBalancedTree9 实现
 *
 * 实现权重平衡树：秩平衡不变量与批量更新全局重建。
 */

#include "utils/tree256/WeightBalancedTree9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WeightBalancedTree9::WeightBalancedTree9(QObject *parent)
    : QObject(parent) {}
WeightBalancedTree9::~WeightBalancedTree9() = default;

/* ---- Configuration ---- */

void WeightBalancedTree9::setAlpha(double alpha)
{
    m_alpha = qBound(0.25, alpha, 0.5);
}
void WeightBalancedTree9::setRebuildThreshold(int threshold)
{
    m_rebuildThreshold = qMax(10, threshold);
}

/* ---- Node allocation / deallocation ---- */

int WeightBalancedTree9::allocNode(int key)
{
    int idx;
    if (m_freeList >= 0) {
        idx = m_freeList;
        m_freeList = m_nodes[idx].right; // Reuse right as next-free link
    } else {
        idx = m_nodes.size();
        m_nodes.append(Node());
    }
    m_nodes[idx].key = key;
    m_nodes[idx].weight = 1;
    m_nodes[idx].height = 1;
    m_nodes[idx].left = -1;
    m_nodes[idx].right = -1;
    m_nodes[idx].parent = -1;
    return idx;
}

void WeightBalancedTree9::freeNode(int idx)
{
    m_nodes[idx].right = m_freeList;
    m_freeList = idx;
}

/* ---- Weight / height helpers ---- */

int WeightBalancedTree9::weight(int idx) const
{
    return (idx >= 0 && idx < m_nodes.size()) ? m_nodes[idx].weight : 0;
}

void WeightBalancedTree9::updateNode(int idx)
{
    if (idx < 0) return;
    int lw = weight(m_nodes[idx].left);
    int rw = weight(m_nodes[idx].right);
    m_nodes[idx].weight = 1 + lw + rw;

    int lh = (m_nodes[idx].left >= 0) ? m_nodes[m_nodes[idx].left].height : 0;
    int rh = (m_nodes[idx].right >= 0) ? m_nodes[m_nodes[idx].right].height : 0;
    m_nodes[idx].height = 1 + qMax(lh, rh);
}

/* ---- Check alpha-weight balance ---- */

bool WeightBalancedTree9::isBalanced(int idx) const
{
    if (idx < 0) return true;
    int lw = weight(m_nodes[idx].left);
    int rw = weight(m_nodes[idx].right);
    int total = lw + rw + 1;
    if (total <= 2) return true;
    double minW = m_alpha * total;
    return lw >= minW && rw >= minW;
}

/* ---- Find unbalanced ancestor ---- */

int WeightBalancedTree9::findUnbalanced(int start) const
{
    int idx = start;
    while (idx >= 0) {
        if (!isBalanced(idx)) return idx;
        idx = m_nodes[idx].parent;
    }
    return -1;
}

/* ---- Flatten subtree to sorted array ---- */

void WeightBalancedTree9::flatten(int idx, QVector<int>& keys) const
{
    if (idx < 0) return;
    flatten(m_nodes[idx].left, keys);
    keys.append(m_nodes[idx].key);
    flatten(m_nodes[idx].right, keys);
}

/* ---- Build balanced tree from sorted keys ---- */

int WeightBalancedTree9::buildBalanced(const QVector<int>& keys, int lo, int hi)
{
    if (lo > hi) return -1;
    int mid = (lo + hi) / 2;
    int idx = allocNode(keys[mid]);

    m_nodes[idx].left = buildBalanced(keys, lo, mid - 1);
    if (m_nodes[idx].left >= 0)
        m_nodes[m_nodes[idx].left].parent = idx;

    m_nodes[idx].right = buildBalanced(keys, mid + 1, hi);
    if (m_nodes[idx].right >= 0)
        m_nodes[m_nodes[idx].right].parent = idx;

    updateNode(idx);
    return idx;
}

/* ---- Rebalance subtree ---- */

int WeightBalancedTree9::rebalance(int root)
{
    if (root < 0) return -1;
    QVector<int> keys;
    flatten(root, keys);
    int parent = m_nodes[root].parent;

    // Free old nodes
    QVector<int> toFree;
    flatten(root, toFree); // Reuse flatten for node collection
    // Collect node indices (not keys) - use separate helper
    QVector<int> nodeIdxs;
    {
        QVector<int> stack;
        stack.append(root);
        while (!stack.isEmpty()) {
            int n = stack.takeLast();
            if (n < 0) continue;
            nodeIdxs.append(n);
            stack.append(m_nodes[n].left);
            stack.append(m_nodes[n].right);
        }
    }

    // Build new balanced tree
    int oldRoot = root;
    // Temporarily invalidate freed nodes
    for (int n : nodeIdxs) {
        m_nodes[n].left = -1;
        m_nodes[n].right = -1;
        m_nodes[n].parent = -1;
    }
    // Re-alloc from free list won't work cleanly, use indices directly
    QVector<Node> oldNodes = m_nodes;
    int newRoot = buildBalanced(keys, 0, keys.size() - 1);
    m_nodes = oldNodes;
    newRoot = buildBalanced(keys, 0, keys.size() - 1);

    m_nodes[newRoot].parent = parent;
    m_stats.numRebalances++;
    return newRoot;
}

/* ---- Global rebuild ---- */

void WeightBalancedTree9::globalRebuild()
{
    if (m_root < 0) return;
    QVector<int> keys;
    flatten(m_root, keys);

    // Reset node pool
    m_nodes.clear();
    m_freeList = -1;

    // Build perfectly balanced tree
    m_root = buildBalanced(keys, 0, keys.size() - 1);
    m_stats.numGlobalRebuilds++;
    m_batchOpsSinceRebuild = 0;
}

/* ---- Find key ---- */

int WeightBalancedTree9::find(int key) const
{
    int idx = m_root;
    while (idx >= 0) {
        if (key == m_nodes[idx].key) return idx;
        idx = (key < m_nodes[idx].key) ? m_nodes[idx].left : m_nodes[idx].right;
    }
    return -1;
}

/* ---- Find minimum ---- */

int WeightBalancedTree9::findMin(int idx) const
{
    while (idx >= 0 && m_nodes[idx].left >= 0)
        idx = m_nodes[idx].left;
    return idx;
}

/* ---- Insert ---- */

void WeightBalancedTree9::insert(int key)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root < 0) {
        m_root = allocNode(key);
        m_stats.numNodes = 1;
        m_stats.numInserts++;
        m_stats.totalOps++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
        emit treeUpdated(m_stats.numNodes, height(), elapsed);
        return;
    }

    // Standard BST insert
    int parent = -1;
    int cur = m_root;
    while (cur >= 0) {
        parent = cur;
        if (key == m_nodes[cur].key) return; // Duplicate
        cur = (key < m_nodes[cur].key) ? m_nodes[cur].left : m_nodes[cur].right;
    }

    int newIdx = allocNode(key);
    m_nodes[newIdx].parent = parent;
    if (key < m_nodes[parent].key)
        m_nodes[parent].left = newIdx;
    else
        m_nodes[parent].right = newIdx;

    // Update weights up to root
    int up = parent;
    while (up >= 0) {
        updateNode(up);
        up = m_nodes[up].parent;
    }

    // Check balance and rebalance if needed
    int unbalanced = findUnbalanced(newIdx);
    if (unbalanced >= 0) {
        int ubParent = m_nodes[unbalanced].parent;
        int newSub = rebalance(unbalanced);
        if (ubParent < 0) {
            m_root = newSub;
        } else {
            if (m_nodes[ubParent].left == unbalanced)
                m_nodes[ubParent].left = newSub;
            else
                m_nodes[ubParent].right = newSub;
        }
    }

    m_batchOpsSinceRebuild++;
    if (m_batchOpsSinceRebuild >= m_rebuildThreshold)
        globalRebuild();

    m_stats.numInserts++;
    m_stats.numNodes = weight(m_root);
    m_stats.totalOps++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_stats.numNodes, height(), elapsed);
}

/* ---- Remove ---- */

void WeightBalancedTree9::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    int idx = find(key);
    if (idx < 0) return;

    int parent = m_nodes[idx].parent;
    int left = m_nodes[idx].left;
    int right = m_nodes[idx].right;

    int replacement = -1;
    if (left < 0 && right < 0) {
        replacement = -1;
    } else if (left < 0) {
        replacement = right;
    } else if (right < 0) {
        replacement = left;
    } else {
        // Find in-order successor
        int succ = findMin(right);
        replacement = succ;
        // Detach successor
        int succParent = m_nodes[succ].parent;
        if (succParent == idx) {
            m_nodes[succ].left = left;
            if (left >= 0) m_nodes[left].parent = succ;
            updateNode(succ);
        } else {
            m_nodes[succParent].left = m_nodes[succ].right;
            if (m_nodes[succ].right >= 0)
                m_nodes[m_nodes[succ].right].parent = succParent;
            m_nodes[succ].left = left;
            m_nodes[succ].right = right;
            if (left >= 0) m_nodes[left].parent = succ;
            if (right >= 0) m_nodes[right].parent = succ;
        }
        m_nodes[succ].parent = parent;
    }

    if (replacement >= 0 && replacement != idx)
        m_nodes[replacement].parent = parent;

    if (parent < 0) {
        m_root = replacement;
    } else {
        if (m_nodes[parent].left == idx)
            m_nodes[parent].left = replacement;
        else
            m_nodes[parent].right = replacement;
    }

    // Update weights
    int up = (replacement >= 0 && replacement != idx) ? replacement : parent;
    while (up >= 0) {
        updateNode(up);
        up = m_nodes[up].parent;
    }

    // Check balance
    if (parent >= 0) {
        int unbalanced = findUnbalanced(parent);
        if (unbalanced >= 0) {
            int ubParent = m_nodes[unbalanced].parent;
            int newSub = rebalance(unbalanced);
            if (ubParent < 0) m_root = newSub;
            else {
                if (m_nodes[ubParent].left == unbalanced)
                    m_nodes[ubParent].left = newSub;
                else
                    m_nodes[ubParent].right = newSub;
            }
        }
    }

    m_batchOpsSinceRebuild++;
    if (m_batchOpsSinceRebuild >= m_rebuildThreshold)
        globalRebuild();

    m_stats.numDeletes++;
    m_stats.numNodes = (m_root >= 0) ? weight(m_root) : 0;
    m_stats.totalOps++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_stats.numNodes, height(), elapsed);
}

/* ---- Contains ---- */

bool WeightBalancedTree9::contains(int key) const { return find(key) >= 0; }

/* ---- Batch insert ---- */

void WeightBalancedTree9::batchInsert(const QVector<int>& keys)
{
    for (int k : keys) insert(k);
}

/* ---- In-order traversal ---- */

void WeightBalancedTree9::inOrderHelper(int idx, QVector<int>& result) const
{
    if (idx < 0) return;
    inOrderHelper(m_nodes[idx].left, result);
    result.append(m_nodes[idx].key);
    inOrderHelper(m_nodes[idx].right, result);
}

QVector<int> WeightBalancedTree9::inOrder() const
{
    QVector<int> result;
    inOrderHelper(m_root, result);
    return result;
}

/* ---- Height / Size ---- */

int WeightBalancedTree9::height() const
{
    return (m_root >= 0) ? m_nodes[m_root].height : 0;
}
int WeightBalancedTree9::size() const
{
    return (m_root >= 0) ? m_nodes[m_root].weight : 0;
}

/* ---- Reset ---- */

void WeightBalancedTree9::resetStatistics()
{
    m_nodes.clear();
    m_root = -1;
    m_freeList = -1;
    m_batchOpsSinceRebuild = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

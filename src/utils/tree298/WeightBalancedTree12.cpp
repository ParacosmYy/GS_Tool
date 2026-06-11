/**
 * @file WeightBalancedTree12.cpp
 * @brief WeightBalancedTree12 实现
 *
 * 实现权重平衡树：alpha平衡旋转与秩增强节点实现保证深度界的确定性平衡BST。
 */

#include "utils/tree298/WeightBalancedTree12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WeightBalancedTree12::WeightBalancedTree12(QObject *parent)
    : QObject(parent) {}

WeightBalancedTree12::~WeightBalancedTree12() = default;

/* ---- Configuration ---- */

void WeightBalancedTree12::setAlpha(double alpha)
{
    // Alpha must be in (0, 1 - 1/sqrt(2)] ~ (0, 0.293]
    m_alpha = qBound(0.1, alpha, 0.5);
}

/* ---- Node allocation ---- */

int WeightBalancedTree12::allocNode(double key)
{
    Node n;
    n.key = key;
    n.weight = 1;
    n.rank = 1;
    n.left = -1;
    n.right = -1;
    n.parent = -1;
    m_nodes.append(n);
    return m_nodes.size() - 1;
}

/* ---- Get weight of subtree ---- */

int WeightBalancedTree12::weight(int idx) const
{
    return (idx >= 0 && idx < m_nodes.size()) ? m_nodes[idx].weight : 0;
}

/* ---- Update weight and rank ---- */

void WeightBalancedTree12::updateMeta(int idx)
{
    if (idx < 0 || idx >= m_nodes.size()) return;
    Node& n = m_nodes[idx];
    n.weight = 1 + weight(n.left) + weight(n.right);
    n.rank = 1 + qMax(
        (n.left >= 0) ? m_nodes[n.left].rank : 0,
        (n.right >= 0) ? m_nodes[n.right].rank : 0);
}

/* ---- Rotate left ---- */

int WeightBalancedTree12::rotateLeft(int idx)
{
    int r = m_nodes[idx].right;
    if (r < 0) return idx;

    // Re-wire
    m_nodes[idx].right = m_nodes[r].left;
    if (m_nodes[r].left >= 0)
        m_nodes[m_nodes[r].left].parent = idx;

    m_nodes[r].parent = m_nodes[idx].parent;
    m_nodes[idx].parent = r;
    m_nodes[r].left = idx;

    // Update metadata bottom-up
    updateMeta(idx);
    updateMeta(r);
    return r;
}

/* ---- Rotate right ---- */

int WeightBalancedTree12::rotateRight(int idx)
{
    int l = m_nodes[idx].left;
    if (l < 0) return idx;

    m_nodes[idx].left = m_nodes[l].right;
    if (m_nodes[l].right >= 0)
        m_nodes[m_nodes[l].right].parent = idx;

    m_nodes[l].parent = m_nodes[idx].parent;
    m_nodes[idx].parent = l;
    m_nodes[l].right = idx;

    updateMeta(idx);
    updateMeta(l);
    return l;
}

/* ---- Rebalance: check alpha-balance and rotate ---- */

int WeightBalancedTree12::rebalance(int idx)
{
    if (idx < 0) return idx;

    updateMeta(idx);
    int wl = weight(m_nodes[idx].left);
    int wr = weight(m_nodes[idx].right);
    int wt = m_nodes[idx].weight;

    bool leftHeavy = (wl > m_alpha * wt);
    bool rightHeavy = (wr > m_alpha * wt);

    if (leftHeavy) {
        // Left-left or left-right case
        int left = m_nodes[idx].left;
        int wll = weight(m_nodes[left].left);
        int wlr = weight(m_nodes[left].right);
        if (wll < wlr) {
            // Left-right: first rotate left child left
            m_nodes[idx].left = rotateLeft(left);
        }
        return rotateRight(idx);
    }

    if (rightHeavy) {
        int right = m_nodes[idx].right;
        int wrl = weight(m_nodes[right].left);
        int wrr = weight(m_nodes[right].right);
        if (wrr < wrl) {
            // Right-left: first rotate right child right
            m_nodes[idx].right = rotateRight(right);
        }
        return rotateLeft(idx);
    }

    return idx;
}

/* ---- Insert ---- */

bool WeightBalancedTree12::insert(double key)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root < 0) {
        m_root = allocNode(key);
        m_stats.treeSize = m_nodes.size();
        m_stats.totalInserts++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum /
            (m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalSearches);
        emit insertDone(key, m_stats.treeSize, timer.elapsed());
        return true;
    }

    // BST insert
    int cur = m_root;
    int parent = -1;
    bool isLeft = false;

    while (cur >= 0) {
        parent = cur;
        if (key < m_nodes[cur].key) {
            cur = m_nodes[cur].left;
            isLeft = true;
        } else if (key > m_nodes[cur].key) {
            cur = m_nodes[cur].right;
            isLeft = false;
        } else {
            // Duplicate key
            return false;
        }
    }

    int newIdx = allocNode(key);
    m_nodes[newIdx].parent = parent;

    if (isLeft)
        m_nodes[parent].left = newIdx;
    else
        m_nodes[parent].right = newIdx;

    // Walk up and rebalance
    int idx = parent;
    while (idx >= 0) {
        int p = m_nodes[idx].parent;
        int newIdx2 = rebalance(idx);

        if (p >= 0) {
            if (m_nodes[p].left == idx)
                m_nodes[p].left = newIdx2;
            else
                m_nodes[p].right = newIdx2;
        } else {
            m_root = newIdx2;
        }
        idx = p;
    }

    m_stats.treeSize = m_nodes.size();
    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalSearches);

    emit insertDone(key, m_stats.treeSize, timer.elapsed());
    return true;
}

/* ---- Remove ---- */

bool WeightBalancedTree12::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    // Find node
    int cur = m_root;
    while (cur >= 0) {
        if (key < m_nodes[cur].key) cur = m_nodes[cur].left;
        else if (key > m_nodes[cur].key) cur = m_nodes[cur].right;
        else break;
    }
    if (cur < 0) return false;

    // BST remove: find in-order successor if two children
    int toRemove = cur;
    if (m_nodes[cur].left >= 0 && m_nodes[cur].right >= 0) {
        int succ = m_nodes[cur].right;
        while (m_nodes[succ].left >= 0)
            succ = m_nodes[succ].left;
        m_nodes[cur].key = m_nodes[succ].key;
        toRemove = succ;
    }

    // Unlink toRemove (at most one child)
    int child = (m_nodes[toRemove].left >= 0)
        ? m_nodes[toRemove].left
        : m_nodes[toRemove].right;

    int parent = m_nodes[toRemove].parent;
    if (child >= 0)
        m_nodes[child].parent = parent;

    if (parent < 0) {
        m_root = child;
    } else {
        if (m_nodes[parent].left == toRemove)
            m_nodes[parent].left = child;
        else
            m_nodes[parent].right = child;
    }

    // Rebalance up from parent
    int idx = parent;
    while (idx >= 0) {
        int p = m_nodes[idx].parent;
        int newIdx2 = rebalance(idx);
        if (p >= 0) {
            if (m_nodes[p].left == idx)
                m_nodes[p].left = newIdx2;
            else
                m_nodes[p].right = newIdx2;
        } else {
            m_root = newIdx2;
        }
        idx = p;
    }

    m_stats.treeSize = m_nodes.size();
    m_stats.totalDeletes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalSearches);

    emit removeDone(key, m_stats.treeSize, timer.elapsed());
    return true;
}

/* ---- Search ---- */

WeightBalancedTree12::SearchResult WeightBalancedTree12::search(double key) const
{
    SearchResult result;
    int cur = m_root;
    int depth = 0;
    int comparisons = 0;

    while (cur >= 0) {
        comparisons++;
        if (key < m_nodes[cur].key) {
            cur = m_nodes[cur].left;
            depth++;
        } else if (key > m_nodes[cur].key) {
            cur = m_nodes[cur].right;
            depth++;
        } else {
            result.found = true;
            result.nodeIndex = cur;
            result.depth = depth;
            result.comparisons = comparisons;
            return result;
        }
    }

    result.found = false;
    result.depth = depth;
    result.comparisons = comparisons;
    return result;
}

/* ---- In-order traversal ---- */

void WeightBalancedTree12::inOrderHelper(int idx, QVector<double>& result) const
{
    if (idx < 0 || idx >= m_nodes.size()) return;
    inOrderHelper(m_nodes[idx].left, result);
    result.append(m_nodes[idx].key);
    inOrderHelper(m_nodes[idx].right, result);
}

QVector<double> WeightBalancedTree12::inOrder() const
{
    QVector<double> result;
    inOrderHelper(m_root, result);
    return result;
}

/* ---- Reset ---- */

void WeightBalancedTree12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_root = -1;
}

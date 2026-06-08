/**
 * @file WeightBalancedTree7.cpp
 * @brief WeightBalancedTree7 实现
 *
 * 实现权平衡树：松弛平衡准则与并发更新局部旋转重平衡。
 */

#include "utils/tree228/WeightBalancedTree7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

WeightBalancedTree7::WeightBalancedTree7(QObject *parent) : QObject(parent) {}
WeightBalancedTree7::~WeightBalancedTree7() = default;

/* ---- Configuration ---- */

void WeightBalancedTree7::setAlpha(double alpha)
{
    m_alpha = qBound(0.25, alpha, 0.5);
}

/* ---- Allocate node ---- */

int WeightBalancedTree7::allocateNode(int key, double value)
{
    int idx;
    if (m_freeList >= 0) {
        idx = m_freeList;
        m_freeList = m_nodes[idx].right;
    } else {
        idx = m_nodes.size();
        m_nodes.append(Node());
    }
    m_nodes[idx].key = key;
    m_nodes[idx].value = value;
    m_nodes[idx].weight = 1;
    m_nodes[idx].height = 1;
    m_nodes[idx].left = -1;
    m_nodes[idx].right = -1;
    m_nodes[idx].parent = -1;
    m_nodes[idx].needsRebalance = false;
    return idx;
}

/* ---- Free node ---- */

void WeightBalancedTree7::freeNode(int idx)
{
    m_nodes[idx].right = m_freeList;
    m_nodes[idx].left = -1;
    m_freeList = idx;
}

/* ---- Update aggregate (weight/height) ---- */

void WeightBalancedTree7::updateAggregate(int idx)
{
    if (idx < 0) return;
    int lw = (m_nodes[idx].left >= 0) ? m_nodes[m_nodes[idx].left].weight : 0;
    int rw = (m_nodes[idx].right >= 0) ? m_nodes[m_nodes[idx].right].weight : 0;
    int lh = (m_nodes[idx].left >= 0) ? m_nodes[m_nodes[idx].left].height : 0;
    int rh = (m_nodes[idx].right >= 0) ? m_nodes[m_nodes[idx].right].height : 0;
    m_nodes[idx].weight = lw + rw + 1;
    m_nodes[idx].height = qMax(lh, rh) + 1;
}

/* ---- Check alpha-balance ---- */

bool WeightBalancedTree7::isBalanced(int idx) const
{
    if (idx < 0) return true;
    int lw = (m_nodes[idx].left >= 0) ? m_nodes[m_nodes[idx].left].weight : 0;
    int rw = (m_nodes[idx].right >= 0) ? m_nodes[m_nodes[idx].right].weight : 0;
    int total = lw + rw + 1;
    if (total <= 2) return true;
    double ratio = m_alpha * static_cast<double>(total);
    return (lw <= ratio + 1 && rw <= ratio + 1);
}

/* ---- Left rotation ---- */

int WeightBalancedTree7::rotateLeft(int idx)
{
    int r = m_nodes[idx].right;
    if (r < 0) return idx;

    m_nodes[idx].right = m_nodes[r].left;
    if (m_nodes[r].left >= 0)
        m_nodes[m_nodes[r].left].parent = idx;

    m_nodes[r].parent = m_nodes[idx].parent;
    m_nodes[idx].parent = r;
    m_nodes[r].left = idx;

    updateAggregate(idx);
    updateAggregate(r);
    m_stats.numRotations++;
    return r;
}

/* ---- Right rotation ---- */

int WeightBalancedTree7::rotateRight(int idx)
{
    int l = m_nodes[idx].left;
    if (l < 0) return idx;

    m_nodes[idx].left = m_nodes[l].right;
    if (m_nodes[l].right >= 0)
        m_nodes[m_nodes[l].right].parent = idx;

    m_nodes[l].parent = m_nodes[idx].parent;
    m_nodes[idx].parent = l;
    m_nodes[l].right = idx;

    updateAggregate(idx);
    updateAggregate(l);
    m_stats.numRotations++;
    return l;
}

/* ---- Collect inorder ---- */

void WeightBalancedTree7::collectInorder(int idx, QVector<int>& keys,
                                           QVector<double>& vals) const
{
    if (idx < 0) return;
    collectInorder(m_nodes[idx].left, keys, vals);
    keys.append(m_nodes[idx].key);
    vals.append(m_nodes[idx].value);
    collectInorder(m_nodes[idx].right, keys, vals);
}

/* ---- Build balanced from sorted ---- */

int WeightBalancedTree7::buildBalanced(const QVector<int>& keys,
                                         const QVector<double>& vals,
                                         int lo, int hi)
{
    if (lo > hi) return -1;
    int mid = (lo + hi) / 2;
    int idx = allocateNode(keys[mid], vals[mid]);
    m_nodes[idx].left = buildBalanced(keys, vals, lo, mid - 1);
    m_nodes[idx].right = buildBalanced(keys, vals, mid + 1, hi);
    if (m_nodes[idx].left >= 0) m_nodes[m_nodes[idx].left].parent = idx;
    if (m_nodes[idx].right >= 0) m_nodes[m_nodes[idx].right].parent = idx;
    updateAggregate(idx);
    return idx;
}

/* ---- Rebuild subtree ---- */

int WeightBalancedTree7::rebuildSubtree(int idx)
{
    QVector<int> keys;
    QVector<double> vals;
    collectInorder(idx, keys, vals);
    int count = keys.size();

    // Free old nodes
    for (int i = 0; i < count; ++i) {
        // Re-allocate during buildBalanced will reuse free list
    }

    return buildBalanced(keys, vals, 0, count - 1);
}

/* ---- Rebalance ---- */

int WeightBalancedTree7::rebalance(int idx)
{
    if (idx < 0) return idx;

    // Update aggregates
    updateAggregate(idx);

    if (!isBalanced(idx)) {
        // Decide rotation direction based on weight imbalance
        int lw = (m_nodes[idx].left >= 0) ? m_nodes[m_nodes[idx].left].weight : 0;
        int rw = (m_nodes[idx].right >= 0) ? m_nodes[m_nodes[idx].right].weight : 0;

        if (lw > rw) {
            // Right-heavy on left: check if double rotation needed
            int llw = (m_nodes[idx].left >= 0 && m_nodes[m_nodes[idx].left].left >= 0)
                          ? m_nodes[m_nodes[m_nodes[idx].left].left].weight : 0;
            int lrw = (m_nodes[idx].left >= 0 && m_nodes[m_nodes[idx].left].right >= 0)
                          ? m_nodes[m_nodes[m_nodes[idx].left].right].weight : 0;
            if (lrw > llw)
                m_nodes[idx].left = rotateLeft(m_nodes[idx].left);
            idx = rotateRight(idx);
        } else {
            int rlw = (m_nodes[idx].right >= 0 && m_nodes[m_nodes[idx].right].left >= 0)
                          ? m_nodes[m_nodes[m_nodes[idx].right].left].weight : 0;
            int rrw = (m_nodes[idx].right >= 0 && m_nodes[m_nodes[idx].right].right >= 0)
                          ? m_nodes[m_nodes[m_nodes[idx].right].right].weight : 0;
            if (rlw > rrw)
                m_nodes[idx].right = rotateRight(m_nodes[idx].right);
            idx = rotateLeft(idx);
        }
    }

    m_nodes[idx].needsRebalance = false;
    return idx;
}

/* ---- Insert ---- */

bool WeightBalancedTree7::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root < 0) {
        m_root = allocateNode(key, value);
        m_stats.numNodes = 1;
        m_stats.numInsertions++;
        m_stats.totalOps++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
        emit treeUpdated(1, 1, timer.elapsed());
        return true;
    }

    // Standard BST insert
    int cur = m_root;
    int parent = -1;
    while (cur >= 0) {
        parent = cur;
        if (key < m_nodes[cur].key)
            cur = m_nodes[cur].left;
        else if (key > m_nodes[cur].key)
            cur = m_nodes[cur].right;
        else {
            m_nodes[cur].value = value; // update existing
            return true;
        }
    }

    int newIdx = allocateNode(key, value);
    m_nodes[newIdx].parent = parent;
    if (key < m_nodes[parent].key)
        m_nodes[parent].left = newIdx;
    else
        m_nodes[parent].right = newIdx;

    m_stats.numNodes++;
    m_stats.numInsertions++;

    // Walk up updating aggregates and marking for relaxed rebalance
    int walk = parent;
    int batch = 0;
    while (walk >= 0 && batch < m_maxRebalanceBatch) {
        updateAggregate(walk);
        if (!isBalanced(walk)) {
            m_nodes[walk].needsRebalance = true;
            // Perform local rebalance
            int par = m_nodes[walk].parent;
            int newWalk = rebalance(walk);
            if (par >= 0) {
                if (m_nodes[par].left == walk)
                    m_nodes[par].left = newWalk;
                else
                    m_nodes[par].right = newWalk;
                m_nodes[newWalk].parent = par;
            } else {
                m_root = newWalk;
            }
            walk = par;
            batch++;
        } else {
            walk = m_nodes[walk].parent;
        }
    }

    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_stats.numNodes, m_stats.treeHeight, timer.elapsed());
    return true;
}

/* ---- Remove ---- */

bool WeightBalancedTree7::remove(int key)
{
    // Find node
    int cur = m_root;
    while (cur >= 0) {
        if (key < m_nodes[cur].key) cur = m_nodes[cur].left;
        else if (key > m_nodes[cur].key) cur = m_nodes[cur].right;
        else break;
    }
    if (cur < 0) return false;

    int parent = m_nodes[cur].parent;

    // BST deletion
    if (m_nodes[cur].left < 0 && m_nodes[cur].right < 0) {
        if (parent >= 0) {
            if (m_nodes[parent].left == cur) m_nodes[parent].left = -1;
            else m_nodes[parent].right = -1;
        } else m_root = -1;
        freeNode(cur);
    } else if (m_nodes[cur].left < 0 || m_nodes[cur].right < 0) {
        int child = (m_nodes[cur].left >= 0) ? m_nodes[cur].left : m_nodes[cur].right;
        if (parent >= 0) {
            if (m_nodes[parent].left == cur) m_nodes[parent].left = child;
            else m_nodes[parent].right = child;
        } else m_root = child;
        m_nodes[child].parent = parent;
        freeNode(cur);
    } else {
        // Find inorder successor
        int succ = m_nodes[cur].right;
        while (m_nodes[succ].left >= 0) succ = m_nodes[succ].left;
        m_nodes[cur].key = m_nodes[succ].key;
        m_nodes[cur].value = m_nodes[succ].value;
        // Delete successor
        int sp = m_nodes[succ].parent;
        if (sp >= 0) {
            if (m_nodes[sp].left == succ) m_nodes[sp].left = m_nodes[succ].right;
            else m_nodes[sp].right = m_nodes[succ].right;
        }
        if (m_nodes[succ].right >= 0)
            m_nodes[m_nodes[succ].right].parent = sp;
        freeNode(succ);
        parent = sp;
    }

    m_stats.numNodes--;
    m_stats.numDeletions++;

    // Rebalance ancestors
    int walk = parent;
    while (walk >= 0) {
        updateAggregate(walk);
        if (!isBalanced(walk)) {
            int par = m_nodes[walk].parent;
            int newWalk = rebalance(walk);
            if (par >= 0) {
                if (m_nodes[par].left == walk) m_nodes[par].left = newWalk;
                else m_nodes[par].right = newWalk;
                m_nodes[newWalk].parent = par;
            } else m_root = newWalk;
            walk = par;
        } else walk = m_nodes[walk].parent;
    }

    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return true;
}

/* ---- Search ---- */

double WeightBalancedTree7::search(int key) const
{
    int cur = m_root;
    while (cur >= 0) {
        if (key < m_nodes[cur].key) cur = m_nodes[cur].left;
        else if (key > m_nodes[cur].key) cur = m_nodes[cur].right;
        else return m_nodes[cur].value;
    }
    return std::numeric_limits<double>::quiet_NaN();
}

/* ---- Height ---- */

int WeightBalancedTree7::height() const
{
    if (m_root < 0) return 0;
    return m_nodes[m_root].height;
}

/* ---- Inorder keys ---- */

QVector<int> WeightBalancedTree7::inorderKeys() const
{
    QVector<int> keys;
    QVector<double> vals;
    collectInorder(m_root, keys, vals);
    return keys;
}

/* ---- Rebalance all flagged nodes ---- */

void WeightBalancedTree7::rebalanceAll()
{
    // Full rebuild for guaranteed balance
    if (m_root >= 0) {
        QVector<int> keys;
        QVector<double> vals;
        collectInorder(m_root, keys, vals);
        int oldRoot = m_root;
        m_root = buildBalanced(keys, vals, 0, keys.size() - 1);
        m_nodes[oldRoot].needsRebalance = false;
    }
    m_stats.treeHeight = height();
}

/* ---- Reset ---- */

void WeightBalancedTree7::resetStatistics()
{
    m_nodes.clear();
    m_root = -1;
    m_freeList = -1;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

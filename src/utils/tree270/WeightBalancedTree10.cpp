/**
 * @file WeightBalancedTree10.cpp
 * @brief WeightBalancedTree10 实现
 *
 * 实现权重平衡树：秩平衡旋转与Alpha权重约束确定性平衡BST。
 */

#include "utils/tree270/WeightBalancedTree10.h"

#include <QElapsedTimer>
#include <QtGlobal>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WeightBalancedTree10::WeightBalancedTree10(QObject *parent)
    : QObject(parent) {}

WeightBalancedTree10::~WeightBalancedTree10() = default;

/* ---- Configuration ---- */

void WeightBalancedTree10::setAlpha(double alpha)
{
    m_alpha = qBound(0.25, alpha, 0.49);
}

/* ---- Node allocation ---- */

int WeightBalancedTree10::allocNode(double key, int value)
{
    int idx;
    if (!m_freeList.isEmpty()) {
        idx = m_freeList.takeLast();
        m_nodes[idx] = {key, value, 1, -1, -1, -1};
    } else {
        idx = m_nodes.size();
        m_nodes.append({key, value, 1, -1, -1, -1});
    }
    return idx;
}

void WeightBalancedTree10::freeNode(int idx)
{
    m_freeList.append(idx);
}

/* ---- Weight maintenance ---- */

void WeightBalancedTree10::updateWeight(int idx)
{
    if (idx < 0) return;
    int w = 1;
    if (m_nodes[idx].left >= 0)
        w += m_nodes[m_nodes[idx].left].weight;
    if (m_nodes[idx].right >= 0)
        w += m_nodes[m_nodes[idx].right].weight;
    m_nodes[idx].weight = w;
}

/* ---- Rotations ---- */

int WeightBalancedTree10::rotateLeft(int idx)
{
    int r = m_nodes[idx].right;
    if (r < 0) return idx;

    m_nodes[idx].right = m_nodes[r].left;
    if (m_nodes[r].left >= 0)
        m_nodes[m_nodes[r].left].parent = idx;

    m_nodes[r].left = idx;
    m_nodes[r].parent = m_nodes[idx].parent;
    m_nodes[idx].parent = r;

    updateWeight(idx);
    updateWeight(r);
    m_rotations++;
    return r;
}

int WeightBalancedTree10::rotateRight(int idx)
{
    int l = m_nodes[idx].left;
    if (l < 0) return idx;

    m_nodes[idx].left = m_nodes[l].right;
    if (m_nodes[l].right >= 0)
        m_nodes[m_nodes[l].right].parent = idx;

    m_nodes[l].right = idx;
    m_nodes[l].parent = m_nodes[idx].parent;
    m_nodes[idx].parent = l;

    updateWeight(idx);
    updateWeight(l);
    m_rotations++;
    return l;
}

/* ---- Rebalance with alpha-weight constraint ---- */

int WeightBalancedTree10::rebalance(int idx)
{
    if (idx < 0) return idx;

    updateWeight(idx);
    int wl = (m_nodes[idx].left >= 0) ? m_nodes[m_nodes[idx].left].weight : 0;
    int wr = (m_nodes[idx].right >= 0) ? m_nodes[m_nodes[idx].right].weight : 0;
    int w = m_nodes[idx].weight;

    // Alpha-weight balance condition: wl >= alpha * w && wr >= alpha * w
    bool leftHeavy = (w > 1) && (static_cast<double>(wl) > (1.0 - m_alpha) * w);
    bool rightHeavy = (w > 1) && (static_cast<double>(wr) > (1.0 - m_alpha) * w);

    if (leftHeavy) {
        // Left-left or left-right
        int ll = m_nodes[idx].left;
        int wll = (m_nodes[ll].left >= 0) ? m_nodes[m_nodes[ll].left].weight : 0;
        int wlr = (m_nodes[ll].right >= 0) ? m_nodes[m_nodes[ll].right].weight : 0;

        if (wlr > wll) {
            m_nodes[idx].left = rotateLeft(ll);
            if (m_nodes[idx].left >= 0)
                m_nodes[m_nodes[idx].left].parent = idx;
        }
        idx = rotateRight(idx);
    } else if (rightHeavy) {
        // Right-right or right-left
        int rr = m_nodes[idx].right;
        int wrl = (m_nodes[rr].left >= 0) ? m_nodes[m_nodes[rr].left].weight : 0;
        int wrr = (m_nodes[rr].right >= 0) ? m_nodes[m_nodes[rr].right].weight : 0;

        if (wrl > wrr) {
            m_nodes[idx].right = rotateRight(rr);
            if (m_nodes[idx].right >= 0)
                m_nodes[m_nodes[idx].right].parent = idx;
        }
        idx = rotateLeft(idx);
    }

    return idx;
}

/* ---- Recursive insert ---- */

int WeightBalancedTree10::insertRec(int idx, double key, int value)
{
    if (idx < 0) return allocNode(key, value);

    if (key < m_nodes[idx].key) {
        int newLeft = insertRec(m_nodes[idx].left, key, value);
        m_nodes[idx].left = newLeft;
        m_nodes[newLeft].parent = idx;
    } else if (key > m_nodes[idx].key) {
        int newRight = insertRec(m_nodes[idx].right, key, value);
        m_nodes[idx].right = newRight;
        m_nodes[newRight].parent = idx;
    } else {
        // Duplicate: update value
        m_nodes[idx].value = value;
        return idx;
    }

    return rebalance(idx);
}

/* ---- Recursive remove ---- */

int WeightBalancedTree10::removeRec(int idx, double key)
{
    if (idx < 0) return -1;

    if (key < m_nodes[idx].key) {
        m_nodes[idx].left = removeRec(m_nodes[idx].left, key);
        if (m_nodes[idx].left >= 0)
            m_nodes[m_nodes[idx].left].parent = idx;
    } else if (key > m_nodes[idx].key) {
        m_nodes[idx].right = removeRec(m_nodes[idx].right, key);
        if (m_nodes[idx].right >= 0)
            m_nodes[m_nodes[idx].right].parent = idx;
    } else {
        // Found node to remove
        if (m_nodes[idx].left < 0) {
            int r = m_nodes[idx].right;
            freeNode(idx);
            return r;
        }
        if (m_nodes[idx].right < 0) {
            int l = m_nodes[idx].left;
            freeNode(idx);
            return l;
        }
        // Two children: replace with in-order successor
        int succ = m_nodes[idx].right;
        while (m_nodes[succ].left >= 0)
            succ = m_nodes[succ].left;
        m_nodes[idx].key = m_nodes[succ].key;
        m_nodes[idx].value = m_nodes[succ].value;
        m_nodes[idx].right = removeRec(m_nodes[idx].right, m_nodes[succ].key);
        if (m_nodes[idx].right >= 0)
            m_nodes[m_nodes[idx].right].parent = idx;
    }

    return rebalance(idx);
}

/* ---- In-order traversal ---- */

void WeightBalancedTree10::inOrderHelper(int idx, QVector<double>& result) const
{
    if (idx < 0) return;
    inOrderHelper(m_nodes[idx].left, result);
    result.append(m_nodes[idx].key);
    inOrderHelper(m_nodes[idx].right, result);
}

/* ---- Height ---- */

int WeightBalancedTree10::heightHelper(int idx) const
{
    if (idx < 0) return 0;
    int lh = heightHelper(m_nodes[idx].left);
    int rh = heightHelper(m_nodes[idx].right);
    return 1 + qMax(lh, rh);
}

/* ---- Public API ---- */

void WeightBalancedTree10::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();
    m_rotations = 0;

    m_root = insertRec(m_root, key, value);

    double elapsed = timer.elapsed();
    m_stats.treeSize = size();
    m_stats.treeHeight = height();
    m_stats.numRotations = m_rotations;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_stats.treeSize, m_stats.treeHeight, m_rotations, elapsed);
}

bool WeightBalancedTree10::remove(double key)
{
    QElapsedTimer timer;
    timer.start();
    m_rotations = 0;

    int oldSize = size();
    m_root = removeRec(m_root, key);
    int newSize = size();

    double elapsed = timer.elapsed();
    m_stats.treeSize = newSize;
    m_stats.treeHeight = height();
    m_stats.numRotations = m_rotations;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_stats.treeSize, m_stats.treeHeight, m_rotations, elapsed);

    return newSize < oldSize;
}

int WeightBalancedTree10::search(double key) const
{
    int idx = m_root;
    while (idx >= 0) {
        if (key < m_nodes[idx].key)
            idx = m_nodes[idx].left;
        else if (key > m_nodes[idx].key)
            idx = m_nodes[idx].right;
        else
            return m_nodes[idx].value;
    }
    return -1;
}

QVector<double> WeightBalancedTree10::inOrderKeys() const
{
    QVector<double> result;
    inOrderHelper(m_root, result);
    return result;
}

int WeightBalancedTree10::height() const { return heightHelper(m_root); }
int WeightBalancedTree10::size() const { return (m_root >= 0) ? m_nodes[m_root].weight : 0; }

/* ---- Reset ---- */

void WeightBalancedTree10::resetStatistics()
{
    m_root = -1;
    m_nodes.clear();
    m_freeList.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file AvlTree5.cpp
 * @brief AvlTree5 实现
 *
 * 实现增强AVL树：子树最小/最大键维护、范围搜索查询、最近邻查找。
 */

#include "utils/tree206/AvlTree5.h"

#include <QElapsedTimer>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

AvlTree5::AvlTree5(QObject *parent) : QObject(parent) {}
AvlTree5::~AvlTree5() = default;

/* ---- Node height ---- */

int AvlTree5::nodeHeight(int idx) const
{
    return (idx >= 0 && idx < m_nodes.size()) ? m_nodes[idx].height : 0;
}

/* ---- Balance factor ---- */

int AvlTree5::balanceFactor(int idx) const
{
    if (idx < 0 || idx >= m_nodes.size()) return 0;
    return nodeHeight(m_nodes[idx].left) - nodeHeight(m_nodes[idx].right);
}

/* ---- Update node height and augmented data ---- */

void AvlTree5::updateNode(int idx)
{
    if (idx < 0 || idx >= m_nodes.size()) return;
    Node& n = m_nodes[idx];
    n.height = 1 + qMax(nodeHeight(n.left), nodeHeight(n.right));
    n.subtreeMin = n.key;
    n.subtreeMax = n.key;
    if (n.left >= 0) {
        n.subtreeMin = qMin(n.subtreeMin, m_nodes[n.left].subtreeMin);
        n.subtreeMax = qMax(n.subtreeMax, m_nodes[n.left].subtreeMax);
    }
    if (n.right >= 0) {
        n.subtreeMin = qMin(n.subtreeMin, m_nodes[n.right].subtreeMin);
        n.subtreeMax = qMax(n.subtreeMax, m_nodes[n.right].subtreeMax);
    }
}

/* ---- Rotations ---- */

int AvlTree5::rotateRight(int y)
{
    int x = m_nodes[y].left;
    m_nodes[y].left = m_nodes[x].right;
    m_nodes[x].right = y;
    updateNode(y);
    updateNode(x);
    return x;
}

int AvlTree5::rotateLeft(int x)
{
    int y = m_nodes[x].right;
    m_nodes[x].right = m_nodes[y].left;
    m_nodes[y].left = x;
    updateNode(x);
    updateNode(y);
    return y;
}

/* ---- Recursive insert ---- */

int AvlTree5::insertRec(int root, int key, double value)
{
    if (root < 0) {
        Node n;
        n.key = key;
        n.value = value;
        n.height = 1;
        n.left = -1;
        n.right = -1;
        n.subtreeMin = key;
        n.subtreeMax = key;
        int idx = m_nodes.size();
        m_nodes.append(n);
        return idx;
    }

    if (key < m_nodes[root].key)
        m_nodes[root].left = insertRec(m_nodes[root].left, key, value);
    else if (key > m_nodes[root].key)
        m_nodes[root].right = insertRec(m_nodes[root].right, key, value);
    else {
        m_nodes[root].value = value;  // Update existing
        return root;
    }

    updateNode(root);

    // Rebalance
    int bf = balanceFactor(root);
    if (bf > 1) {
        if (key < m_nodes[m_nodes[root].left].key)
            return rotateRight(root);
        else {
            m_nodes[root].left = rotateLeft(m_nodes[root].left);
            return rotateRight(root);
        }
    }
    if (bf < -1) {
        if (key > m_nodes[m_nodes[root].right].key)
            return rotateLeft(root);
        else {
            m_nodes[root].right = rotateRight(m_nodes[root].right);
            return rotateLeft(root);
        }
    }
    return root;
}

/* ---- Find min ---- */

int AvlTree5::findMin(int root) const
{
    while (root >= 0 && m_nodes[root].left >= 0)
        root = m_nodes[root].left;
    return root;
}

/* ---- Recursive remove ---- */

int AvlTree5::removeRec(int root, int key)
{
    if (root < 0) return -1;

    if (key < m_nodes[root].key)
        m_nodes[root].left = removeRec(m_nodes[root].left, key);
    else if (key > m_nodes[root].key)
        m_nodes[root].right = removeRec(m_nodes[root].right, key);
    else {
        // Node to delete
        if (m_nodes[root].left < 0) return m_nodes[root].right;
        if (m_nodes[root].right < 0) return m_nodes[root].left;

        int succ = findMin(m_nodes[root].right);
        m_nodes[root].key = m_nodes[succ].key;
        m_nodes[root].value = m_nodes[succ].value;
        m_nodes[root].right = removeRec(m_nodes[root].right, m_nodes[succ].key);
    }

    updateNode(root);

    int bf = balanceFactor(root);
    if (bf > 1) {
        if (balanceFactor(m_nodes[root].left) >= 0)
            return rotateRight(root);
        else {
            m_nodes[root].left = rotateLeft(m_nodes[root].left);
            return rotateRight(root);
        }
    }
    if (bf < -1) {
        if (balanceFactor(m_nodes[root].right) <= 0)
            return rotateLeft(root);
        else {
            m_nodes[root].right = rotateRight(m_nodes[root].right);
            return rotateLeft(root);
        }
    }
    return root;
}

/* ---- Insert ---- */

void AvlTree5::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();
    m_root = insertRec(m_root, key, value);
    m_stats.totalOps++;
    m_stats.treeSize++;
    m_stats.treeHeight = height();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("insert", key, timer.elapsed());
}

/* ---- Remove ---- */

void AvlTree5::remove(int key)
{
    QElapsedTimer timer;
    timer.start();
    auto [found, _] = lookup(key);
    m_root = removeRec(m_root, key);
    if (found) m_stats.treeSize--;
    m_stats.totalOps++;
    m_stats.treeHeight = height();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("remove", key, timer.elapsed());
}

/* ---- Lookup ---- */

QPair<bool, double> AvlTree5::lookup(int key) const
{
    int cur = m_root;
    while (cur >= 0 && cur < m_nodes.size()) {
        if (key < m_nodes[cur].key) cur = m_nodes[cur].left;
        else if (key > m_nodes[cur].key) cur = m_nodes[cur].right;
        else return {true, m_nodes[cur].value};
    }
    return {false, 0.0};
}

/* ---- Range search ---- */

void AvlTree5::rangeSearchRec(int root, int lo, int hi,
                                QVector<QPair<int, double>>& result) const
{
    if (root < 0 || root >= m_nodes.size()) return;
    // Prune: if subtree max < lo or subtree min > hi, skip
    if (m_nodes[root].subtreeMax < lo || m_nodes[root].subtreeMin > hi) return;

    rangeSearchRec(m_nodes[root].left, lo, hi, result);

    if (m_nodes[root].key >= lo && m_nodes[root].key <= hi)
        result.append({m_nodes[root].key, m_nodes[root].value});

    rangeSearchRec(m_nodes[root].right, lo, hi, result);
}

QVector<QPair<int, double>> AvlTree5::rangeSearch(int lo, int hi) const
{
    QVector<QPair<int, double>> result;
    rangeSearchRec(m_root, lo, hi, result);
    return result;
}

/* ---- Nearest neighbor ---- */

void AvlTree5::nearestRec(int root, int target, int& bestKey,
                            double& bestDist, double& bestVal) const
{
    if (root < 0 || root >= m_nodes.size()) return;

    double dist = qFabs(m_nodes[root].key - target);
    if (dist < bestDist) {
        bestDist = dist;
        bestKey = m_nodes[root].key;
        bestVal = m_nodes[root].value;
    }

    // Prune using augmented subtree bounds
    if (target < m_nodes[root].key) {
        nearestRec(m_nodes[root].left, target, bestKey, bestDist, bestVal);
        // Only go right if subtreeMin is closer than bestDist
        if (m_nodes[root].right >= 0) {
            double rightDist = qFabs(m_nodes[m_nodes[root].right].subtreeMin - target);
            if (rightDist < bestDist)
                nearestRec(m_nodes[root].right, target, bestKey, bestDist, bestVal);
        }
    } else {
        nearestRec(m_nodes[root].right, target, bestKey, bestDist, bestVal);
        if (m_nodes[root].left >= 0) {
            double leftDist = qFabs(m_nodes[m_nodes[root].left].subtreeMax - target);
            if (leftDist < bestDist)
                nearestRec(m_nodes[root].left, target, bestKey, bestDist, bestVal);
        }
    }
}

QPair<int, double> AvlTree5::nearestNeighbor(int target) const
{
    int bestKey = 0;
    double bestDist = std::numeric_limits<double>::max();
    double bestVal = 0.0;
    nearestRec(m_root, target, bestKey, bestDist, bestVal);
    return {bestKey, bestVal};
}

/* ---- Height ---- */

int AvlTree5::height() const { return nodeHeight(m_root); }

/* ---- Empty ---- */

bool AvlTree5::isEmpty() const { return m_root < 0; }

/* ---- Clear ---- */

void AvlTree5::clear()
{
    m_nodes.clear();
    m_root = -1;
    m_stats.treeSize = 0;
    m_stats.treeHeight = 0;
}

/* ---- Reset ---- */

void AvlTree5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    clear();
}

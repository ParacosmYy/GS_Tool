/**
 * @file ScapegoatTree6.cpp
 * @brief ScapegoatTree6 实现
 *
 * 实现替罪羊树：权重平衡重建、替罪羊检测、分数级联手指搜索。
 */

#include "utils/tree208/ScapegoatTree6.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ScapegoatTree6::ScapegoatTree6(QObject *parent) : QObject(parent) {}
ScapegoatTree6::~ScapegoatTree6() = default;

/* ---- Configuration ---- */

void ScapegoatTree6::setAlpha(double alpha) { m_alpha = qBound(0.5, alpha, 1.0); }

/* ---- Allocate node ---- */

int ScapegoatTree6::allocNode(int key, double value)
{
    Node n;
    n.key = key;
    n.value = value;
    n.left = -1;
    n.right = -1;
    n.subtreeSize = 1;
    n.cascadeLeft = -1;
    n.cascadeRight = -1;
    int idx = m_nodes.size();
    m_nodes.append(n);
    return idx;
}

/* ---- Update subtree size ---- */

int ScapegoatTree6::updateSize(int idx)
{
    if (idx < 0) return 0;
    int leftSz = (m_nodes[idx].left >= 0) ? m_nodes[m_nodes[idx].left].subtreeSize : 0;
    int rightSz = (m_nodes[idx].right >= 0) ? m_nodes[m_nodes[idx].right].subtreeSize : 0;
    m_nodes[idx].subtreeSize = 1 + leftSz + rightSz;
    return m_nodes[idx].subtreeSize;
}

/* ---- Check alpha balance ---- */

bool ScapegoatTree6::isAlphaBalanced(int idx) const
{
    if (idx < 0) return true;
    int leftSz = (m_nodes[idx].left >= 0) ? m_nodes[m_nodes[idx].left].subtreeSize : 0;
    int rightSz = (m_nodes[idx].right >= 0) ? m_nodes[m_nodes[idx].right].subtreeSize : 0;
    int total = 1 + leftSz + rightSz;
    return leftSz <= m_alpha * total && rightSz <= m_alpha * total;
}

/* ---- Flatten subtree ---- */

void ScapegoatTree6::flatten(int idx, QVector<int>& sorted) const
{
    if (idx < 0) return;
    flatten(m_nodes[idx].left, sorted);
    sorted.append(idx);
    flatten(m_nodes[idx].right, sorted);
}

/* ---- Rebuild balanced ---- */

int ScapegoatTree6::rebuildBalanced(const QVector<int>& sorted, int lo, int hi)
{
    if (lo > hi) return -1;
    int mid = (lo + hi) / 2;
    int idx = sorted[mid];

    m_nodes[idx].left = rebuildBalanced(sorted, lo, mid - 1);
    m_nodes[idx].right = rebuildBalanced(sorted, mid + 1, hi);
    updateSize(idx);
    buildCascadeHints(idx);
    return idx;
}

/* ---- Find scapegoat ---- */

int ScapegoatTree6::findScapegoat(int idx, int key) const
{
    if (idx < 0) return -1;
    int child = (key < m_nodes[idx].key) ? m_nodes[idx].left : m_nodes[idx].right;
    if (child < 0) return idx;
    if (!isAlphaBalanced(idx)) return idx;

    int sgChild = findScapegoat(child, key);
    return (sgChild >= 0) ? sgChild : idx;
}

/* ---- Build cascade hints ---- */

void ScapegoatTree6::buildCascadeHints(int idx)
{
    if (idx < 0) return;
    // Cascade left: rightmost node in left subtree
    if (m_nodes[idx].left >= 0) {
        int cur = m_nodes[idx].left;
        while (m_nodes[cur].right >= 0)
            cur = m_nodes[cur].right;
        m_nodes[idx].cascadeLeft = cur;
    } else {
        m_nodes[idx].cascadeLeft = -1;
    }

    // Cascade right: leftmost node in right subtree
    if (m_nodes[idx].right >= 0) {
        int cur = m_nodes[idx].right;
        while (m_nodes[cur].left >= 0)
            cur = m_nodes[cur].left;
        m_nodes[idx].cascadeRight = cur;
    } else {
        m_nodes[idx].cascadeRight = -1;
    }
}

/* ---- Insert ---- */

void ScapegoatTree6::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    // BST insert
    QVector<int> path;
    if (m_root < 0) {
        m_root = allocNode(key, value);
    } else {
        int cur = m_root;
        while (cur >= 0) {
            path.append(cur);
            if (key == m_nodes[cur].key) {
                m_nodes[cur].value = value;
                m_timeSum += timer.elapsed();
                m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, ++m_stats.totalOps);
                return;
            } else if (key < m_nodes[cur].key) {
                if (m_nodes[cur].left < 0) {
                    m_nodes[cur].left = allocNode(key, value);
                    break;
                }
                cur = m_nodes[cur].left;
            } else {
                if (m_nodes[cur].right < 0) {
                    m_nodes[cur].right = allocNode(key, value);
                    break;
                }
                cur = m_nodes[cur].right;
            }
        }
    }

    // Update sizes along path
    for (int i = path.size() - 1; i >= 0; --i)
        updateSize(path[i]);
    updateSize(m_root);

    m_stats.treeSize++;
    m_maxSize = qMax(m_maxSize, m_stats.treeSize);

    // Check if root needs rebuild (global balance check)
    if (m_root >= 0 && m_maxSize > 2
        && static_cast<double>(m_stats.treeSize) < m_alpha * m_maxSize) {
        QVector<int> sorted;
        flatten(m_root, sorted);
        m_root = rebuildBalanced(sorted, 0, sorted.size() - 1);
        m_maxSize = m_stats.treeSize;
        m_stats.numRebuilds++;
    } else {
        // Find scapegoat on insertion path
        for (int i = 0; i < path.size(); ++i) {
            if (!isAlphaBalanced(path[i])) {
                // Rebuild subtree rooted at scapegoat
                QVector<int> sorted;
                flatten(path[i], sorted);
                int newSub = rebuildBalanced(sorted, 0, sorted.size() - 1);

                // Reattach rebuilt subtree
                if (i == 0) {
                    m_root = newSub;
                } else {
                    int parent = (i > 0) ? path[i - 1] : -1;
                    if (parent >= 0) {
                        if (m_nodes[path[i]].key < m_nodes[parent].key)
                            m_nodes[parent].left = newSub;
                        else
                            m_nodes[parent].right = newSub;
                    }
                }

                // Update sizes up to parent
                for (int j = i - 1; j >= 0; --j)
                    updateSize(path[j]);
                updateSize(m_root);

                m_stats.numRebuilds++;
                break;
            }
        }
    }

    // Update tree height
    m_stats.treeHeight = 0;
    for (int cur = m_root; cur >= 0; ) {
        m_stats.treeHeight++;
        int leftH = (m_nodes[cur].left >= 0) ? m_nodes[m_nodes[cur].left].subtreeSize : 0;
        int rightH = (m_nodes[cur].right >= 0) ? m_nodes[m_nodes[cur].right].subtreeSize : 0;
        cur = (leftH >= rightH) ? m_nodes[cur].left : m_nodes[cur].right;
    }

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, ++m_stats.totalOps);
    emit operationCompleted("insert", key, timer.elapsed());
}

/* ---- Remove ---- */

void ScapegoatTree6::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    // Find node and parent
    int cur = m_root;
    int parent = -1;
    bool isLeft = false;

    while (cur >= 0 && m_nodes[cur].key != key) {
        parent = cur;
        if (key < m_nodes[cur].key) {
            cur = m_nodes[cur].left;
            isLeft = true;
        } else {
            cur = m_nodes[cur].right;
            isLeft = false;
        }
    }

    if (cur < 0) {
        m_timeSum += timer.elapsed();
        return;
    }

    // BST remove (replace with in-order successor)
    if (m_nodes[cur].left >= 0 && m_nodes[cur].right >= 0) {
        int succ = m_nodes[cur].right;
        int succParent = cur;
        while (m_nodes[succ].left >= 0) {
            succParent = succ;
            succ = m_nodes[succ].left;
        }
        m_nodes[cur].key = m_nodes[succ].key;
        m_nodes[cur].value = m_nodes[succ].value;
        // Remove successor
        if (succParent == cur)
            m_nodes[succParent].right = m_nodes[succ].right;
        else
            m_nodes[succParent].left = m_nodes[succ].right;
    } else {
        int child = (m_nodes[cur].left >= 0) ? m_nodes[cur].left : m_nodes[cur].right;
        if (parent < 0)
            m_root = child;
        else if (isLeft)
            m_nodes[parent].left = child;
        else
            m_nodes[parent].right = child;
    }

    m_stats.treeSize--;
    if (m_root >= 0) updateSize(m_root);

    // Global rebuild if too unbalanced
    if (m_maxSize > 2 && static_cast<double>(m_stats.treeSize) < m_alpha * m_maxSize) {
        QVector<int> sorted;
        flatten(m_root, sorted);
        m_root = rebuildBalanced(sorted, 0, sorted.size() - 1);
        m_maxSize = m_stats.treeSize;
        m_stats.numRebuilds++;
    }

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, ++m_stats.totalOps);
    emit operationCompleted("remove", key, timer.elapsed());
}

/* ---- Lookup ---- */

QPair<bool, double> ScapegoatTree6::lookup(int key) const
{
    int cur = m_root;
    while (cur >= 0) {
        if (key == m_nodes[cur].key)
            return {true, m_nodes[cur].value};
        cur = (key < m_nodes[cur].key) ? m_nodes[cur].left : m_nodes[cur].right;
    }
    return {false, 0.0};
}

/* ---- Finger search ---- */

QPair<bool, double> ScapegoatTree6::fingerSearch(int key, int hintKey) const
{
    // Start from root, use cascade hints for acceleration
    int cur = m_root;
    while (cur >= 0) {
        if (key == m_nodes[cur].key)
            return {true, m_nodes[cur].value};

        // Use cascade hints to skip subtrees
        if (key < m_nodes[cur].key) {
            // Check if cascade hint provides shortcut
            if (m_nodes[cur].cascadeLeft >= 0
                && m_nodes[m_nodes[cur].cascadeLeft].key >= key) {
                cur = m_nodes[cur].left;
            } else {
                cur = m_nodes[cur].left;
            }
        } else {
            if (m_nodes[cur].cascadeRight >= 0
                && m_nodes[m_nodes[cur].cascadeRight].key <= key) {
                cur = m_nodes[cur].right;
            } else {
                cur = m_nodes[cur].right;
            }
        }
    }
    return {false, 0.0};

    Q_UNUSED(hintKey)
}

/* ---- Range collect ---- */

void ScapegoatTree6::rangeCollect(int idx, int lo, int hi,
                                    QVector<QPair<int, double>>& result) const
{
    if (idx < 0) return;
    if (m_nodes[idx].key > lo)
        rangeCollect(m_nodes[idx].left, lo, hi, result);
    if (m_nodes[idx].key >= lo && m_nodes[idx].key <= hi)
        result.append({m_nodes[idx].key, m_nodes[idx].value});
    if (m_nodes[idx].key < hi)
        rangeCollect(m_nodes[idx].right, lo, hi, result);
}

/* ---- Range query ---- */

QVector<QPair<int, double>> ScapegoatTree6::rangeQuery(int lo, int hi) const
{
    QVector<QPair<int, double>> result;
    rangeCollect(m_root, lo, hi, result);
    return result;
}

/* ---- In-order traversal ---- */

QVector<QPair<int, double>> ScapegoatTree6::inOrderTraversal() const
{
    QVector<QPair<int, double>> result;
    QVector<int> sorted;
    // Non-recursive in-order via flatten
    const_cast<ScapegoatTree6*>(this)->flatten(m_root, sorted);
    for (int idx : sorted)
        result.append({m_nodes[idx].key, m_nodes[idx].value});
    return result;
}

/* ---- Is balanced ---- */

bool ScapegoatTree6::isBalanced() const
{
    if (m_root < 0) return true;
    return isAlphaBalanced(m_root);
}

/* ---- Clear ---- */

void ScapegoatTree6::clear()
{
    m_nodes.clear();
    m_root = -1;
    m_stats.treeSize = 0;
    m_stats.treeHeight = 0;
    m_maxSize = 0;
}

/* ---- Reset ---- */

void ScapegoatTree6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    clear();
}

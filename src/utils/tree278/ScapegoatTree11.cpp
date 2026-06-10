/**
 * @file ScapegoatTree11.cpp
 * @brief ScapegoatTree11 实现
 *
 * 实现替罪羊树：对数加权重建阈值与仅插入摊还保证的写优化二叉搜索树。
 */

#include "utils/tree278/ScapegoatTree11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ScapegoatTree11::ScapegoatTree11(QObject *parent)
    : QObject(parent) {}

ScapegoatTree11::~ScapegoatTree11() = default;

/* ---- Configuration ---- */

void ScapegoatTree11::setAlpha(double alpha) { m_alpha = qBound(0.5, alpha, 0.99); }

/* ---- Node allocation ---- */

int ScapegoatTree11::allocNode(int key, double value)
{
    // Reuse inactive (deleted) slots
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (!m_nodes[i].active) {
            m_nodes[i] = {key, value, -1, -1, 1, true};
            return i;
        }
    }
    int idx = m_nodes.size();
    m_nodes.append({key, value, -1, -1, 1, true});
    m_stats.numNodes++;
    return idx;
}

/* ---- Subtree size ---- */

int ScapegoatTree11::subtreeSize(int nodeIdx) const
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return 0;
    const Node& n = m_nodes[nodeIdx];
    if (!n.active) return 0;
    int s = 1;
    if (n.left >= 0) s += subtreeSize(n.left);
    if (n.right >= 0) s += subtreeSize(n.right);
    return s;
}

/* ---- Compute height ---- */

int ScapegoatTree11::computeHeight(int nodeIdx) const
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size() || !m_nodes[nodeIdx].active) return 0;
    int lh = computeHeight(m_nodes[nodeIdx].left);
    int rh = computeHeight(m_nodes[nodeIdx].right);
    return 1 + qMax(lh, rh);
}

/* ---- Check alpha-weight-balanced ---- */

bool ScapegoatTree11::isBalanced(int nodeIdx) const
{
    if (nodeIdx < 0 || !m_nodes[nodeIdx].active) return true;
    const Node& n = m_nodes[nodeIdx];
    int ls = subtreeSize(n.left);
    int rs = subtreeSize(n.right);
    int total = ls + rs + 1;

    double logAlpha = qLn(1.0 / m_alpha) / M_LN2;
    // Log-weighted threshold: max child size <= alpha * total
    // And height <= log_{1/alpha}(total) + 1
    int maxChild = qMax(ls, rs);
    if (maxChild > m_alpha * total) return false;

    int h = computeHeight(nodeIdx);
    if (h > qCeil(qLn(total + 1) / qLn(1.0 / m_alpha)) + 1) return false;
    return true;
}

/* ---- Find scapegoat ---- */

int ScapegoatTree11::findScapegoat(int nodeIdx, const QVector<int>& path) const
{
    // Walk up from deepest node, find first unbalanced ancestor
    for (int i = path.size() - 1; i >= 0; --i) {
        if (!isBalanced(path[i])) return path[i];
    }
    return -1;
}

/* ---- Flatten subtree ---- */

void ScapegoatTree11::flatten(int nodeIdx, QVector<QPair<int, double>>& entries) const
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return;
    const Node& n = m_nodes[nodeIdx];
    if (!n.active) return;
    flatten(n.left, entries);
    entries.append({n.key, n.value});
    flatten(n.right, entries);
}

/* ---- Rebuild balanced BST from sorted entries ---- */

int ScapegoatTree11::rebuildBalanced(const QVector<QPair<int, double>>& entries, int lo, int hi)
{
    if (lo > hi) return -1;
    int mid = (lo + hi) / 2;
    int idx = allocNode(entries[mid].first, entries[mid].second);

    m_nodes[idx].left = rebuildBalanced(entries, lo, mid - 1);
    m_nodes[idx].right = rebuildBalanced(entries, mid + 1, hi);
    m_nodes[idx].size = hi - lo + 1;
    m_nodes[idx].active = true;

    return idx;
}

/* ---- Update sizes along path ---- */

void ScapegoatTree11::updateSizes(const QVector<int>& path)
{
    for (int i = path.size() - 1; i >= 0; --i) {
        int idx = path[i];
        if (idx < 0 || idx >= m_nodes.size()) continue;
        Node& n = m_nodes[idx];
        n.size = 1;
        if (n.left >= 0 && m_nodes[n.left].active) n.size += m_nodes[n.left].size;
        if (n.right >= 0 && m_nodes[n.right].active) n.size += m_nodes[n.right].size;
    }
}

/* ---- Recursive insert ---- */

int ScapegoatTree11::insertRec(int nodeIdx, int key, double value, QVector<int>& path)
{
    if (nodeIdx < 0 || !m_nodes[nodeIdx].active) {
        int newIdx = allocNode(key, value);
        return newIdx;
    }

    path.append(nodeIdx);
    Node& n = m_nodes[nodeIdx];

    if (key == n.key) {
        n.value = value; // Update existing
        return nodeIdx;
    } else if (key < n.key) {
        n.left = insertRec(n.left, key, value, path);
    } else {
        n.right = insertRec(n.right, key, value, path);
    }

    // Update size
    n.size = 1;
    if (n.left >= 0 && m_nodes[n.left].active) n.size += m_nodes[n.left].size;
    if (n.right >= 0 && m_nodes[n.right].active) n.size += m_nodes[n.right].size;

    return nodeIdx;
}

/* ---- Insert ---- */

void ScapegoatTree11::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> path;
    m_root = insertRec(m_root, key, value, path);
    m_stats.numKeys++;

    // Check if tree needs rebuilding
    // Log-weighted: check if height exceeds log_{1/alpha}(n) + 1
    int h = computeHeight(m_root);
    int n = subtreeSize(m_root);
    double logThresh = qCeil(qLn(n + 1) / qLn(1.0 / m_alpha)) + 1;

    if (h > logThresh) {
        // Find scapegoat
        int goat = findScapegoat(m_root, path);
        if (goat >= 0) {
            // Rebuild subtree at goat
            QVector<QPair<int, double>> entries;
            flatten(goat, entries);

            // Find parent of goat in path
            int parentIdx = -1;
            bool isLeft = false;
            for (int i = 0; i < path.size() - 1; ++i) {
                if (path[i + 1] == goat) {
                    parentIdx = path[i];
                    isLeft = (m_nodes[parentIdx].left == goat);
                    break;
                }
            }

            // Deactivate old nodes in subtree
            QVector<int> toDeactivate;
            toDeactivate.append(goat);
            for (int i = 0; i < toDeactivate.size(); ++i) {
                int idx = toDeactivate[i];
                if (idx < 0 || idx >= m_nodes.size()) continue;
                m_nodes[idx].active = false;
                m_stats.numNodes--;
                if (m_nodes[idx].left >= 0) toDeactivate.append(m_nodes[idx].left);
                if (m_nodes[idx].right >= 0) toDeactivate.append(m_nodes[idx].right);
            }

            int newSub = rebuildBalanced(entries, 0, entries.size() - 1);

            if (parentIdx >= 0) {
                if (isLeft) m_nodes[parentIdx].left = newSub;
                else m_nodes[parentIdx].right = newSub;
            } else {
                m_root = newSub;
            }

            m_stats.numRebuilds++;
            m_stats.treeHeight = computeHeight(m_root);
            emit rebuildTriggered(m_nodes[goat < m_nodes.size() ? goat : 0].key,
                                  entries.size(), m_stats.treeHeight, timer.elapsed());
        }
    }

    m_maxSize = qMax(m_maxSize, m_stats.numKeys);
    m_stats.treeHeight = computeHeight(m_root);

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    m_stats.alpha = m_alpha;
    emit operationDone(QStringLiteral("insert"), m_stats.numKeys, m_stats.treeHeight, elapsed);
}

/* ---- Find node ---- */

int ScapegoatTree11::findNode(int key) const
{
    int cur = m_root;
    while (cur >= 0 && cur < m_nodes.size() && m_nodes[cur].active) {
        if (key == m_nodes[cur].key) return cur;
        cur = (key < m_nodes[cur].key) ? m_nodes[cur].left : m_nodes[cur].right;
    }
    return -1;
}

/* ---- Find ---- */

double ScapegoatTree11::find(int key) const
{
    int idx = findNode(key);
    return (idx >= 0) ? m_nodes[idx].value : qQNaN();
}

/* ---- Remove (soft-delete) ---- */

void ScapegoatTree11::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    int idx = findNode(key);
    if (idx >= 0) {
        m_nodes[idx].active = false;
        m_stats.numKeys--;
        m_stats.numNodes--;
    }

    // Check if global rebuild needed after deletion
    // Rebuild if numKeys < alpha * maxSize
    if (m_maxSize > 0 && m_stats.numKeys < m_alpha * m_maxSize) {
        QVector<QPair<int, double>> entries;
        flatten(m_root, entries);

        // Deactivate all
        for (auto& n : m_nodes) n.active = false;
        m_stats.numNodes = 0;

        m_root = rebuildBalanced(entries, 0, entries.size() - 1);
        m_stats.numRebuilds++;
        m_maxSize = m_stats.numKeys;
    }

    m_stats.treeHeight = computeHeight(m_root);
    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationDone(QStringLiteral("remove"), m_stats.numKeys, m_stats.treeHeight, elapsed);
}

/* ---- Contains ---- */

bool ScapegoatTree11::contains(int key) const { return findNode(key) >= 0; }

/* ---- In-order traversal ---- */

void ScapegoatTree11::inOrder(int nodeIdx, QVector<int>& keys) const
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size() || !m_nodes[nodeIdx].active) return;
    inOrder(m_nodes[nodeIdx].left, keys);
    keys.append(m_nodes[nodeIdx].key);
    inOrder(m_nodes[nodeIdx].right, keys);
}

/* ---- All keys ---- */

QVector<int> ScapegoatTree11::allKeys() const
{
    QVector<int> keys;
    inOrder(m_root, keys);
    return keys;
}

/* ---- Height ---- */

int ScapegoatTree11::height() const { return computeHeight(m_root); }

/* ---- Size ---- */

int ScapegoatTree11::size() const { return m_stats.numKeys; }

/* ---- Clear ---- */

void ScapegoatTree11::clear()
{
    m_nodes.clear();
    m_root = -1;
    m_maxSize = 0;
    m_stats.numKeys = 0;
    m_stats.numNodes = 0;
    m_stats.treeHeight = 0;
}

/* ---- Reset ---- */

void ScapegoatTree11::resetStatistics()
{
    clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

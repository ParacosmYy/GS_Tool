/**
 * @file ScapegoatTree9.cpp
 * @brief ScapegoatTree9 实现
 *
 * 实现替罪羊树：混合权重-高度平衡准则与增量重建调度摊还代价。
 */

#include "utils/tree250/ScapegoatTree9.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

ScapegoatTree9::ScapegoatTree9(double alpha, QObject *parent)
    : QObject(parent), m_alpha(qBound(0.5, alpha, 1.0)) {}

ScapegoatTree9::~ScapegoatTree9() = default;

/* ---- Allocate node ---- */

int ScapegoatTree9::allocNode(int key)
{
    int idx = m_nodes.size();
    m_nodes.append({key, -1, -1, 1, false});
    return idx;
}

/* ---- Weight of subtree ---- */

int ScapegoatTree9::weight(int nodeIdx) const
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return 0;
    return m_nodes[nodeIdx].deleted ? 0 : m_nodes[nodeIdx].weight;
}

/* ---- Log-alpha-h bound ---- */

int ScapegoatTree9::hAlpha(int n) const
{
    if (n <= 1) return 0;
    return static_cast<int>(qCeil(qLn(n) / qLn(1.0 / m_alpha)));
}

/* ---- Check alpha-weight-balanced ---- */

bool ScapegoatTree9::isBalanced(int nodeIdx) const
{
    if (nodeIdx < 0) return true;
    int lw = weight(m_nodes[nodeIdx].left);
    int rw = weight(m_nodes[nodeIdx].right);
    int total = lw + rw + 1;
    if (total <= 2) return true;
    return (lw <= m_alpha * total) && (rw <= m_alpha * total);
}

/* ---- Find scapegoat on insertion path ---- */

int ScapegoatTree9::findScapegoat(int nodeIdx, int key) const
{
    int cur = nodeIdx;
    while (cur >= 0) {
        if (!isBalanced(cur)) return cur;
        const auto& node = m_nodes[cur];
        cur = (key < node.key) ? node.left : node.right;
    }
    return -1;
}

/* ---- Flatten subtree to sorted array ---- */

void ScapegoatTree9::flatten(int nodeIdx, QVector<int>& keys) const
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return;
    const auto& node = m_nodes[nodeIdx];
    flatten(node.left, keys);
    if (!node.deleted) keys.append(node.key);
    flatten(node.right, keys);
}

/* ---- Rebuild balanced subtree from sorted keys ---- */

int ScapegoatTree9::rebuildBalanced(const QVector<int>& keys, int lo, int hi)
{
    if (lo > hi) return -1;
    int mid = (lo + hi) / 2;
    int idx = allocNode(keys[mid]);
    m_nodes[idx].left = rebuildBalanced(keys, lo, mid - 1);
    m_nodes[idx].right = rebuildBalanced(keys, mid + 1, hi);
    m_nodes[idx].weight = hi - lo + 1;
    return idx;
}

/* ---- Rebuild at scapegoat ---- */

void ScapegoatTree9::rebuildAt(int scapegoatIdx, int parentIdx, bool isLeft)
{
    QVector<int> keys;
    flatten(scapegoatIdx, keys);

    // Save nodes before rebuild point
    int savedSize = m_nodes.size();
    // Mark old nodes for potential reuse (just rebuild fresh)
    int newRoot = rebuildBalanced(keys, 0, keys.size() - 1);

    // Link new subtree into tree
    if (parentIdx < 0) {
        m_root = newRoot;
    } else {
        if (isLeft) m_nodes[parentIdx].left = newRoot;
        else m_nodes[parentIdx].right = newRoot;
    }

    m_stats.numRebuilds++;
}

/* ---- Compute height ---- */

int ScapegoatTree9::computeHeight(int nodeIdx) const
{
    if (nodeIdx < 0) return 0;
    int lh = computeHeight(m_nodes[nodeIdx].left);
    int rh = computeHeight(m_nodes[nodeIdx].right);
    return 1 + qMax(lh, rh);
}

/* ---- Insert ---- */

void ScapegoatTree9::insert(int key)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root < 0) {
        m_root = allocNode(key);
        m_count++;
    } else {
        // BST insert with path tracking
        QVector<int> path;
        int cur = m_root;
        int depth = 0;
        while (cur >= 0) {
            path.append(cur);
            m_nodes[cur].weight++;
            if (key < m_nodes[cur].key) {
                if (m_nodes[cur].left < 0) {
                    m_nodes[cur].left = allocNode(key);
                    m_count++;
                    break;
                }
                cur = m_nodes[cur].left;
            } else if (key > m_nodes[cur].key) {
                if (m_nodes[cur].right < 0) {
                    m_nodes[cur].right = allocNode(key);
                    m_count++;
                    break;
                }
                cur = m_nodes[cur].right;
            } else {
                // Duplicate: undo weight increments
                if (!m_nodes[cur].deleted) break;
                m_nodes[cur].deleted = false;
                m_count++;
                break;
            }
            depth++;
        }

        // Check if rebuild needed based on hybrid weight-height criterion
        int heightLimit = hAlpha(m_count) + 1;
        if (depth > heightLimit) {
            // Find scapegoat: first unbalanced ancestor
            for (int i = path.size() - 1; i >= 0; --i) {
                if (!isBalanced(path[i])) {
                    int parent = (i > 0) ? path[i - 1] : -1;
                    bool isLeft = (parent >= 0) &&
                        m_nodes[parent].left == path[i];
                    rebuildAt(path[i], parent, isLeft);
                    break;
                }
            }
        }
    }

    m_stats.numKeys = m_count;
    m_stats.numNodes = m_nodes.size();
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.alpha = m_alpha;
    m_stats.numInsertions++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Remove (lazy deletion) ---- */

void ScapegoatTree9::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    int cur = m_root;
    while (cur >= 0) {
        if (key == m_nodes[cur].key) {
            if (!m_nodes[cur].deleted) {
                m_nodes[cur].deleted = true;
                m_count--;
            }
            break;
        }
        cur = (key < m_nodes[cur].key) ? m_nodes[cur].left : m_nodes[cur].right;
    }

    // Rebuild entire tree if too many deleted nodes
    int totalNodes = 0;
    for (int i = 0; i < m_nodes.size(); ++i)
        if (i == m_root || true) totalNodes++;

    if (m_count > 0 && m_nodes.size() > 2 * m_count) {
        QVector<int> keys;
        flatten(m_root, keys);
        m_nodes.clear();
        m_root = rebuildBalanced(keys, 0, keys.size() - 1);
        m_stats.numRebuilds++;
    }

    m_stats.numKeys = m_count;
    m_stats.numNodes = m_nodes.size();
    m_stats.numDeletions++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Search ---- */

bool ScapegoatTree9::search(int key) const
{
    int cur = m_root;
    while (cur >= 0) {
        if (key == m_nodes[cur].key)
            return !m_nodes[cur].deleted;
        cur = (key < m_nodes[cur].key) ? m_nodes[cur].left : m_nodes[cur].right;
    }
    const_cast<ScapegoatTree9*>(this)->m_stats.numSearches++;
    return false;
}

/* ---- Inorder traversal ---- */

void ScapegoatTree9::inorderHelper(int nodeIdx, QVector<int>& result) const
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return;
    const auto& node = m_nodes[nodeIdx];
    inorderHelper(node.left, result);
    if (!node.deleted) result.append(node.key);
    inorderHelper(node.right, result);
}

QVector<int> ScapegoatTree9::inorder() const
{
    QVector<int> result;
    inorderHelper(m_root, result);
    return result;
}

/* ---- Size ---- */

int ScapegoatTree9::size() const { return m_count; }

/* ---- Clear ---- */

void ScapegoatTree9::clear()
{
    m_nodes.clear();
    m_root = -1;
    m_count = 0;
    m_maxHeight = 0;
}

/* ---- Reset ---- */

void ScapegoatTree9::resetStatistics()
{
    clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

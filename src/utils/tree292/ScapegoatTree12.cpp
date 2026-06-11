/**
 * @file ScapegoatTree12.cpp
 * @brief ScapegoatTree12 实现
 *
 * 实现替罪羊树：摊还权重平衡重建与α比率触发子树重构实现自调整二叉搜索树。
 */

#include "utils/tree292/ScapegoatTree12.h"

#include <QElapsedTimer>
#include <QStringBuilder>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ScapegoatTree12::ScapegoatTree12(QObject *parent)
    : QObject(parent) {}

ScapegoatTree12::~ScapegoatTree12() = default;

/* ---- Configuration ---- */

void ScapegoatTree12::setAlpha(double alpha) { m_alpha = qBound(0.51, alpha, 0.99); }
void ScapegoatTree12::setMaxSize(int maxSz) { m_maxSize = qBound(100, maxSz, 10000000); }

/* ---- Node allocation ---- */

int ScapegoatTree12::allocNode(int key)
{
    int idx;
    if (!m_freeList.isEmpty()) {
        idx = m_freeList.takeLast();
        m_nodes[idx] = Node{};
        m_nodes[idx].key = key;
    } else {
        idx = m_nodes.size();
        m_nodes.append(Node{});
        m_nodes[idx].key = key;
    }
    return idx;
}

void ScapegoatTree12::freeNode(int idx)
{
    if (idx >= 0 && idx < m_nodes.size()) {
        m_nodes[idx].deleted = true;
        m_freeList.append(idx);
    }
}

/* ---- Compute subtree size ---- */

int ScapegoatTree12::computeSize(int idx) const
{
    if (idx == NULL_IDX) return 0;
    int sz = 1;
    sz += computeSize(m_nodes[idx].left);
    sz += computeSize(m_nodes[idx].right);
    return sz;
}

/* ---- Update sizes along path ---- */

void ScapegoatTree12::updateSizes(int idx)
{
    if (idx == NULL_IDX) return;
    updateSizes(m_nodes[idx].left);
    updateSizes(m_nodes[idx].right);
    m_nodes[idx].subSize = 1
        + ((m_nodes[idx].left != NULL_IDX) ? m_nodes[m_nodes[idx].left].subSize : 0)
        + ((m_nodes[idx].right != NULL_IDX) ? m_nodes[m_nodes[idx].right].subSize : 0);
}

/* ---- Check α-weight-balanced ---- */

bool ScapegoatTree12::isAlphaBalanced(int idx) const
{
    if (idx == NULL_IDX) return true;
    int leftSz = (m_nodes[idx].left != NULL_IDX) ? m_nodes[m_nodes[idx].left].subSize : 0;
    int rightSz = (m_nodes[idx].right != NULL_IDX) ? m_nodes[m_nodes[idx].right].subSize : 0;
    int total = leftSz + rightSz + 1;
    if (total <= 2) return true;
    return (leftSz <= m_alpha * total) && (rightSz <= m_alpha * total);
}

/* ---- Find scapegoat node ---- */

int ScapegoatTree12::findScapegoat(int idx, QVector<int>& path)
{
    // Walk up from inserted node to find first unbalanced ancestor
    for (int i = path.size() - 1; i >= 0; --i) {
        int node = path[i];
        updateSizes(node);
        if (!isAlphaBalanced(node))
            return node;
    }
    return NULL_IDX;
}

/* ---- Flatten subtree into sorted array ---- */

void ScapegoatTree12::flatten(int idx, QVector<int>& keys)
{
    if (idx == NULL_IDX) return;
    flatten(m_nodes[idx].left, keys);
    if (!m_nodes[idx].deleted)
        keys.append(m_nodes[idx].key);
    flatten(m_nodes[idx].right, keys);
}

/* ---- Build balanced BST from sorted keys ---- */

int ScapegoatTree12::buildBalanced(const QVector<int>& keys, int lo, int hi)
{
    if (lo > hi) return NULL_IDX;
    int mid = (lo + hi) / 2;
    int idx = allocNode(keys[mid]);
    m_nodes[idx].left = buildBalanced(keys, lo, mid - 1);
    m_nodes[idx].right = buildBalanced(keys, mid + 1, hi);
    m_nodes[idx].subSize = hi - lo + 1;
    return idx;
}

/* ---- Rebuild subtree ---- */

int ScapegoatTree12::rebuildSubtree(int idx)
{
    if (idx == NULL_IDX) return NULL_IDX;

    QVector<int> keys;
    flatten(idx, keys);
    if (keys.isEmpty()) return NULL_IDX;

    // Collect nodes to free (old subtree)
    QVector<int> oldNodes;
    flatten(idx, oldNodes); // reuse for node indices (not ideal but works)
    // Actually we need to free old nodes - simplified: just rebuild
    int newSize = keys.size();

    // Reset the node pool entries for this subtree
    // Simple approach: mark all nodes in subtree as free, then rebuild
    QVector<int> subtreeNodes;
    // Collect node indices via inorder
    QStack<int> stack;
    int cur = idx;
    while (cur != NULL_IDX || !stack.isEmpty()) {
        while (cur != NULL_IDX) {
            stack.push(cur);
            cur = m_nodes[cur].left;
        }
        cur = stack.pop();
        subtreeNodes.append(cur);
        cur = m_nodes[cur].right;
    }
    for (int ni : subtreeNodes)
        m_freeList.append(ni);

    // Build new balanced subtree
    int newRoot = buildBalanced(keys, 0, keys.size() - 1);
    return newRoot;
}

/* ---- Insert ---- */

void ScapegoatTree12::insert(int key)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root == NULL_IDX) {
        m_root = allocNode(key);
        m_size++;
        m_maxTreeSize = 1;
        m_stats.numKeys = m_size;
        m_stats.numNodes = m_nodes.size();
        return;
    }

    // Find insertion point and record path
    QVector<int> path;
    int cur = m_root;
    int parent = NULL_IDX;
    bool goLeft = false;

    while (cur != NULL_IDX) {
        path.append(cur);
        parent = cur;
        if (key < m_nodes[cur].key) {
            goLeft = true;
            cur = m_nodes[cur].left;
        } else if (key > m_nodes[cur].key) {
            goLeft = false;
            cur = m_nodes[cur].right;
        } else {
            // Duplicate: resurrect if deleted
            if (m_nodes[cur].deleted) {
                m_nodes[cur].deleted = false;
                m_size++;
            }
            return;
        }
    }

    // Insert new node
    int newIdx = allocNode(key);
    if (parent != NULL_IDX) {
        if (goLeft) m_nodes[parent].left = newIdx;
        else m_nodes[parent].right = newIdx;
    }
    m_size++;
    m_maxTreeSize = qMax(m_maxTreeSize, m_size);

    // Update sizes along path
    for (int i = 0; i < path.size(); ++i)
        m_nodes[path[i]].subSize++;

    // Find scapegoat and rebuild if needed
    int scapegoat = findScapegoat(newIdx, path);
    if (scapegoat != NULL_IDX) {
        QElapsedTimer rebuildTimer;
        rebuildTimer.start();

        int newSub = rebuildSubtree(scapegoat);
        double rebuildTime = rebuildTimer.elapsed();

        // Reconnect rebuilt subtree
        if (scapegoat == m_root) {
            m_root = newSub;
        } else {
            // Find parent of scapegoat
            for (int i = 0; i < path.size() - 1; ++i) {
                if (m_nodes[path[i]].left == scapegoat)
                    m_nodes[path[i]].left = newSub;
                else if (m_nodes[path[i]].right == scapegoat)
                    m_nodes[path[i]].right = newSub;
            }
        }
        m_stats.numRebuilds++;
        emit rebuildTriggered(computeSize(newSub), rebuildTime);
    }

    double elapsed = timer.elapsed();
    m_stats.numKeys = m_size;
    m_stats.numNodes = m_nodes.size() - m_freeList.size();
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationDone(QStringLiteral("insert"), key, m_stats.treeHeight, elapsed);
}

/* ---- Remove (lazy deletion with global rebuild) ---- */

void ScapegoatTree12::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    int idx = findNode(key);
    if (idx == NULL_IDX || m_nodes[idx].deleted) return;

    // Lazy deletion
    m_nodes[idx].deleted = true;
    m_size--;

    // Trigger global rebuild if too many deleted nodes
    if (m_size < m_alpha * m_maxTreeSize) {
        QElapsedTimer rebuildTimer;
        rebuildTimer.start();

        QVector<int> keys;
        flatten(m_root, keys);
        m_nodes.clear();
        m_freeList.clear();
        m_maxTreeSize = keys.size();
        if (!keys.isEmpty())
            m_root = buildBalanced(keys, 0, keys.size() - 1);
        else
            m_root = NULL_IDX;

        m_stats.numRebuilds++;
        emit rebuildTriggered(keys.size(), rebuildTimer.elapsed());
    }

    double elapsed = timer.elapsed();
    m_stats.numKeys = m_size;
    m_stats.numNodes = m_nodes.size() - m_freeList.size();
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationDone(QStringLiteral("remove"), key, m_stats.treeHeight, elapsed);
}

/* ---- Search ---- */

bool ScapegoatTree12::search(int key) const
{
    int idx = findNode(key);
    return (idx != NULL_IDX && !m_nodes[idx].deleted);
}

/* ---- Find node ---- */

int ScapegoatTree12::findNode(int key) const
{
    int cur = m_root;
    while (cur != NULL_IDX) {
        if (key < m_nodes[cur].key) cur = m_nodes[cur].left;
        else if (key > m_nodes[cur].key) cur = m_nodes[cur].right;
        else return cur;
    }
    return NULL_IDX;
}

/* ---- Inorder traversal ---- */

void ScapegoatTree12::inorderCollect(int idx, QVector<int>& result) const
{
    if (idx == NULL_IDX) return;
    inorderCollect(m_nodes[idx].left, result);
    if (!m_nodes[idx].deleted)
        result.append(m_nodes[idx].key);
    inorderCollect(m_nodes[idx].right, result);
}

QVector<int> ScapegoatTree12::inorderTraversal() const
{
    QVector<int> result;
    inorderCollect(m_root, result);
    return result;
}

/* ---- Compute height ---- */

int ScapegoatTree12::computeHeight(int idx) const
{
    if (idx == NULL_IDX) return 0;
    int lh = computeHeight(m_nodes[idx].left);
    int rh = computeHeight(m_nodes[idx].right);
    return 1 + qMax(lh, rh);
}

/* ---- Check balanced ---- */

bool ScapegoatTree12::isBalanced() const
{
    if (m_root == NULL_IDX) return true;
    int h = computeHeight(m_root);
    // Scapegoat tree height should be ≤ log_{1/α}(n)
    double maxH = qLn(qMax(m_size, 1)) / qLn(1.0 / m_alpha);
    return h <= maxH + 1;
}

/* ---- Reset ---- */

void ScapegoatTree12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_freeList.clear();
    m_root = NULL_IDX;
    m_size = 0;
    m_maxTreeSize = 0;
}

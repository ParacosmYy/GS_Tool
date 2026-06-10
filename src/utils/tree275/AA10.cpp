/**
 * @file AA10.cpp
 * @brief AA10 实现
 *
 * 实现AA树：递归skew/split重平衡与基于层级删除简化平衡树维护。
 */

#include "utils/tree275/AA10.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

AA10::AA10(QObject *parent)
    : QObject(parent) {}

AA10::~AA10() = default;

/* ---- Node allocation ---- */

int AA10::allocNode(int key)
{
    int idx = m_nodes.size();
    m_nodes.append(Node{key, 1, -1, -1});
    return idx;
}

/* ---- Skew: right-rotate to eliminate left horizontal link ---- */

int AA10::skew(int nodeIdx)
{
    if (nodeIdx < 0) return nodeIdx;
    Node& T = m_nodes[nodeIdx];
    if (T.left < 0) return nodeIdx;

    // If left child has same level as T => horizontal left link
    if (m_nodes[T.left].level == T.level) {
        int L = T.left;
        T.left = m_nodes[L].right;
        m_nodes[L].right = nodeIdx;
        return L;  // L becomes new root of this subtree
    }
    return nodeIdx;
}

/* ---- Split: left-rotate to eliminate consecutive right horizontal links ---- */

int AA10::split(int nodeIdx)
{
    if (nodeIdx < 0) return nodeIdx;
    Node& T = m_nodes[nodeIdx];
    if (T.right < 0 || m_nodes[T.right].right < 0) return nodeIdx;

    // If right-right grandchild has same level => two right horizontals
    if (m_nodes[m_nodes[T.right].right].level == T.level) {
        int R = T.right;
        T.right = m_nodes[R].left;
        m_nodes[R].left = nodeIdx;
        m_nodes[R].level++;  // Promote R's level
        return R;
    }
    return nodeIdx;
}

/* ---- Recursive insert ---- */

int AA10::insertRec(int nodeIdx, int key)
{
    // Base case: create new leaf
    if (nodeIdx < 0) return allocNode(key);

    Node& node = m_nodes[nodeIdx];

    if (key < node.key) {
        node.left = insertRec(node.left, key);
    } else if (key > node.key) {
        node.right = insertRec(node.right, key);
    }
    // Duplicate key: no insert (AA tree property)

    // Rebalance: skew then split
    nodeIdx = skew(nodeIdx);
    nodeIdx = split(nodeIdx);
    return nodeIdx;
}

/* ---- Find minimum in subtree ---- */

int AA10::findMin(int nodeIdx) const
{
    while (nodeIdx >= 0 && m_nodes[nodeIdx].left >= 0)
        nodeIdx = m_nodes[nodeIdx].left;
    return nodeIdx;
}

/* ---- Decrease level and rebalance after deletion ---- */

int AA10::decreaseLevel(int nodeIdx)
{
    if (nodeIdx < 0) return nodeIdx;
    Node& T = m_nodes[nodeIdx];

    // Compute ideal level from children
    int leftLevel = (T.left >= 0) ? m_nodes[T.left].level : 0;
    int rightLevel = (T.right >= 0) ? m_nodes[T.right].level : 0;
    int shouldBe = qMin(leftLevel, rightLevel) + 1;

    if (shouldBe < T.level) {
        T.level = shouldBe;
        if (T.right >= 0 && m_nodes[T.right].level > shouldBe)
            m_nodes[T.right].level = shouldBe;
    }

    // Rebalance: skew right child first, then current node
    if (T.right >= 0) {
        T.right = skew(T.right);
        if (m_nodes[T.right].right >= 0)
            m_nodes[T.right].right = skew(m_nodes[T.right].right);
    }
    nodeIdx = skew(nodeIdx);

    // Split
    if (T.right >= 0) T.right = split(T.right);
    nodeIdx = split(nodeIdx);

    return nodeIdx;
}

/* ---- Recursive remove ---- */

int AA10::removeRec(int nodeIdx, int key)
{
    if (nodeIdx < 0) return nodeIdx;

    Node& node = m_nodes[nodeIdx];

    if (key < node.key) {
        node.left = removeRec(node.left, key);
    } else if (key > node.key) {
        node.right = removeRec(node.right, key);
    } else {
        // Found the node to delete
        if (node.left < 0 && node.right < 0) {
            // Leaf node: mark as removed (we keep it in pool but return -1)
            return -1;
        }
        if (node.left < 0) {
            // Only right child: replace with successor
            int succ = findMin(node.right);
            node.key = m_nodes[succ].key;
            node.right = removeRec(node.right, m_nodes[succ].key);
        } else {
            // Has left child: replace with predecessor
            int pred = node.left;
            while (m_nodes[pred].right >= 0) pred = m_nodes[pred].right;
            node.key = m_nodes[pred].key;
            node.left = removeRec(node.left, m_nodes[pred].key);
        }
    }

    // Decrease level and rebalance
    return decreaseLevel(nodeIdx);
}

/* ---- In-order traversal ---- */

void AA10::inOrderRec(int nodeIdx, QVector<int>& result) const
{
    if (nodeIdx < 0) return;
    inOrderRec(m_nodes[nodeIdx].left, result);
    result.append(m_nodes[nodeIdx].key);
    inOrderRec(m_nodes[nodeIdx].right, result);
}

/* ---- Compute height ---- */

int AA10::computeHeight(int nodeIdx) const
{
    if (nodeIdx < 0) return 0;
    int lh = computeHeight(m_nodes[nodeIdx].left);
    int rh = computeHeight(m_nodes[nodeIdx].right);
    return 1 + qMax(lh, rh);
}

/* ---- Public: insert ---- */

void AA10::insert(int key)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertRec(m_root, key);

    double elapsed = timer.elapsed();
    m_stats.numNodes = size();
    m_stats.treeHeight = height();
    m_stats.numInsertions++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationDone(QStringLiteral("insert"), m_stats.numNodes, elapsed);
}

/* ---- Public: remove ---- */

void AA10::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    if (!contains(key)) return;
    m_root = removeRec(m_root, key);

    double elapsed = timer.elapsed();
    m_stats.numNodes = size();
    m_stats.treeHeight = height();
    m_stats.numDeletions++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationDone(QStringLiteral("remove"), m_stats.numNodes, elapsed);
}

/* ---- Public: search / contains ---- */

bool AA10::contains(int key) const { return search(key); }

bool AA10::search(int key) const
{
    int cur = m_root;
    while (cur >= 0) {
        if (key < m_nodes[cur].key) cur = m_nodes[cur].left;
        else if (key > m_nodes[cur].key) cur = m_nodes[cur].right;
        else return true;
    }
    return false;
}

/* ---- Public: in-order traversal ---- */

QVector<int> AA10::inOrder() const
{
    QVector<int> result;
    inOrderRec(m_root, result);
    return result;
}

/* ---- Public: height ---- */

int AA10::height() const { return computeHeight(m_root); }

/* ---- Public: size ---- */

int AA10::size() const
{
    // Count reachable nodes from root
    int count = 0;
    QVector<int> stack;
    if (m_root >= 0) stack.append(m_root);
    while (!stack.isEmpty()) {
        int idx = stack.takeLast();
        count++;
        if (m_nodes[idx].left >= 0) stack.append(m_nodes[idx].left);
        if (m_nodes[idx].right >= 0) stack.append(m_nodes[idx].right);
    }
    return count;
}

/* ---- Public: clear ---- */

void AA10::clear()
{
    m_nodes.clear();
    m_root = -1;
}

/* ---- Reset ---- */

void AA10::resetStatistics()
{
    clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

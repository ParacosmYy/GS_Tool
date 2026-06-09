/**
 * @file AA8.cpp
 * @brief AA8 实现
 *
 * 实现AA树：层级平衡与删除后自底向上skew/split修复。
 */

#include "utils/tree247/AA8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

AA8::AA8(QObject *parent) : QObject(parent) {}
AA8::~AA8() = default;

/* ---- Allocate node ---- */

int AA8::allocNode(int key)
{
    int idx = m_nodes.size();
    m_nodes.append({key, 1, -1, -1});
    return idx;
}

/* ---- Skew: right rotate to fix left horizontal link ---- */

int AA8::skew(int nodeIdx)
{
    if (nodeIdx < 0) return nodeIdx;
    auto& node = m_nodes[nodeIdx];
    if (node.left < 0) return nodeIdx;

    auto& left = m_nodes[node.left];
    if (left.level == node.level) {
        // Right rotation
        int l = node.left;
        node.left = left.right;
        left.right = nodeIdx;
        return l;
    }
    return nodeIdx;
}

/* ---- Split: left rotate to fix consecutive right horizontal links ---- */

int AA8::split(int nodeIdx)
{
    if (nodeIdx < 0) return nodeIdx;
    auto& node = m_nodes[nodeIdx];
    if (node.right < 0) return nodeIdx;

    auto& right = m_nodes[node.right];
    if (right.right >= 0 && m_nodes[right.right].level == node.level) {
        // Left rotation
        int r = node.right;
        node.right = right.left;
        right.left = nodeIdx;
        right.level++;
        return r;
    }
    return nodeIdx;
}

/* ---- Recursive insert ---- */

int AA8::insertRec(int nodeIdx, int key)
{
    if (nodeIdx < 0) {
        m_count++;
        return allocNode(key);
    }

    auto& node = m_nodes[nodeIdx];
    if (key < node.key)
        node.left = insertRec(node.left, key);
    else if (key > node.key)
        node.right = insertRec(node.right, key);
    else
        return nodeIdx;  // Duplicate, no insert

    // Bottom-up skew then split
    nodeIdx = skew(nodeIdx);
    nodeIdx = split(nodeIdx);
    return nodeIdx;
}

/* ---- Successor ---- */

int AA8::successor(int nodeIdx) const
{
    if (nodeIdx < 0 || m_nodes[nodeIdx].right < 0) return -1;
    int cur = m_nodes[nodeIdx].right;
    while (m_nodes[cur].left >= 0) cur = m_nodes[cur].left;
    return cur;
}

/* ---- Predecessor ---- */

int AA8::predecessor(int nodeIdx) const
{
    if (nodeIdx < 0 || m_nodes[nodeIdx].left < 0) return -1;
    int cur = m_nodes[nodeIdx].left;
    while (m_nodes[cur].right >= 0) cur = m_nodes[cur].right;
    return cur;
}

/* ---- Decrease level and rebalance ---- */

int AA8::decreaseLevel(int nodeIdx)
{
    if (nodeIdx < 0) return nodeIdx;
    auto& node = m_nodes[nodeIdx];

    int leftLevel = (node.left >= 0) ? m_nodes[node.left].level : 0;
    int rightLevel = (node.right >= 0) ? m_nodes[node.right].level : 0;
    int shouldBe = qMin(leftLevel, rightLevel) + 1;

    if (shouldBe < node.level) {
        node.level = shouldBe;
        if (node.right >= 0 && rightLevel > shouldBe)
            m_nodes[node.right].level = shouldBe;
    }

    // Rebalance with three skew + two split
    nodeIdx = skew(nodeIdx);
    if (nodeIdx >= 0) {
        m_nodes[nodeIdx].right = skew(m_nodes[nodeIdx].right);
        if (m_nodes[nodeIdx].right >= 0)
            m_nodes[m_nodes[nodeIdx].right].right =
                skew(m_nodes[m_nodes[nodeIdx].right].right);
        nodeIdx = split(nodeIdx);
        if (nodeIdx >= 0)
            m_nodes[nodeIdx].right = split(m_nodes[nodeIdx].right);
    }
    return nodeIdx;
}

/* ---- Recursive remove ---- */

int AA8::removeRec(int nodeIdx, int key)
{
    if (nodeIdx < 0) return -1;  // Key not found

    auto& node = m_nodes[nodeIdx];
    if (key < node.key) {
        node.left = removeRec(node.left, key);
    } else if (key > node.key) {
        node.right = removeRec(node.right, key);
    } else {
        // Found the key
        if (node.left < 0 && node.right < 0) {
            m_count--;
            return -1;  // Leaf: remove
        }
        if (node.left < 0) {
            // Replace with successor
            int s = successor(nodeIdx);
            node.key = m_nodes[s].key;
            node.right = removeRec(node.right, m_nodes[s].key);
        } else {
            // Replace with predecessor
            int p = predecessor(nodeIdx);
            node.key = m_nodes[p].key;
            node.left = removeRec(node.left, m_nodes[p].key);
        }
    }

    // Bottom-up rebalance after deletion
    return decreaseLevel(nodeIdx);
}

/* ---- Insert ---- */

void AA8::insert(int key)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertRec(m_root, key);

    m_stats.numNodes = m_count;
    m_stats.numInsertions++;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit treeModified("insert", key, m_count, timer.elapsed());
}

/* ---- Remove ---- */

void AA8::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    m_root = removeRec(m_root, key);

    m_stats.numNodes = m_count;
    m_stats.numDeletions++;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit treeModified("remove", key, m_count, timer.elapsed());
}

/* ---- Contains ---- */

bool AA8::contains(int key) const
{
    int cur = m_root;
    while (cur >= 0) {
        if (key < m_nodes[cur].key) cur = m_nodes[cur].left;
        else if (key > m_nodes[cur].key) cur = m_nodes[cur].right;
        else return true;
    }
    const_cast<AA8*>(this)->m_stats.numSearches++;
    return false;
}

/* ---- In-order traversal ---- */

void AA8::inorderRec(int nodeIdx, QVector<int>& result) const
{
    if (nodeIdx < 0) return;
    inorderRec(m_nodes[nodeIdx].left, result);
    result.append(m_nodes[nodeIdx].key);
    inorderRec(m_nodes[nodeIdx].right, result);
}

QVector<int> AA8::inorder() const
{
    QVector<int> result;
    inorderRec(m_root, result);
    return result;
}

/* ---- Compute height ---- */

int AA8::computeHeight(int nodeIdx) const
{
    if (nodeIdx < 0) return 0;
    int lh = computeHeight(m_nodes[nodeIdx].left);
    int rh = computeHeight(m_nodes[nodeIdx].right);
    return 1 + qMax(lh, rh);
}

/* ---- Accessors ---- */

int AA8::size() const { return m_count; }
bool AA8::isEmpty() const { return m_count == 0; }

/* ---- Clear ---- */

void AA8::clear()
{
    m_nodes.clear();
    m_root = -1;
    m_count = 0;
}

/* ---- Reset ---- */

void AA8::resetStatistics()
{
    clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}

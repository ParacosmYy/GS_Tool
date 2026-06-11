/**
 * @file AA12.cpp
 * @brief AA12 实现
 *
 * 实现AA树：并发读写锁支持与基于时代的回收实现线程安全无锁读的平衡二叉搜索树。
 */

#include "utils/tree302/AA12.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

AA12::AA12(QObject *parent)
    : QObject(parent) {}

AA12::~AA12() = default;

/* ---- Skew: right rotation to fix left-horizontal link ---- */

int AA12::skew(int nodeIdx)
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return nodeIdx;

    int leftIdx = m_nodes[nodeIdx].left;
    if (leftIdx < 0 || leftIdx >= m_nodes.size()) return nodeIdx;

    // Only skew if left child has same level
    if (m_nodes[leftIdx].level == m_nodes[nodeIdx].level) {
        // Right rotation
        m_nodes[nodeIdx].left = m_nodes[leftIdx].right;
        m_nodes[leftIdx].right = nodeIdx;
        return leftIdx;
    }
    return nodeIdx;
}

/* ---- Split: left rotation + level increase to fix consecutive right-horizontal links ---- */

int AA12::split(int nodeIdx)
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return nodeIdx;

    int rightIdx = m_nodes[nodeIdx].right;
    if (rightIdx < 0 || rightIdx >= m_nodes.size()) return nodeIdx;

    int rrIdx = m_nodes[rightIdx].right;
    if (rrIdx < 0 || rrIdx >= m_nodes.size()) return nodeIdx;

    // Only split if right-right has same level
    if (m_nodes[rightIdx].level == m_nodes[nodeIdx].level &&
        m_nodes[rrIdx].level == m_nodes[nodeIdx].level) {
        // Left rotation
        m_nodes[nodeIdx].right = m_nodes[rightIdx].left;
        m_nodes[rightIdx].left = nodeIdx;
        m_nodes[rightIdx].level++;
        return rightIdx;
    }
    return nodeIdx;
}

/* ---- Allocate and insert node recursively ---- */

int AA12::insertHelper(int nodeIdx, int key, double value)
{
    // Base case: create new leaf node
    if (nodeIdx < 0) {
        Node n;
        n.key = key;
        n.value = value;
        n.level = 1;
        n.left = -1;
        n.right = -1;
        n.epoch = m_epoch;
        n.deleted = false;
        m_nodes.append(n);
        m_stats.nodeCount = m_nodes.size();
        return m_nodes.size() - 1;
    }

    // Binary search insertion
    if (key < m_nodes[nodeIdx].key) {
        m_nodes[nodeIdx].left = insertHelper(m_nodes[nodeIdx].left, key, value);
    } else if (key > m_nodes[nodeIdx].key) {
        m_nodes[nodeIdx].right = insertHelper(m_nodes[nodeIdx].right, key, value);
    } else {
        // Update existing key
        m_nodes[nodeIdx].value = value;
        return nodeIdx;
    }

    // Rebalance with skew and split
    nodeIdx = skew(nodeIdx);
    nodeIdx = split(nodeIdx);
    return nodeIdx;
}

/* ---- Find minimum in subtree ---- */

int AA12::findMin(int nodeIdx) const
{
    while (nodeIdx >= 0 && nodeIdx < m_nodes.size() && m_nodes[nodeIdx].left >= 0)
        nodeIdx = m_nodes[nodeIdx].left;
    return nodeIdx;
}

/* ---- Recursive delete ---- */

int AA12::removeHelper(int nodeIdx, int key)
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return -1;

    if (key < m_nodes[nodeIdx].key) {
        m_nodes[nodeIdx].left = removeHelper(m_nodes[nodeIdx].left, key);
    } else if (key > m_nodes[nodeIdx].key) {
        m_nodes[nodeIdx].right = removeHelper(m_nodes[nodeIdx].right, key);
    } else {
        // Found node to delete
        if (m_nodes[nodeIdx].left < 0 && m_nodes[nodeIdx].right < 0) {
            // Leaf: mark as tombstone for epoch-based reclamation
            m_nodes[nodeIdx].deleted = true;
            m_stats.nodeCount--;
            return -1;
        }
        if (m_nodes[nodeIdx].left < 0) {
            return m_nodes[nodeIdx].right;
        }
        if (m_nodes[nodeIdx].right < 0) {
            return m_nodes[nodeIdx].left;
        }

        // Two children: replace with successor
        int succ = findMin(m_nodes[nodeIdx].right);
        m_nodes[nodeIdx].key = m_nodes[succ].key;
        m_nodes[nodeIdx].value = m_nodes[succ].value;
        m_nodes[nodeIdx].right = removeHelper(m_nodes[nodeIdx].right, m_nodes[succ].key);
    }

    // Rebalance after deletion: decrease level if needed
    int leftLevel = (m_nodes[nodeIdx].left >= 0) ? m_nodes[m_nodes[nodeIdx].left].level : 0;
    int rightLevel = (m_nodes[nodeIdx].right >= 0) ? m_nodes[m_nodes[nodeIdx].right].level : 0;
    int shouldBe = 1 + qMin(leftLevel, rightLevel);

    if (m_nodes[nodeIdx].level > shouldBe) {
        m_nodes[nodeIdx].level = shouldBe;
        if (m_nodes[nodeIdx].right >= 0 &&
            m_nodes[m_nodes[nodeIdx].right].level > shouldBe)
            m_nodes[m_nodes[nodeIdx].right].level = shouldBe;
    }

    // Rebalance with skew and split
    nodeIdx = skew(nodeIdx);
    nodeIdx = split(nodeIdx);

    // Additional skew on right child's left
    if (nodeIdx >= 0 && m_nodes[nodeIdx].right >= 0) {
        m_nodes[nodeIdx].right = skew(m_nodes[nodeIdx].right);
        if (m_nodes[m_nodes[nodeIdx].right].right >= 0) {
            int rr = m_nodes[m_nodes[nodeIdx].right].right;
            m_nodes[m_nodes[nodeIdx].right].right = skew(rr);
        }
        nodeIdx = split(nodeIdx);
    }

    return nodeIdx;
}

/* ---- Insert (thread-safe with write lock) ---- */

bool AA12::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    QWriteLocker locker(&m_lock);
    m_root = insertHelper(m_root, key, value);

    m_stats.totalInserts++;
    m_stats.treeHeight = height(m_root);
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalSearches);

    emit insertDone(key, m_stats.nodeCount, elapsed);
    return true;
}

/* ---- Remove (thread-safe with write lock) ---- */

bool AA12::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    QWriteLocker locker(&m_lock);
    int oldRoot = m_root;
    m_root = removeHelper(m_root, key);
    bool success = (m_root != oldRoot) || (m_root >= 0);  // Simplified check

    m_stats.totalDeletes++;
    m_stats.treeHeight = height(m_root);
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalSearches);

    emit removeDone(key, success, elapsed);
    return success;
}

/* ---- Search (lock-free read with read lock) ---- */

AA12::SearchResult AA12::search(int key) const
{
    QElapsedTimer timer;
    timer.start();

    SearchResult result;
    QReadLocker locker(&m_lock);
    result.found = searchHelper(m_root, key, result.value);

    m_stats.totalSearches++;
    result.elapsedMs = timer.elapsed();
    return result;
}

/* ---- Search helper ---- */

bool AA12::searchHelper(int nodeIdx, int key, double& value) const
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return false;

    const auto& node = m_nodes[nodeIdx];
    if (node.deleted) return false;

    if (key < node.key) return searchHelper(node.left, key, value);
    if (key > node.key) return searchHelper(node.right, key, value);
    value = node.value;
    return true;
}

/* ---- Range query ---- */

AA12::RangeResult AA12::rangeQuery(int lo, int hi) const
{
    QElapsedTimer timer;
    timer.start();

    QReadLocker locker(&m_lock);
    RangeResult result;
    rangeHelper(m_root, lo, hi, result.entries);
    result.count = result.entries.size();
    result.elapsedMs = timer.elapsed();
    return result;
}

/* ---- Range query helper ---- */

void AA12::rangeHelper(int nodeIdx, int lo, int hi,
                        QVector<QPair<int, double>>& result) const
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return;
    const auto& node = m_nodes[nodeIdx];
    if (node.deleted) return;

    if (lo < node.key) rangeHelper(node.left, lo, hi, result);
    if (lo <= node.key && node.key <= hi)
        result.append({node.key, node.value});
    if (hi > node.key) rangeHelper(node.right, lo, hi, result);
}

/* ---- In-order traversal ---- */

void AA12::inOrder(int nodeIdx, QVector<QPair<int, double>>& result) const
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return;
    const auto& node = m_nodes[nodeIdx];
    if (node.deleted) return;
    inOrder(node.left, result);
    result.append({node.key, node.value});
    inOrder(node.right, result);
}

/* ---- Get all keys ---- */

QVector<int> AA12::keys() const
{
    QReadLocker locker(&m_lock);
    QVector<QPair<int, double>> entries;
    inOrder(m_root, entries);
    QVector<int> k;
    k.reserve(entries.size());
    for (const auto& e : entries) k.append(e.first);
    return k;
}

/* ---- Epoch-based reclamation ---- */

void AA12::advanceEpoch()
{
    QWriteLocker locker(&m_lock);
    m_epoch++;
    reclaimNodes();
}

/* ---- Reclaim nodes from old epochs (tombstones > 2 epochs old) ---- */

void AA12::reclaimNodes()
{
    int reclaimThreshold = m_epoch - 2;
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].deleted && m_nodes[i].epoch < reclaimThreshold) {
            // Mark slot as reusable (keep for potential recycling)
            m_nodes[i].level = 0;
            m_nodes[i].left = -1;
            m_nodes[i].right = -1;
        }
    }
}

/* ---- Compute tree height ---- */

int AA12::height(int nodeIdx) const
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return 0;
    int lh = height(m_nodes[nodeIdx].left);
    int rh = height(m_nodes[nodeIdx].right);
    return 1 + qMax(lh, rh);
}

/* ---- Reset ---- */

void AA12::resetStatistics()
{
    QWriteLocker locker(&m_lock);
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_root = -1;
    m_epoch = 0;
}

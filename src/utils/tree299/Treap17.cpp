/**
 * @file Treap17.cpp
 * @brief Treap17 实现
 *
 * 实现树堆：确定性优先级哈希与指针搜索实现局部性感知顺序访问模式优化。
 */

#include "utils/tree299/Treap17.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Treap17::Treap17(QObject *parent)
    : QObject(parent) {}

Treap17::~Treap17() = default;

/* ---- Deterministic priority via FNV-1a inspired hash ---- */

quint64 Treap17::hashPriority(double key) const
{
    // Convert double to bits for deterministic hashing
    quint64 bits = 0;
    memcpy(&bits, &key, sizeof(double));

    // FNV-1a style hash
    quint64 hash = 14695981039346656037ULL;
    for (int i = 0; i < 8; ++i) {
        hash ^= (bits >> (i * 8)) & 0xFF;
        hash *= 1099511628211ULL;
    }
    return hash;
}

/* ---- Node allocation ---- */

int Treap17::allocNode(double key)
{
    Node n;
    n.key = key;
    n.priority = hashPriority(key);
    n.left = -1;
    n.right = -1;
    n.parent = -1;
    n.subtreeSize = 1;
    m_nodes.append(n);
    return m_nodes.size() - 1;
}

/* ---- Rotate right (promote left child) ---- */

int Treap17::rotateRight(int idx)
{
    int l = m_nodes[idx].left;
    if (l < 0) return idx;

    m_nodes[idx].left = m_nodes[l].right;
    if (m_nodes[l].right >= 0)
        m_nodes[m_nodes[l].right].parent = idx;

    m_nodes[l].parent = m_nodes[idx].parent;
    m_nodes[idx].parent = l;
    m_nodes[l].right = idx;

    // Update sizes
    m_nodes[idx].subtreeSize = 1 +
        ((m_nodes[idx].left >= 0) ? m_nodes[m_nodes[idx].left].subtreeSize : 0) +
        ((m_nodes[idx].right >= 0) ? m_nodes[m_nodes[idx].right].subtreeSize : 0);
    m_nodes[l].subtreeSize = 1 +
        ((m_nodes[l].left >= 0) ? m_nodes[m_nodes[l].left].subtreeSize : 0) +
        m_nodes[idx].subtreeSize;

    return l;
}

/* ---- Rotate left (promote right child) ---- */

int Treap17::rotateLeft(int idx)
{
    int r = m_nodes[idx].right;
    if (r < 0) return idx;

    m_nodes[idx].right = m_nodes[r].left;
    if (m_nodes[r].left >= 0)
        m_nodes[m_nodes[r].left].parent = idx;

    m_nodes[r].parent = m_nodes[idx].parent;
    m_nodes[idx].parent = r;
    m_nodes[r].left = idx;

    m_nodes[idx].subtreeSize = 1 +
        ((m_nodes[idx].left >= 0) ? m_nodes[m_nodes[idx].left].subtreeSize : 0) +
        ((m_nodes[idx].right >= 0) ? m_nodes[m_nodes[idx].right].subtreeSize : 0);
    m_nodes[r].subtreeSize = 1 +
        m_nodes[idx].subtreeSize +
        ((m_nodes[r].right >= 0) ? m_nodes[m_nodes[r].right].subtreeSize : 0);

    return r;
}

/* ---- Heapify up: maintain max-heap property ---- */

void Treap17::heapifyUp(int idx)
{
    while (idx >= 0) {
        int p = m_nodes[idx].parent;
        if (p < 0 || m_nodes[idx].priority <= m_nodes[p].priority) break;

        int newP = (m_nodes[p].left == idx) ? rotateRight(p) : rotateLeft(p);
        int gp = m_nodes[newP].parent;
        if (gp >= 0) {
            if (m_nodes[gp].left == p) m_nodes[gp].left = newP;
            else m_nodes[gp].right = newP;
        } else {
            m_root = newP;
        }
    }
}

/* ---- Heapify down: maintain max-heap property ---- */

void Treap17::heapifyDown(int idx)
{
    while (idx >= 0) {
        int maxChild = -1;
        quint64 maxPri = 0;

        if (m_nodes[idx].left >= 0 && m_nodes[m_nodes[idx].left].priority > maxPri) {
            maxPri = m_nodes[m_nodes[idx].left].priority;
            maxChild = m_nodes[idx].left;
        }
        if (m_nodes[idx].right >= 0 && m_nodes[m_nodes[idx].right].priority > maxPri) {
            maxPri = m_nodes[m_nodes[idx].right].priority;
            maxChild = m_nodes[idx].right;
        }

        if (maxChild < 0 || m_nodes[idx].priority >= maxPri) break;

        int newIdx;
        if (maxChild == m_nodes[idx].left)
            newIdx = rotateRight(idx);
        else
            newIdx = rotateLeft(idx);

        int gp = m_nodes[newIdx].parent;
        if (gp >= 0) {
            if (m_nodes[gp].left == idx)
                m_nodes[gp].left = newIdx;
            else
                m_nodes[gp].right = newIdx;
        } else {
            m_root = newIdx;
        }
        // Continue heapifying at the new position of the original node
        // After rotation, idx is now a child of newIdx
    }
}

/* ---- Update sizes ---- */

void Treap17::updateSizes(int idx)
{
    while (idx >= 0) {
        m_nodes[idx].subtreeSize = 1 +
            ((m_nodes[idx].left >= 0) ? m_nodes[m_nodes[idx].left].subtreeSize : 0) +
            ((m_nodes[idx].right >= 0) ? m_nodes[m_nodes[idx].right].subtreeSize : 0);
        idx = m_nodes[idx].parent;
    }
}

/* ---- Insert ---- */

bool Treap17::insert(double key)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root < 0) {
        m_root = allocNode(key);
        m_stats.treeSize = m_nodes.size();
        m_stats.totalInserts++;
        double el = timer.elapsed();
        m_timeSum += el;
        m_stats.avgProcessingTimeMs = m_timeSum /
            (m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalSearches);
        emit insertDone(key, m_stats.treeSize, el);
        return true;
    }

    // BST insert
    int cur = m_root, parent = -1;
    bool isLeft = false;
    while (cur >= 0) {
        parent = cur;
        if (key < m_nodes[cur].key) { cur = m_nodes[cur].left; isLeft = true; }
        else if (key > m_nodes[cur].key) { cur = m_nodes[cur].right; isLeft = false; }
        else { return false; } // duplicate
    }

    int newIdx = allocNode(key);
    m_nodes[newIdx].parent = parent;
    if (isLeft) m_nodes[parent].left = newIdx;
    else m_nodes[parent].right = newIdx;

    // Update sizes up the tree
    updateSizes(parent);

    // Heapify up to maintain treap property
    heapifyUp(newIdx);

    m_stats.treeSize = m_nodes.size();
    m_stats.totalInserts++;
    double el = timer.elapsed();
    m_timeSum += el;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalSearches);

    emit insertDone(key, m_stats.treeSize, el);
    return true;
}

/* ---- Remove ---- */

bool Treap17::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    // Find node
    int cur = m_root;
    while (cur >= 0) {
        if (key < m_nodes[cur].key) cur = m_nodes[cur].left;
        else if (key > m_nodes[cur].key) cur = m_nodes[cur].right;
        else break;
    }
    if (cur < 0) return false;

    // Push node down to leaf via rotations, then remove
    while (m_nodes[cur].left >= 0 || m_nodes[cur].right >= 0) {
        int l = m_nodes[cur].left;
        int r = m_nodes[cur].right;
        int newCur;

        if (l < 0) {
            newCur = rotateLeft(cur);
        } else if (r < 0) {
            newCur = rotateRight(cur);
        } else if (m_nodes[l].priority > m_nodes[r].priority) {
            newCur = rotateRight(cur);
        } else {
            newCur = rotateLeft(cur);
        }

        int gp = m_nodes[newCur].parent;
        if (gp >= 0) {
            if (m_nodes[gp].left == cur)
                m_nodes[gp].left = newCur;
            else
                m_nodes[gp].right = newCur;
        } else {
            m_root = newCur;
        }
    }

    // Now cur is a leaf, unlink it
    int p = m_nodes[cur].parent;
    if (p >= 0) {
        if (m_nodes[p].left == cur)
            m_nodes[p].left = -1;
        else
            m_nodes[p].right = -1;
        updateSizes(p);
    } else {
        m_root = -1;
    }

    // Reset finger if it pointed to removed node
    if (m_finger == cur) m_finger = -1;

    m_stats.treeSize = m_nodes.size();
    m_stats.totalDeletes++;
    double el = timer.elapsed();
    m_timeSum += el;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalSearches);

    emit removeDone(key, m_stats.treeSize, el);
    return true;
}

/* ---- Root search ---- */

Treap17::SearchResult Treap17::rootSearch(double key) const
{
    SearchResult result;
    int cur = m_root;
    int depth = 0;

    while (cur >= 0) {
        result.comparisons++;
        if (key < m_nodes[cur].key) {
            cur = m_nodes[cur].left;
            depth++;
        } else if (key > m_nodes[cur].key) {
            cur = m_nodes[cur].right;
            depth++;
        } else {
            result.found = true;
            result.nodeIndex = cur;
            result.depth = depth;
            return result;
        }
    }

    result.found = false;
    result.depth = depth;
    return result;
}

/* ---- Finger search: locality-aware sequential access ---- */

Treap17::SearchResult Treap17::fingerSearch(double key)
{
    SearchResult result;

    if (m_finger < 0 || m_finger >= m_nodes.size()) {
        result = rootSearch(key);
        if (result.found) {
            m_finger = result.nodeIndex;
            m_fingerKey = key;
        }
        return result;
    }

    // Start from finger and walk up/down
    int cur = m_finger;
    int depth = 0;

    // Walk up to find LCA with target key
    while (cur >= 0) {
        result.comparisons++;
        if (key == m_nodes[cur].key) {
            result.found = true;
            result.nodeIndex = cur;
            result.depth = depth;
            result.fingerUsed = true;
            m_finger = cur;
            m_fingerKey = key;
            m_stats.fingerHits++;
            return result;
        }

        // Determine direction
        if ((key < m_nodes[cur].key && m_fingerKey >= m_nodes[cur].key) ||
            (key >= m_nodes[cur].key && m_fingerKey < m_nodes[cur].key)) {
            // Crossed the LCA, now search in subtree
            break;
        }

        cur = m_nodes[cur].parent;
        depth++;
    }

    // Now search down from current position
    if (cur < 0) cur = m_root;
    while (cur >= 0) {
        result.comparisons++;
        if (key < m_nodes[cur].key) {
            cur = m_nodes[cur].left;
            depth++;
        } else if (key > m_nodes[cur].key) {
            cur = m_nodes[cur].right;
            depth++;
        } else {
            result.found = true;
            result.nodeIndex = cur;
            result.depth = depth;
            result.fingerUsed = true;
            m_finger = cur;
            m_fingerKey = key;
            m_stats.fingerHits++;
            return result;
        }
    }

    result.found = false;
    result.depth = depth;
    return result;
}

/* ---- Search (dispatches to finger or root search) ---- */

Treap17::SearchResult Treap17::search(double key)
{
    SearchResult result;

    // Use finger search for sequential access patterns
    if (m_finger >= 0) {
        // Check if key is near finger key (sequential locality)
        double dist = qAbs(key - m_fingerKey);
        double treeRange = 1.0;
        if (m_root >= 0) {
            // Estimate range
            int minIdx = m_root, maxIdx = m_root;
            while (m_nodes[minIdx].left >= 0) minIdx = m_nodes[minIdx].left;
            while (m_nodes[maxIdx].right >= 0) maxIdx = m_nodes[maxIdx].right;
            treeRange = qMax(1.0, m_nodes[maxIdx].key - m_nodes[minIdx].key);
        }
        if (dist < treeRange * 0.1) {
            result = fingerSearch(key);
        } else {
            result = rootSearch(key);
            if (result.found) {
                m_finger = result.nodeIndex;
                m_fingerKey = key;
            }
        }
    } else {
        result = rootSearch(key);
        if (result.found) {
            m_finger = result.nodeIndex;
            m_fingerKey = key;
        }
    }

    m_stats.totalSearches++;
    return result;
}

/* ---- Set finger ---- */

void Treap17::setFinger(double key)
{
    auto r = rootSearch(key);
    if (r.found) {
        m_finger = r.nodeIndex;
        m_fingerKey = key;
    }
}

/* ---- Range query ---- */

void Treap17::rangeHelper(int idx, double lo, double hi,
                            QVector<double>& result) const
{
    if (idx < 0 || idx >= m_nodes.size()) return;
    if (m_nodes[idx].key > lo)
        rangeHelper(m_nodes[idx].left, lo, hi, result);
    if (m_nodes[idx].key >= lo && m_nodes[idx].key <= hi)
        result.append(m_nodes[idx].key);
    if (m_nodes[idx].key < hi)
        rangeHelper(m_nodes[idx].right, lo, hi, result);
}

Treap17::RangeResult Treap17::rangeQuery(double lo, double hi) const
{
    RangeResult result;
    rangeHelper(m_root, lo, hi, result.keys);
    result.count = result.keys.size();
    return result;
}

/* ---- In-order traversal ---- */

void Treap17::inOrderHelper(int idx, QVector<double>& result) const
{
    if (idx < 0 || idx >= m_nodes.size()) return;
    inOrderHelper(m_nodes[idx].left, result);
    result.append(m_nodes[idx].key);
    inOrderHelper(m_nodes[idx].right, result);
}

QVector<double> Treap17::inOrder() const
{
    QVector<double> result;
    inOrderHelper(m_root, result);
    return result;
}

/* ---- Reset ---- */

void Treap17::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_root = m_finger = -1;
    m_fingerKey = 0.0;
}

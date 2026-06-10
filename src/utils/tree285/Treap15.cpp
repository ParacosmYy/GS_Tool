/**
 * @file Treap15.cpp
 * @brief Treap15 实现
 *
 * 实现树堆：哈希确定性优先级与隐式分裂合并的数组索引序列操作。
 */

#include "utils/tree285/Treap15.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Treap15::Treap15(QObject *parent)
    : QObject(parent) {}

Treap15::~Treap15() = default;

/* ---- Deterministic hash priority ---- */

quint64 Treap15::hashPriority(int key) const
{
    // FNV-1a inspired deterministic hash
    quint64 h = 14695981039346656037ULL;
    h ^= static_cast<quint64>(key) & 0xFF;
    h *= 1099511628211ULL;
    h ^= (static_cast<quint64>(key) >> 8) & 0xFF;
    h *= 1099511628211ULL;
    h ^= (static_cast<quint64>(key) >> 16) & 0xFF;
    h *= 1099511628211ULL;
    h ^= (static_cast<quint64>(key) >> 24) & 0xFF;
    h *= 1099511628211ULL;
    return h;
}

/* ---- Node allocation ---- */

int Treap15::allocNode(int key, double value)
{
    int idx;
    if (!m_freeList.isEmpty()) {
        idx = m_freeList.takeLast();
        m_nodes[idx] = TreapNode{key, value, hashPriority(key), -1, -1, -1, 1, false};
    } else {
        idx = m_nodes.size();
        m_nodes.append(TreapNode{key, value, hashPriority(key), -1, -1, -1, 1, false});
    }
    return idx;
}

void Treap15::freeNode(int idx)
{
    m_freeList.append(idx);
}

/* ---- Update subtree size ---- */

void Treap15::updateSize(int idx)
{
    if (idx < 0) return;
    TreapNode& n = m_nodes[idx];
    int ls = (n.left >= 0) ? m_nodes[n.left].size : 0;
    int rs = (n.right >= 0) ? m_nodes[n.right].size : 0;
    n.size = ls + rs + 1;
}

/* ---- Push down lazy reverse ---- */

void Treap15::pushDown(int idx)
{
    if (idx < 0) return;
    TreapNode& n = m_nodes[idx];
    if (!n.reversed) return;
    n.reversed = false;

    // Swap children
    std::swap(n.left, n.right);

    // Propagate to children
    if (n.left >= 0) m_nodes[n.left].reversed = !m_nodes[n.left].reversed;
    if (n.right >= 0) m_nodes[n.right].reversed = !m_nodes[n.right].reversed;
}

/* ---- Implicit treap split ---- */

void Treap15::split(int rootIdx, int pos, int& left, int& right)
{
    if (rootIdx < 0) { left = -1; right = -1; return; }

    pushDown(rootIdx);

    TreapNode& n = m_nodes[rootIdx];
    int leftSize = (n.left >= 0) ? m_nodes[n.left].size : 0;

    if (leftSize >= pos) {
        // Split point is in left subtree
        split(n.left, pos, left, n.left);
        right = rootIdx;
        if (n.left >= 0) m_nodes[n.left].parent = rootIdx;
        updateSize(rootIdx);
    } else {
        // Split point is in right subtree
        split(n.right, pos - leftSize - 1, n.right, right);
        left = rootIdx;
        if (n.right >= 0) m_nodes[n.right].parent = rootIdx;
        updateSize(rootIdx);
    }
}

/* ---- Merge two treaps ---- */

int Treap15::merge(int leftIdx, int rightIdx)
{
    if (leftIdx < 0) return rightIdx;
    if (rightIdx < 0) return leftIdx;

    pushDown(leftIdx);
    pushDown(rightIdx);

    // Higher priority becomes root
    if (m_nodes[leftIdx].priority > m_nodes[rightIdx].priority) {
        m_nodes[leftIdx].right = merge(m_nodes[leftIdx].right, rightIdx);
        if (m_nodes[leftIdx].right >= 0)
            m_nodes[m_nodes[leftIdx].right].parent = leftIdx;
        updateSize(leftIdx);
        return leftIdx;
    } else {
        m_nodes[rightIdx].left = merge(leftIdx, m_nodes[rightIdx].left);
        if (m_nodes[rightIdx].left >= 0)
            m_nodes[m_nodes[rightIdx].left].parent = rightIdx;
        updateSize(rightIdx);
        return rightIdx;
    }
}

/* ---- Rotate up (for BST mode) ---- */

int Treap15::rotateUp(int idx)
{
    if (idx < 0) return idx;
    int p = m_nodes[idx].parent;
    if (p < 0) return idx;

    int gp = m_nodes[p].parent;

    if (m_nodes[p].left == idx) {
        // Right rotation
        m_nodes[p].left = m_nodes[idx].right;
        if (m_nodes[idx].right >= 0)
            m_nodes[m_nodes[idx].right].parent = p;
        m_nodes[idx].right = p;
        m_nodes[p].parent = idx;
    } else {
        // Left rotation
        m_nodes[p].right = m_nodes[idx].left;
        if (m_nodes[idx].left >= 0)
            m_nodes[m_nodes[idx].left].parent = p;
        m_nodes[idx].left = p;
        m_nodes[p].parent = idx;
    }

    m_nodes[idx].parent = gp;
    if (gp >= 0) {
        if (m_nodes[gp].left == p) m_nodes[gp].left = idx;
        else m_nodes[gp].right = idx;
    } else {
        m_root = idx;
    }

    updateSize(p);
    updateSize(idx);

    return idx;
}

/* ---- BST insert with heap property ---- */

int Treap15::bstInsert(int rootIdx, int nodeIdx)
{
    if (rootIdx < 0) return nodeIdx;

    pushDown(rootIdx);

    if (m_nodes[nodeIdx].key < m_nodes[rootIdx].key) {
        m_nodes[rootIdx].left = bstInsert(m_nodes[rootIdx].left, nodeIdx);
        m_nodes[m_nodes[rootIdx].left].parent = rootIdx;
    } else {
        m_nodes[rootIdx].right = bstInsert(m_nodes[rootIdx].right, nodeIdx);
        m_nodes[m_nodes[rootIdx].right].parent = rootIdx;
    }

    updateSize(rootIdx);

    // Rotate up if heap property violated
    if (m_nodes[nodeIdx].parent == rootIdx &&
        m_nodes[nodeIdx].priority > m_nodes[rootIdx].priority) {
        return rotateUp(nodeIdx);
    }

    return rootIdx;
}

/* ---- Insert at position (implicit treap) ---- */

bool Treap15::insertAt(int pos, int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    int nodeIdx = allocNode(key, value);
    m_nodes[nodeIdx].priority = hashPriority(m_size);  // Position-based priority

    int left = -1, right = -1;
    split(m_root, pos, left, right);

    m_root = merge(merge(left, nodeIdx), right);
    m_size++;

    m_stats.treeSize = m_size;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit insertDone(key, pos, timer.elapsed());

    return true;
}

/* ---- Remove at position ---- */

bool Treap15::removeAt(int pos)
{
    QElapsedTimer timer;
    timer.start();

    if (pos < 0 || pos >= m_size) return false;

    int left = -1, mid = -1, right = -1;

    split(m_root, pos, left, right);
    split(right, 1, mid, right);

    if (mid >= 0) {
        int key = m_nodes[mid].key;
        freeNode(mid);
        m_size--;

        m_root = merge(left, right);

        m_stats.treeSize = m_size;
        m_stats.treeHeight = computeHeight(m_root);
        m_stats.totalOps++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
        emit removeDone(key, pos, timer.elapsed());
        return true;
    }

    m_root = merge(left, right);
    return false;
}

/* ---- Get at position ---- */

Treap15::SearchResult Treap15::getAt(int pos) const
{
    SearchResult result;
    if (pos < 0 || pos >= m_size) return result;

    int current = m_root;
    int offset = 0;

    while (current >= 0) {
        const_cast<Treap15*>(this)->pushDown(current);

        int leftSize = (m_nodes[current].left >= 0)
                       ? m_nodes[m_nodes[current].left].size : 0;

        if (pos - offset < leftSize) {
            current = m_nodes[current].left;
        } else if (pos - offset == leftSize) {
            result.nodeIdx = current;
            result.value = m_nodes[current].value;
            result.found = true;
            result.position = pos;
            return result;
        } else {
            offset += leftSize + 1;
            current = m_nodes[current].right;
        }
    }

    return result;
}

/* ---- BST insert by key ---- */

bool Treap15::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    int nodeIdx = allocNode(key, value);
    m_root = bstInsert(m_root, nodeIdx);

    // Bubble up to maintain heap property
    while (nodeIdx >= 0 && m_nodes[nodeIdx].parent >= 0) {
        int p = m_nodes[nodeIdx].parent;
        if (m_nodes[nodeIdx].priority > m_nodes[p].priority)
            rotateUp(nodeIdx);
        else
            break;
    }

    m_size++;
    m_stats.treeSize = m_size;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit insertDone(key, -1, timer.elapsed());

    return true;
}

/* ---- BST remove by key ---- */

bool Treap15::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    // Find node
    int current = m_root;
    while (current >= 0) {
        if (key < m_nodes[current].key)
            current = m_nodes[current].left;
        else if (key > m_nodes[current].key)
            current = m_nodes[current].right;
        else
            break;
    }

    if (current < 0) return false;

    // Push priority down, then remove leaf
    while (m_nodes[current].left >= 0 || m_nodes[current].right >= 0) {
        pushDown(current);
        int l = m_nodes[current].left;
        int r = m_nodes[current].right;

        if (l >= 0 && (r < 0 || m_nodes[l].priority > m_nodes[r].priority))
            rotateUp(l);
        else
            rotateUp(r);
    }

    // Now current is a leaf
    int p = m_nodes[current].parent;
    if (p < 0) {
        m_root = -1;
    } else {
        if (m_nodes[p].left == current) m_nodes[p].left = -1;
        else m_nodes[p].right = -1;

        // Update sizes up to root
        int walk = p;
        while (walk >= 0) {
            updateSize(walk);
            walk = m_nodes[walk].parent;
        }
    }

    freeNode(current);
    m_size--;

    m_stats.treeSize = m_size;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit removeDone(key, -1, timer.elapsed());

    return true;
}

/* ---- Search by key ---- */

Treap15::SearchResult Treap15::search(int key) const
{
    SearchResult result;
    int current = m_root;
    int depth = 0;

    while (current >= 0) {
        if (key < m_nodes[current].key) {
            current = m_nodes[current].left;
            depth++;
        } else if (key > m_nodes[current].key) {
            current = m_nodes[current].right;
            depth++;
        } else {
            result.nodeIdx = current;
            result.value = m_nodes[current].value;
            result.found = true;
            result.position = depth;
            return result;
        }
    }

    result.position = depth;
    return result;
}

/* ---- Reverse range ---- */

void Treap15::reverseRange(int l, int r)
{
    if (l < 0) l = 0;
    if (r > m_size) r = m_size;
    if (l >= r) return;

    int left = -1, mid = -1, right = -1;

    split(m_root, l, left, right);
    split(right, r - l, mid, right);

    if (mid >= 0)
        m_nodes[mid].reversed = !m_nodes[mid].reversed;

    m_root = merge(merge(left, mid), right);
}

/* ---- Range query ---- */

Treap15::RangeResult Treap15::rangeQuery(int l, int r)
{
    RangeResult result;
    if (l < 0) l = 0;
    if (r > m_size) r = m_size;
    if (l >= r) return result;

    int left = -1, mid = -1, right = -1;

    split(m_root, l, left, right);
    split(right, r - l, mid, right);

    // Collect values from mid subtree
    QVector<QPair<int, double>> items;
    inOrderHelper(mid, items);

    for (const auto& item : items) {
        result.values.append(item.second);
        result.sum += item.second;
    }
    result.count = items.size();

    m_root = merge(merge(left, mid), right);

    return result;
}

/* ---- Inorder traversal ---- */

void Treap15::inOrderHelper(int idx, QVector<QPair<int, double>>& result) const
{
    if (idx < 0) return;
    const_cast<Treap15*>(this)->pushDown(idx);
    inOrderHelper(m_nodes[idx].left, result);
    result.append(qMakePair(m_nodes[idx].key, m_nodes[idx].value));
    inOrderHelper(m_nodes[idx].right, result);
}

/* ---- Compute height ---- */

int Treap15::computeHeight(int idx) const
{
    if (idx < 0) return 0;
    int lh = computeHeight(m_nodes[idx].left);
    int rh = computeHeight(m_nodes[idx].right);
    return 1 + qMax(lh, rh);
}

/* ---- Reset ---- */

void Treap15::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_root = -1;
    m_size = 0;
    m_nodes.clear();
    m_freeList.clear();
}

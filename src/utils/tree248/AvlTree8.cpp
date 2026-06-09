/**
 * @file AvlTree8.cpp
 * @brief AvlTree8 实现
 *
 * 实现AVL树：写时复制并发读优化与细粒度轮转回收。
 */

#include "utils/tree248/AvlTree8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

AvlTree8::AvlTree8(QObject *parent) : QObject(parent)
{
    m_epoch.storeRelaxed(0);
}

AvlTree8::~AvlTree8() { clear(); }

/* ---- Allocate node ---- */

int AvlTree8::allocNode(int key, int value, int epoch)
{
    int idx;
    if (!m_freeList.isEmpty()) {
        idx = m_freeList.takeLast();
        m_pool[idx] = {key, value, 1, -1, -1, 1, epoch};
    } else {
        idx = m_pool.size();
        m_pool.append({key, value, 1, -1, -1, 1, epoch});
    }
    return idx;
}

/* ---- Clone a node (copy-on-write) ---- */

int AvlTree8::cloneNode(int nodeIdx, int epoch)
{
    if (nodeIdx < 0) return -1;
    const auto& src = m_pool[nodeIdx];
    int idx = allocNode(src.key, src.value, epoch);
    m_pool[idx].left = src.left;
    m_pool[idx].right = src.right;
    m_pool[idx].height = src.height;
    m_stats.numCopies++;
    return idx;
}

/* ---- Retire node to epoch bucket ---- */

void AvlTree8::retireNode(int nodeIdx)
{
    if (nodeIdx < 0) return;
    int ep = m_pool[nodeIdx].birthEpoch % 3;
    m_retired[ep].append(nodeIdx);
}

/* ---- Reclaim nodes from oldest epoch ---- */

void AvlTree8::reclaimEpoch(int oldEpoch)
{
    int bucket = oldEpoch % 3;
    for (int idx : m_retired[bucket]) {
        if (idx >= 0 && idx < m_pool.size()) {
            m_pool[idx].refCount--;
            if (m_pool[idx].refCount <= 0)
                m_freeList.append(idx);
        }
    }
    m_retired[bucket].clear();
}

/* ---- Node height helpers ---- */

int AvlTree8::getHeight(int nodeIdx) const
{
    return (nodeIdx >= 0) ? m_pool[nodeIdx].height : 0;
}

void AvlTree8::updateHeight(int nodeIdx)
{
    if (nodeIdx < 0) return;
    int lh = getHeight(m_pool[nodeIdx].left);
    int rh = getHeight(m_pool[nodeIdx].right);
    m_pool[nodeIdx].height = 1 + qMax(lh, rh);
}

int AvlTree8::balanceFactor(int nodeIdx) const
{
    if (nodeIdx < 0) return 0;
    return getHeight(m_pool[nodeIdx].left) - getHeight(m_pool[nodeIdx].right);
}

/* ---- Rotations (COW: clone modified nodes) ---- */

int AvlTree8::rotateRight(int y)
{
    // y is already a COW copy; clone x
    int x = m_pool[y].left;
    int xCopy = cloneNode(x, m_epoch.loadRelaxed());
    int T2 = m_pool[xCopy].right;

    m_pool[xCopy].right = y;
    m_pool[y].left = T2;

    updateHeight(y);
    updateHeight(xCopy);
    return xCopy;
}

int AvlTree8::rotateLeft(int x)
{
    int y = m_pool[x].right;
    int yCopy = cloneNode(y, m_epoch.loadRelaxed());
    int T2 = m_pool[yCopy].left;

    m_pool[yCopy].left = x;
    m_pool[x].right = T2;

    updateHeight(x);
    updateHeight(yCopy);
    return yCopy;
}

/* ---- Balance ---- */

int AvlTree8::balance(int nodeIdx)
{
    if (nodeIdx < 0) return -1;
    updateHeight(nodeIdx);
    int bf = balanceFactor(nodeIdx);

    if (bf > 1) {
        if (balanceFactor(m_pool[nodeIdx].left) < 0) {
            int leftCopy = cloneNode(m_pool[nodeIdx].left, m_epoch.loadRelaxed());
            m_pool[nodeIdx].left = rotateLeft(leftCopy);
        }
        return rotateRight(nodeIdx);
    }
    if (bf < -1) {
        if (balanceFactor(m_pool[nodeIdx].right) > 0) {
            int rightCopy = cloneNode(m_pool[nodeIdx].right, m_epoch.loadRelaxed());
            m_pool[nodeIdx].right = rotateRight(rightCopy);
        }
        return rotateLeft(nodeIdx);
    }
    return nodeIdx;
}

/* ---- Recursive insert with COW ---- */

int AvlTree8::insertRec(int nodeIdx, int key, int value, int epoch)
{
    if (nodeIdx < 0) {
        m_count++;
        return allocNode(key, value, epoch);
    }

    // COW: clone the node before modification
    int copyIdx = cloneNode(nodeIdx, epoch);
    retireNode(nodeIdx);

    auto& node = m_pool[copyIdx];
    if (key < node.key)
        node.left = insertRec(node.left, key, value, epoch);
    else if (key > node.key)
        node.right = insertRec(node.right, key, value, epoch);
    else {
        node.value = value;  // Update existing
        m_count--;           // Don't count duplicate
    }

    return balance(copyIdx);
}

/* ---- Find min ---- */

int AvlTree8::findMin(int nodeIdx) const
{
    while (nodeIdx >= 0 && m_pool[nodeIdx].left >= 0)
        nodeIdx = m_pool[nodeIdx].left;
    return nodeIdx;
}

/* ---- Recursive remove with COW ---- */

int AvlTree8::removeRec(int nodeIdx, int key, int epoch)
{
    if (nodeIdx < 0) return -1;

    int copyIdx = cloneNode(nodeIdx, epoch);
    retireNode(nodeIdx);
    auto& node = m_pool[copyIdx];

    if (key < node.key) {
        node.left = removeRec(node.left, key, epoch);
    } else if (key > node.key) {
        node.right = removeRec(node.right, key, epoch);
    } else {
        m_count--;
        if (node.left < 0) return node.right;
        if (node.right < 0) return node.left;

        int succ = findMin(node.right);
        node.key = m_pool[succ].key;
        node.value = m_pool[succ].value;
        node.right = removeRec(node.right, m_pool[succ].key, epoch);
        m_count++;  // Compensate for double decrement
    }
    return balance(copyIdx);
}

/* ---- Insert ---- */

void AvlTree8::insert(int key, int value)
{
    QElapsedTimer timer;
    timer.start();

    int ep = m_epoch.loadRelaxed();
    m_root = insertRec(m_root, key, value, ep);

    m_stats.numNodes = m_count;
    m_stats.numInsertions++;
    m_stats.treeHeight = getHeight(m_root);
    m_stats.epoch = ep;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit treeModified("insert", key, m_count, timer.elapsed());
}

/* ---- Remove ---- */

void AvlTree8::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    int ep = m_epoch.loadRelaxed();
    m_root = removeRec(m_root, key, ep);

    m_stats.numNodes = m_count;
    m_stats.numDeletions++;
    m_stats.treeHeight = getHeight(m_root);
    m_stats.epoch = ep;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit treeModified("remove", key, m_count, timer.elapsed());
}

/* ---- Search (lock-free read on immutable snapshot) ---- */

int AvlTree8::search(int key) const
{
    int cur = m_root;
    while (cur >= 0) {
        if (key < m_pool[cur].key) cur = m_pool[cur].left;
        else if (key > m_pool[cur].key) cur = m_pool[cur].right;
        else return m_pool[cur].value;
    }
    const_cast<AvlTree8*>(this)->m_stats.numSearches++;
    return -1;
}

bool AvlTree8::contains(int key) const
{
    int cur = m_root;
    while (cur >= 0) {
        if (key < m_pool[cur].key) cur = m_pool[cur].left;
        else if (key > m_pool[cur].key) cur = m_pool[cur].right;
        else return true;
    }
    const_cast<AvlTree8*>(this)->m_stats.numSearches++;
    return false;
}

/* ---- In-order traversal ---- */

void AvlTree8::inorderRec(int nodeIdx, QVector<QPair<int, int>>& result) const
{
    if (nodeIdx < 0) return;
    inorderRec(m_pool[nodeIdx].left, result);
    result.append({m_pool[nodeIdx].key, m_pool[nodeIdx].value});
    inorderRec(m_pool[nodeIdx].right, result);
}

QVector<QPair<int, int>> AvlTree8::inorder() const
{
    QVector<QPair<int, int>> result;
    inorderRec(m_root, result);
    return result;
}

/* ---- Size ---- */

int AvlTree8::size() const { return m_count; }

/* ---- Clear ---- */

void AvlTree8::clear()
{
    m_pool.clear();
    m_freeList.clear();
    m_root = -1;
    m_count = 0;
    for (int i = 0; i < 3; ++i) m_retired[i].clear();
}

/* ---- Advance epoch ---- */

void AvlTree8::advanceEpoch()
{
    int old = m_epoch.loadRelaxed();
    m_epoch.storeRelaxed(old + 1);
    reclaimEpoch(old - 2);  // Reclaim 2 epochs ago
}

/* ---- Reset ---- */

void AvlTree8::resetStatistics()
{
    clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

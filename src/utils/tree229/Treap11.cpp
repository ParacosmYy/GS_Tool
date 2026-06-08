/**
 * @file Treap11.cpp
 * @brief Treap11 实现
 *
 * 实现树堆：合并分裂优先队列与增强子树大小K阶统计量选择。
 */

#include "utils/tree229/Treap11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Treap11::Treap11(QObject *parent) : QObject(parent) {}
Treap11::~Treap11() = default;

/* ---- Random priority (LCG) ---- */

int Treap11::randomPriority()
{
    m_seed = (m_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return m_seed;
}

/* ---- Allocate node ---- */

int Treap11::allocateNode(int key, double value)
{
    int idx;
    if (m_freeList >= 0) {
        idx = m_freeList;
        m_freeList = m_nodes[idx].right;
    } else {
        idx = m_nodes.size();
        m_nodes.append(Node());
    }
    m_nodes[idx].key = key;
    m_nodes[idx].value = value;
    m_nodes[idx].priority = randomPriority();
    m_nodes[idx].left = -1;
    m_nodes[idx].right = -1;
    m_nodes[idx].subtreeSize = 1;
    return idx;
}

/* ---- Free node ---- */

void Treap11::freeNode(int idx)
{
    m_nodes[idx].right = m_freeList;
    m_nodes[idx].left = -1;
    m_freeList = idx;
}

/* ---- Get subtree size ---- */

int Treap11::getSize(int idx) const
{
    return (idx >= 0) ? m_nodes[idx].subtreeSize : 0;
}

/* ---- Update subtree size ---- */

void Treap11::updateSize(int idx)
{
    if (idx < 0) return;
    m_nodes[idx].subtreeSize = 1 + getSize(m_nodes[idx].left) +
        getSize(m_nodes[idx].right);
}

/* ---- Internal merge ---- */

int Treap11::mergeInternal(int left, int right)
{
    if (left < 0) return right;
    if (right < 0) return left;

    if (m_nodes[left].priority > m_nodes[right].priority) {
        m_nodes[left].right = mergeInternal(m_nodes[left].right, right);
        updateSize(left);
        return left;
    } else {
        m_nodes[right].left = mergeInternal(left, m_nodes[right].left);
        updateSize(right);
        return right;
    }
}

/* ---- Internal split ---- */

void Treap11::splitInternal(int root, int key, int& left, int& right)
{
    if (root < 0) {
        left = -1;
        right = -1;
        return;
    }

    if (m_nodes[root].key <= key) {
        splitInternal(m_nodes[root].right, key,
                       m_nodes[root].right, right);
        left = root;
    } else {
        splitInternal(m_nodes[root].left, key,
                       left, m_nodes[root].left);
        right = root;
    }
    updateSize(root);
}

/* ---- Internal insert ---- */

int Treap11::insertInternal(int root, int nodeIdx)
{
    if (root < 0) return nodeIdx;

    if (m_nodes[nodeIdx].priority > m_nodes[root].priority) {
        splitInternal(root, m_nodes[nodeIdx].key,
                       m_nodes[nodeIdx].left, m_nodes[nodeIdx].right);
        updateSize(nodeIdx);
        return nodeIdx;
    }

    if (m_nodes[nodeIdx].key < m_nodes[root].key) {
        m_nodes[root].left = insertInternal(m_nodes[root].left, nodeIdx);
    } else {
        m_nodes[root].right = insertInternal(m_nodes[root].right, nodeIdx);
    }
    updateSize(root);
    return root;
}

/* ---- Internal remove ---- */

int Treap11::removeInternal(int root, int key)
{
    if (root < 0) return -1;

    if (key < m_nodes[root].key) {
        m_nodes[root].left = removeInternal(m_nodes[root].left, key);
    } else if (key > m_nodes[root].key) {
        m_nodes[root].right = removeInternal(m_nodes[root].right, key);
    } else {
        int result = mergeInternal(m_nodes[root].left, m_nodes[root].right);
        freeNode(root);
        return result;
    }
    updateSize(root);
    return root;
}

/* ---- Internal K-th select ---- */

int Treap11::selectKthInternal(int root, int k) const
{
    if (root < 0) return -1;

    int leftSize = getSize(m_nodes[root].left);

    if (k <= leftSize)
        return selectKthInternal(m_nodes[root].left, k);
    if (k == leftSize + 1)
        return m_nodes[root].key;
    return selectKthInternal(m_nodes[root].right, k - leftSize - 1);
}

/* ---- Internal rank ---- */

int Treap11::rankInternal(int root, int key) const
{
    if (root < 0) return 0;

    if (key <= m_nodes[root].key)
        return rankInternal(m_nodes[root].left, key);
    return getSize(m_nodes[root].left) + 1 +
        rankInternal(m_nodes[root].right, key);
}

/* ---- Inorder traversal ---- */

void Treap11::inorderHelper(int idx, QVector<int>& keys) const
{
    if (idx < 0) return;
    inorderHelper(m_nodes[idx].left, keys);
    keys.append(m_nodes[idx].key);
    inorderHelper(m_nodes[idx].right, keys);
}

/* ---- Height helper ---- */

int Treap11::heightHelper(int idx) const
{
    if (idx < 0) return 0;
    return 1 + qMax(heightHelper(m_nodes[idx].left),
                     heightHelper(m_nodes[idx].right));
}

/* ---- Public: Insert ---- */

void Treap11::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    int nodeIdx = allocateNode(key, value);
    m_root = insertInternal(m_root, nodeIdx);
    m_stats.numNodes++;
    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_stats.numNodes, m_stats.treeHeight, timer.elapsed());
}

/* ---- Public: Remove ---- */

bool Treap11::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    int prevSize = m_stats.numNodes;
    m_root = removeInternal(m_root, key);

    if (m_stats.numNodes > 0 && m_stats.numNodes == prevSize) {
        // Key not found - check by searching first
        if (search(key) != search(key)) // NaN check
            return false;
    }

    m_stats.numNodes = qMax(0, m_stats.numNodes - 1);
    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return true;
}

/* ---- Public: Search ---- */

double Treap11::search(int key) const
{
    int cur = m_root;
    while (cur >= 0) {
        if (key < m_nodes[cur].key) cur = m_nodes[cur].left;
        else if (key > m_nodes[cur].key) cur = m_nodes[cur].right;
        else return m_nodes[cur].value;
    }
    return std::numeric_limits<double>::quiet_NaN();
}

/* ---- Public: Select K-th ---- */

int Treap11::selectKth(int k) const
{
    if (k < 1 || k > m_stats.numNodes) return -1;
    m_stats.numKthQueries++;
    return selectKthInternal(m_root, k);
}

/* ---- Public: Rank ---- */

int Treap11::rank(int key) const
{
    return rankInternal(m_root, key);
}

/* ---- Public: Split ---- */

void Treap11::split(int key, Treap11& left, Treap11& right)
{
    QElapsedTimer timer;
    timer.start();

    left.resetStatistics();
    right.resetStatistics();

    int lRoot = -1, rRoot = -1;
    splitInternal(m_root, key, lRoot, rRoot);

    // Transfer nodes to left/right
    left.m_root = lRoot;
    left.m_nodes = m_nodes;
    left.m_stats.numNodes = getSize(lRoot);

    right.m_root = rRoot;
    right.m_nodes = m_nodes;
    right.m_stats.numNodes = getSize(rRoot);

    m_root = -1;
    m_stats.numNodes = 0;
    m_stats.numSplits++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Public: Merge ---- */

void Treap11::merge(Treap11& left, Treap11& right)
{
    QElapsedTimer timer;
    timer.start();

    m_nodes = left.m_nodes;
    // Ensure right's nodes are also available
    for (int i = 0; i < right.m_nodes.size(); ++i) {
        if (i >= m_nodes.size())
            m_nodes.append(right.m_nodes[i]);
    }

    m_root = mergeInternal(left.m_root, right.m_root);
    m_stats.numNodes = getSize(m_root);
    m_stats.numMerges++;
    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    left.m_root = -1;
    left.m_stats.numNodes = 0;
    right.m_root = -1;
    right.m_stats.numNodes = 0;

    emit treeUpdated(m_stats.numNodes, m_stats.treeHeight, timer.elapsed());
}

/* ---- Public: Inorder keys ---- */

QVector<int> Treap11::inorderKeys() const
{
    QVector<int> keys;
    inorderHelper(m_root, keys);
    return keys;
}

/* ---- Public: Height ---- */

int Treap11::height() const
{
    return heightHelper(m_root);
}

/* ---- Public: Reset ---- */

void Treap11::resetStatistics()
{
    m_nodes.clear();
    m_root = -1;
    m_freeList = -1;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

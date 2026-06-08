/**
 * @file BTree6.cpp
 * @brief BTree6 实现
 *
 * 实现B树：有序批量加载与缓冲树延迟刷写批量插入。
 */

#include "utils/tree231/BTree6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BTree6::BTree6(QObject *parent) : QObject(parent) {}
BTree6::~BTree6() = default;

/* ---- Configuration ---- */

void BTree6::setParameters(int minDegree, int bufferCapacity)
{
    m_minDegree = qMax(2, minDegree);
    m_bufferCapacity = qMax(16, bufferCapacity);
}

/* ---- Allocate / Free node ---- */

int BTree6::allocNode()
{
    int idx;
    if (m_freeList >= 0) {
        idx = m_freeList;
        m_freeList = m_nodes[idx].children.isEmpty() ? -1 : m_nodes[idx].children[0];
    } else {
        idx = m_nodes.size();
        m_nodes.append(Node());
    }
    m_nodes[idx] = Node();
    m_nodes[idx].keys.reserve(2 * m_minDegree - 1);
    m_nodes[idx].values.reserve(2 * m_minDegree - 1);
    m_nodes[idx].children.reserve(2 * m_minDegree);
    return idx;
}

void BTree6::freeNode(int idx)
{
    m_nodes[idx].keys.clear();
    m_nodes[idx].values.clear();
    m_nodes[idx].children.resize(1);
    m_nodes[idx].children[0] = m_freeList;
    m_nodes[idx].parent = -1;
    m_nodes[idx].isLeaf = true;
    m_nodes[idx].buffer.clear();
    m_freeList = idx;
}

/* ---- Split full child ---- */

void BTree6::splitChild(int parentIdx, int childPos)
{
    int childIdx = m_nodes[parentIdx].children[childPos];
    int newIdx = allocNode();
    int t = m_minDegree;

    Node& child = m_nodes[childIdx];
    Node& newNode = m_nodes[newIdx];
    Node& parent = m_nodes[parentIdx];

    newNode.isLeaf = child.isLeaf;
    newNode.parent = parentIdx;

    // Copy upper half of keys/values to new node
    for (int i = 0; i < t - 1; ++i) {
        newNode.keys.append(child.keys[i + t]);
        newNode.values.append(child.values[i + t]);
    }

    // Copy upper half of children
    if (!child.isLeaf) {
        for (int i = 0; i < t; ++i) {
            newNode.children.append(child.children[i + t]);
            m_nodes[child.children[i + t]].parent = newIdx;
        }
    }

    // Promote median key to parent
    parent.keys.insert(childPos, child.keys[t - 1]);
    parent.values.insert(childPos, child.values[t - 1]);
    parent.children.insert(childPos + 1, newIdx);

    // Truncate child to lower half
    child.keys.resize(t - 1);
    child.values.resize(t - 1);
    if (!child.isLeaf) child.children.resize(t);
}

/* ---- Insert into non-full node ---- */

void BTree6::insertNonFull(int nodeIdx, double key, double value)
{
    Node& node = m_nodes[nodeIdx];
    int i = node.keys.size() - 1;

    if (node.isLeaf) {
        // Find position and insert
        while (i >= 0 && node.keys[i] > key) --i;
        node.keys.insert(i + 1, key);
        node.values.insert(i + 1, value);
    } else {
        while (i >= 0 && node.keys[i] > key) --i;
        ++i;

        if (m_nodes[node.children[i]].keys.size() == 2 * m_minDegree - 1) {
            splitChild(nodeIdx, i);
            if (key > node.keys[i]) ++i;
        }
        insertNonFull(node.children[i], key, value);
    }
}

/* ---- Flush buffer downward ---- */

void BTree6::flushNodeBuffer(int nodeIdx)
{
    Node& node = m_nodes[nodeIdx];
    if (node.buffer.size() < m_bufferCapacity) return;

    QVector<QPair<double, double>> buf = node.buffer;
    node.buffer.clear();

    for (const auto& kv : buf) {
        // Find child to descend into
        if (node.isLeaf) {
            // Insert directly
            int i = node.keys.size() - 1;
            while (i >= 0 && node.keys[i] > kv.first) --i;
            node.keys.insert(i + 1, kv.first);
            node.values.insert(i + 1, kv.second);
        } else {
            int i = node.keys.size() - 1;
            while (i >= 0 && node.keys[i] > kv.first) --i;
            ++i;
            m_nodes[node.children[i]].buffer.append(kv);

            // Recursively flush if child buffer full
            if (m_nodes[node.children[i]].buffer.size() >= m_bufferCapacity)
                flushNodeBuffer(node.children[i]);
        }
    }

    m_stats.numFlushes++;
    m_stats.bufferSize = 0;
    for (const auto& n : m_nodes)
        m_stats.bufferSize += n.buffer.size();
}

/* ---- Search in subtree ---- */

double BTree6::searchIn(int nodeIdx, double key) const
{
    if (nodeIdx < 0) return -1.0;
    const Node& node = m_nodes[nodeIdx];

    int i = 0;
    while (i < node.keys.size() && key > node.keys[i]) ++i;

    if (i < node.keys.size() && qFuzzyCompare(node.keys[i], key))
        return node.values[i];

    if (node.isLeaf) return -1.0;
    return searchIn(node.children[i], key);
}

/* ---- Range query in subtree ---- */

void BTree6::rangeIn(int nodeIdx, double lo, double hi,
                       QVector<QPair<double, double>>& result) const
{
    if (nodeIdx < 0) return;
    const Node& node = m_nodes[nodeIdx];

    int i = 0;
    while (i < node.keys.size() && node.keys[i] < lo) ++i;

    while (i < node.keys.size() && node.keys[i] <= hi) {
        if (!node.isLeaf)
            rangeIn(node.children[i], lo, hi, result);
        result.append(qMakePair(node.keys[i], node.values[i]));
        ++i;
    }

    if (!node.isLeaf && i < node.children.size())
        rangeIn(node.children[i], lo, hi, result);
}

/* ---- Remove from subtree ---- */

bool BTree6::removeFrom(int nodeIdx, double key)
{
    if (nodeIdx < 0) return false;
    Node& node = m_nodes[nodeIdx];
    int t = m_minDegree;

    int idx = 0;
    while (idx < node.keys.size() && node.keys[idx] < key) ++idx;

    if (idx < node.keys.size() && qFuzzyCompare(node.keys[idx], key)) {
        if (node.isLeaf) {
            node.keys.removeAt(idx);
            node.values.removeAt(idx);
        } else {
            if (m_nodes[node.children[idx]].keys.size() >= t) {
                auto pred = predecessor(nodeIdx, idx);
                node.keys[idx] = pred.first;
                node.values[idx] = pred.second;
                removeFrom(node.children[idx], pred.first);
            } else if (m_nodes[node.children[idx + 1]].keys.size() >= t) {
                auto succ = successor(nodeIdx, idx);
                node.keys[idx] = succ.first;
                node.values[idx] = succ.second;
                removeFrom(node.children[idx + 1], succ.first);
            } else {
                mergeChildren(nodeIdx, idx);
                removeFrom(node.children[idx], key);
            }
        }
        return true;
    }

    if (node.isLeaf) return false;

    bool lastChild = (idx == node.keys.size());
    if (m_nodes[node.children[idx]].keys.size() < t)
        fillChild(nodeIdx, idx);

    int nextIdx = (lastChild && idx > node.keys.size()) ? idx - 1 : idx;
    return removeFrom(node.children[nextIdx], key);
}

/* ---- Predecessor ---- */

QPair<double, double> BTree6::predecessor(int nodeIdx, int keyIdx) const
{
    int cur = m_nodes[nodeIdx].children[keyIdx];
    while (!m_nodes[cur].isLeaf)
        cur = m_nodes[cur].children.last();
    return qMakePair(m_nodes[cur].keys.last(), m_nodes[cur].values.last());
}

/* ---- Successor ---- */

QPair<double, double> BTree6::successor(int nodeIdx, int keyIdx) const
{
    int cur = m_nodes[nodeIdx].children[keyIdx + 1];
    while (!m_nodes[cur].isLeaf)
        cur = m_nodes[cur].children.first();
    return qMakePair(m_nodes[cur].keys.first(), m_nodes[cur].values.first());
}

/* ---- Fill child ---- */

void BTree6::fillChild(int nodeIdx, int childPos)
{
    if (childPos > 0 && m_nodes[m_nodes[nodeIdx].children[childPos - 1]].keys.size() >= m_minDegree)
        borrowFromLeft(nodeIdx, childPos);
    else if (childPos < m_nodes[nodeIdx].keys.size() &&
             m_nodes[m_nodes[nodeIdx].children[childPos + 1]].keys.size() >= m_minDegree)
        borrowFromRight(nodeIdx, childPos);
    else
        mergeChildren(nodeIdx, qMin(childPos, m_nodes[nodeIdx].keys.size() - 1));
}

/* ---- Borrow from left ---- */

void BTree6::borrowFromLeft(int nodeIdx, int childPos)
{
    Node& parent = m_nodes[nodeIdx];
    int childIdx = parent.children[childPos];
    int sibIdx = parent.children[childPos - 1];

    m_nodes[childIdx].keys.prepend(parent.keys[childPos - 1]);
    m_nodes[childIdx].values.prepend(parent.values[childPos - 1]);
    parent.keys[childPos - 1] = m_nodes[sibIdx].keys.last();
    parent.values[childPos - 1] = m_nodes[sibIdx].values.last();
    m_nodes[sibIdx].keys.removeLast();
    m_nodes[sibIdx].values.removeLast();

    if (!m_nodes[sibIdx].isLeaf) {
        m_nodes[childIdx].children.prepend(m_nodes[sibIdx].children.last());
        m_nodes[sibIdx].children.removeLast();
    }
}

/* ---- Borrow from right ---- */

void BTree6::borrowFromRight(int nodeIdx, int childPos)
{
    Node& parent = m_nodes[nodeIdx];
    int childIdx = parent.children[childPos];
    int sibIdx = parent.children[childPos + 1];

    m_nodes[childIdx].keys.append(parent.keys[childPos]);
    m_nodes[childIdx].values.append(parent.values[childPos]);
    parent.keys[childPos] = m_nodes[sibIdx].keys.first();
    parent.values[childPos] = m_nodes[sibIdx].values.first();
    m_nodes[sibIdx].keys.removeFirst();
    m_nodes[sibIdx].values.removeFirst();

    if (!m_nodes[sibIdx].isLeaf) {
        m_nodes[childIdx].children.append(m_nodes[sibIdx].children.first());
        m_nodes[sibIdx].children.removeFirst();
    }
}

/* ---- Merge children ---- */

void BTree6::mergeChildren(int nodeIdx, int childPos)
{
    Node& parent = m_nodes[nodeIdx];
    int leftIdx = parent.children[childPos];
    int rightIdx = parent.children[childPos + 1];

    m_nodes[leftIdx].keys.append(parent.keys[childPos]);
    m_nodes[leftIdx].values.append(parent.values[childPos]);

    for (int i = 0; i < m_nodes[rightIdx].keys.size(); ++i) {
        m_nodes[leftIdx].keys.append(m_nodes[rightIdx].keys[i]);
        m_nodes[leftIdx].values.append(m_nodes[rightIdx].values[i]);
    }
    for (int c : m_nodes[rightIdx].children)
        m_nodes[leftIdx].children.append(c);

    parent.keys.removeAt(childPos);
    parent.values.removeAt(childPos);
    parent.children.removeAt(childPos + 1);
    freeNode(rightIdx);
}

/* ---- Build from sorted ---- */

int BTree6::buildFromSorted(const QVector<double>& keys,
                              const QVector<double>& values,
                              int start, int end)
{
    int maxKeys = 2 * m_minDegree - 1;
    int n = end - start;

    if (n <= maxKeys) {
        // Leaf node
        int idx = allocNode();
        m_nodes[idx].isLeaf = true;
        for (int i = start; i < end; ++i) {
            m_nodes[idx].keys.append(keys[i]);
            m_nodes[idx].values.append(values[i]);
        }
        return idx;
    }

    // Internal node: split into children
    int idx = allocNode();
    m_nodes[idx].isLeaf = false;

    int numChildren = qMax(2, (n + maxKeys - 1) / maxKeys);
    int perChild = n / numChildren;

    int pos = start;
    for (int c = 0; c < numChildren; ++c) {
        int childStart = pos;
        int childEnd = qMin(pos + perChild, end);

        // One key promoted as separator
        if (c < numChildren - 1) {
            childEnd = qMin(childEnd, end - 1);
            m_nodes[idx].keys.append(keys[childEnd]);
            m_nodes[idx].values.append(values[childEnd]);
            childEnd++;
        }

        int childIdx = buildFromSorted(keys, values, childStart, childEnd);
        m_nodes[idx].children.append(childIdx);
        m_nodes[childIdx].parent = idx;
        pos = childEnd;
    }

    return idx;
}

/* ---- Bulk load ---- */

bool BTree6::bulkLoad(const QVector<double>& sortedKeys,
                        const QVector<double>& sortedValues)
{
    QElapsedTimer timer;
    timer.start();

    int n = sortedKeys.size();
    if (n != sortedValues.size() || n == 0) return false;

    m_nodes.clear();
    m_freeList = -1;
    m_root = buildFromSorted(sortedKeys, sortedValues, 0, n);

    m_stats.numKeys = n;
    m_stats.numNodes = m_nodes.size();
    m_stats.treeHeight = 0;
    int cur = m_root;
    while (cur >= 0 && !m_nodes[cur].isLeaf) {
        m_stats.treeHeight++;
        cur = m_nodes[cur].children[0];
    }
    m_stats.treeHeight++;

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit bulkLoadCompleted(n, m_stats.treeHeight, timer.elapsed());
    return true;
}

/* ---- Insert (with buffering) ---- */

void BTree6::insert(double key, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root < 0) {
        m_root = allocNode();
        m_nodes[m_root].isLeaf = true;
        m_nodes[m_root].keys.append(key);
        m_nodes[m_root].values.append(value);
    } else {
        // Buffer the insertion at root
        m_nodes[m_root].buffer.append(qMakePair(key, value));

        // Check if buffer needs flushing
        if (m_nodes[m_root].buffer.size() >= m_bufferCapacity) {
            // If root is full, increase height first
            if (m_nodes[m_root].keys.size() == 2 * m_minDegree - 1) {
                int newRoot = allocNode();
                m_nodes[newRoot].isLeaf = false;
                m_nodes[newRoot].children.append(m_root);
                m_nodes[m_root].parent = newRoot;
                m_root = newRoot;
                splitChild(m_root, 0);
            }
            flushNodeBuffer(m_root);
        }
    }

    m_stats.numKeys++;
    m_stats.numNodes = m_nodes.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Search ---- */

double BTree6::search(double key) const
{
    // Check buffers along the path first
    int cur = m_root;
    while (cur >= 0) {
        for (const auto& kv : m_nodes[cur].buffer) {
            if (qFuzzyCompare(kv.first, key))
                return kv.second;
        }
        cur = -1;  // Fallback to tree search
    }
    return searchIn(m_root, key);
}

/* ---- Remove ---- */

bool BTree6::remove(double key)
{
    return removeFrom(m_root, key);
}

/* ---- Range query ---- */

QVector<QPair<double, double>> BTree6::rangeQuery(double lo, double hi) const
{
    QVector<QPair<double, double>> result;
    rangeIn(m_root, lo, hi, result);
    return result;
}

/* ---- Flush all buffers ---- */

void BTree6::flushBuffers()
{
    // Flush buffers from root downward
    if (m_root >= 0) flushNodeBuffer(m_root);
}

/* ---- Reset ---- */

void BTree6::resetStatistics()
{
    m_nodes.clear();
    m_root = -1;
    m_freeList = -1;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

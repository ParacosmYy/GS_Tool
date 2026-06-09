/**
 * @file BTree8.cpp
 * @brief BTree8 实现
 *
 * 实现B树：写优化缓冲树与分数级联批量插入范围查询。
 */

#include "utils/tree259/BTree8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

BTree8::BTree8(int order, QObject *parent)
    : QObject(parent), m_order(qMax(4, order))
{
    m_root = allocNode(true);
    m_height = 1;
}

BTree8::~BTree8() = default;

/* ---- Configuration ---- */

void BTree8::setOrder(int order) { m_order = qMax(4, order); }
void BTree8::setBufferSize(int size) { m_bufferSize = qMax(16, size); }

/* ---- Node allocation ---- */

int BTree8::allocNode(bool isLeaf)
{
    int idx;
    if (m_freeList >= 0) {
        idx = m_freeList;
        m_freeList = m_nodes[idx].children.isEmpty() ? -1 : m_nodes[idx].children[0];
    } else {
        idx = m_nodes.size();
        m_nodes.append(Node());
    }
    m_nodes[idx] = Node{};
    m_nodes[idx].isLeaf = isLeaf;
    return idx;
}

void BTree8::freeNode(int idx)
{
    m_nodes[idx].keys.clear();
    m_nodes[idx].values.clear();
    m_nodes[idx].children.resize(1);
    m_nodes[idx].children[0] = m_freeList;
    m_nodes[idx].buffer.clear();
    m_freeList = idx;
}

/* ---- Find leaf for key ---- */

int BTree8::findLeaf(double key) const
{
    int idx = m_root;
    while (idx >= 0 && !m_nodes[idx].isLeaf) {
        const Node& node = m_nodes[idx];
        int childIdx = 0;
        for (int i = 0; i < node.keys.size(); ++i) {
            if (key < node.keys[i]) break;
            childIdx = i + 1;
        }
        if (childIdx < node.children.size())
            idx = node.children[childIdx];
        else
            break;
    }
    return idx;
}

/* ---- Key index via binary search ---- */

int BTree8::keyIndex(int nodeIdx, double key) const
{
    const auto& keys = m_nodes[nodeIdx].keys;
    auto it = std::lower_bound(keys.begin(), keys.end(), key);
    return static_cast<int>(it - keys.begin());
}

/* ---- Insert directly into sorted node ---- */

void BTree8::insertDirect(int nodeIdx, double key, double value)
{
    Node& node = m_nodes[nodeIdx];
    int pos = keyIndex(nodeIdx, key);
    node.keys.insert(pos, key);
    node.values.insert(pos, value);
}

/* ---- Split an overflowing node ---- */

int BTree8::splitNode(int idx)
{
    Node& node = m_nodes[idx];
    int mid = node.keys.size() / 2;
    double midKey = node.keys[mid];

    bool leaf = node.isLeaf;
    int newNodeIdx = allocNode(leaf);
    Node& newNode = m_nodes[newNodeIdx];

    // Move upper half to new node
    int start = leaf ? mid : mid + 1;
    for (int i = start; i < node.keys.size(); ++i) {
        newNode.keys.append(node.keys[i]);
        newNode.values.append(node.values[i]);
    }
    if (!leaf) {
        for (int i = mid + 1; i < node.children.size(); ++i) {
            newNode.children.append(node.children[i]);
            if (node.children[i] >= 0)
                m_nodes[node.children[i]].parent = newNodeIdx;
        }
    }

    // Trim original node
    int removeCount = node.keys.size() - start;
    for (int i = 0; i < removeCount; ++i) {
        node.keys.removeLast();
        node.values.removeLast();
    }
    if (!leaf) {
        while (node.children.size() > mid + 1)
            node.children.removeLast();
    }

    // Promote middle key to parent
    if (idx == m_root) {
        int newRoot = allocNode(false);
        m_nodes[newRoot].keys.append(midKey);
        m_nodes[newRoot].values.append(node.values[mid]);
        m_nodes[newRoot].children.append(idx);
        m_nodes[newRoot].children.append(newNodeIdx);
        m_nodes[idx].parent = newRoot;
        m_nodes[newNodeIdx].parent = newRoot;
        m_root = newRoot;
        m_height++;
        if (!leaf) { node.keys.removeAt(mid); node.values.removeAt(mid); }
    } else {
        int parent = node.parent;
        Node& pNode = m_nodes[parent];
        int pos = keyIndex(parent, midKey);
        pNode.keys.insert(pos, midKey);
        pNode.values.insert(pos, node.values[mid]);
        pNode.children.insert(pos + 1, newNodeIdx);
        m_nodes[newNodeIdx].parent = parent;
        if (!leaf) { node.keys.removeAt(mid); node.values.removeAt(mid); }

        // Check if parent overflows
        if (pNode.keys.size() >= m_order - 1)
            splitNode(parent);
    }

    return newNodeIdx;
}

/* ---- Insert into buffer ---- */

void BTree8::insertBuffer(int nodeIdx, double key, double value)
{
    Node& node = m_nodes[nodeIdx];
    if (node.isLeaf || node.buffer.size() < m_bufferSize) {
        node.buffer.append({key, value});
        if (node.isLeaf || node.buffer.size() >= m_bufferSize)
            flushNodeBuffer(nodeIdx);
    }
}

/* ---- Flush buffer to children ---- */

void BTree8::flushNodeBuffer(int nodeIdx)
{
    Node& node = m_nodes[nodeIdx];
    if (node.buffer.isEmpty()) return;

    if (node.isLeaf) {
        // Merge buffer into leaf sorted keys
        for (const auto& kv : node.buffer) {
            insertDirect(nodeIdx, kv.first, kv.second);
        }
        node.buffer.clear();

        // Split if overflow
        if (node.keys.size() >= m_order)
            splitNode(nodeIdx);
    } else {
        // Route buffer entries to children
        QVector<QVector<QPair<double, double>>> childBuf(node.children.size());
        for (const auto& kv : node.buffer) {
            int ci = 0;
            for (int i = 0; i < node.keys.size(); ++i) {
                if (kv.first >= node.keys[i]) ci = i + 1;
            }
            childBuf[ci].append(kv);
        }
        node.buffer.clear();

        for (int c = 0; c < node.children.size(); ++c) {
            for (const auto& kv : childBuf[c])
                insertBuffer(node.children[c], kv.first, kv.second);
        }
    }
}

/* ---- Single insert ---- */

void BTree8::insert(double key, double value)
{
    QElapsedTimer timer;
    timer.start();

    insertBuffer(m_root, key, value);

    double elapsed = timer.elapsed();
    m_stats.numKeys++;
    m_stats.numNodes = m_nodes.size();
    m_stats.treeHeight = m_height;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit insertCompleted(1, m_height, elapsed);
}

/* ---- Batch insert ---- */

void BTree8::batchInsert(const QVector<double>& keys, const QVector<double>& values)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(keys.size(), values.size());
    for (int i = 0; i < n; ++i)
        insertBuffer(m_root, keys[i], values[i]);

    double elapsed = timer.elapsed();
    m_stats.numKeys += n;
    m_stats.numNodes = m_nodes.size();
    m_stats.treeHeight = m_height;
    m_stats.bufferSize = m_bufferSize;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit insertCompleted(n, m_height, elapsed);
}

/* ---- Exact search ---- */

double BTree8::search(double key) const
{
    int idx = m_root;
    while (idx >= 0) {
        const Node& node = m_nodes[idx];
        int pos = keyIndex(idx, key);
        if (pos < node.keys.size() && qFuzzyCompare(node.keys[pos], key))
            return node.values[pos];
        // Check buffer
        for (const auto& kv : node.buffer) {
            if (qFuzzyCompare(kv.first, key)) return kv.second;
        }
        if (node.isLeaf) break;
        if (pos < node.children.size())
            idx = node.children[pos];
        else break;
    }
    return qQNaN();
}

/* ---- Range query helper with fractional cascading ---- */

void BTree8::rangeHelper(int nodeIdx, double lo, double hi,
                          QVector<double>& outKeys,
                          QVector<double>& outValues) const
{
    if (nodeIdx < 0) return;
    const Node& node = m_nodes[nodeIdx];

    if (node.isLeaf) {
        // Fractional cascading: start from lower_bound position
        int start = static_cast<int>(
            std::lower_bound(node.keys.begin(), node.keys.end(), lo)
            - node.keys.begin());
        for (int i = start; i < node.keys.size() && node.keys[i] <= hi; ++i) {
            outKeys.append(node.keys[i]);
            outValues.append(node.values[i]);
        }
        // Also check buffer
        for (const auto& kv : node.buffer) {
            if (kv.first >= lo && kv.first <= hi) {
                outKeys.append(kv.first);
                outValues.append(kv.second);
            }
        }
    } else {
        // Use key positions as fractional cascading hints
        int loPos = static_cast<int>(
            std::lower_bound(node.keys.begin(), node.keys.end(), lo)
            - node.keys.begin());
        int hiPos = static_cast<int>(
            std::upper_bound(node.keys.begin(), node.keys.end(), hi)
            - node.keys.begin());

        // Check buffer entries in range
        for (const auto& kv : node.buffer) {
            if (kv.first >= lo && kv.first <= hi) {
                outKeys.append(kv.first);
                outValues.append(kv.second);
            }
        }

        // Recurse into relevant children using cascading positions
        for (int c = loPos; c <= hiPos && c < node.children.size(); ++c) {
            rangeHelper(node.children[c], lo, hi, outKeys, outValues);
        }
    }
}

/* ---- Range query ---- */

void BTree8::rangeQuery(double lo, double hi, QVector<double>& outKeys,
                          QVector<double>& outValues) const
{
    QElapsedTimer timer;
    timer.start();

    outKeys.clear();
    outValues.clear();
    rangeHelper(m_root, lo, hi, outKeys, outValues);

    double elapsed = timer.elapsed();
    const_cast<BTree8*>(this)->m_stats.totalOps++;
    const_cast<BTree8*>(this)->m_timeSum += elapsed;
    const_cast<BTree8*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    emit const_cast<BTree8*>(this)->rangeCompleted(outKeys.size(), elapsed);
}

/* ---- Remove ---- */

bool BTree8::remove(double key)
{
    int idx = m_root;
    while (idx >= 0) {
        Node& node = m_nodes[idx];
        int pos = keyIndex(idx, key);
        if (pos < node.keys.size() && qFuzzyCompare(node.keys[pos], key)) {
            node.keys.removeAt(pos);
            node.values.removeAt(pos);
            m_stats.numKeys--;
            return true;
        }
        // Try buffer
        for (int i = 0; i < node.buffer.size(); ++i) {
            if (qFuzzyCompare(node.buffer[i].first, key)) {
                node.buffer.removeAt(i);
                m_stats.numKeys--;
                return true;
            }
        }
        if (node.isLeaf) break;
        if (pos < node.children.size())
            idx = node.children[pos];
        else break;
    }
    return false;
}

/* ---- Flush all buffers ---- */

void BTree8::flushBuffers()
{
    // BFS flush from root to leaves
    QVector<int> queue;
    queue.append(m_root);
    while (!queue.isEmpty()) {
        int idx = queue.takeFirst();
        if (idx < 0) continue;
        flushNodeBuffer(idx);
        if (!m_nodes[idx].isLeaf) {
            for (int c : m_nodes[idx].children)
                if (c >= 0) queue.append(c);
        }
    }
}

/* ---- Height ---- */

int BTree8::height() const { return m_height; }

/* ---- Reset ---- */

void BTree8::resetStatistics()
{
    m_nodes.clear();
    m_freeList = -1;
    m_root = allocNode(true);
    m_height = 1;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

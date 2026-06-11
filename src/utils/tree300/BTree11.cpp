/**
 * @file BTree11.cpp
 * @brief BTree11 实现
 *
 * 实现B树：写优化缓冲树与延迟刷新实现摊还高效批量插入及范围查询支持。
 */

#include "utils/tree300/BTree11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BTree11::BTree11(QObject *parent)
    : QObject(parent) {}

BTree11::~BTree11() = default;

/* ---- Configuration ---- */

void BTree11::setOrder(int t) { m_t = qBound(2, t, 256); }
void BTree11::setBufferSize(int size) { m_maxBufferSize = qBound(4, size, 4096); }

/* ---- Allocate a new node ---- */

int BTree11::allocNode(bool isLeaf)
{
    Node n;
    n.isLeaf = isLeaf;
    n.parent = -1;
    n.bufferSize = 0;
    m_nodes.append(n);
    return m_nodes.size() - 1;
}

/* ---- Check if buffer needs flushing ---- */

bool BTree11::needsFlush(int nodeIdx) const
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return false;
    return m_nodes[nodeIdx].buffer.size() >= m_maxBufferSize;
}

/* ---- Flush buffer of a node down the tree ---- */

void BTree11::flushBuffer(int nodeIdx)
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return;
    auto& node = m_nodes[nodeIdx];

    if (node.buffer.isEmpty()) return;

    m_stats.totalFlushes++;

    if (node.isLeaf) {
        // Leaf: merge buffer into sorted keys
        for (double k : node.buffer) {
            // Binary search insert position
            int pos = 0;
            while (pos < node.keys.size() && node.keys[pos] < k) ++pos;
            if (pos < node.keys.size() && node.keys[pos] == k) continue; // duplicate
            node.keys.insert(pos, k);
        }
        node.buffer.clear();
        node.bufferSize = 0;
        return;
    }

    // Internal node: route buffer entries to appropriate children
    for (double k : node.buffer) {
        // Find child: first key position where keys[i] > k
        int childIdx = 0;
        while (childIdx < node.keys.size() && node.keys[childIdx] <= k)
            ++childIdx;

        int childNode = node.children[childIdx];
        if (childNode < 0) {
            // Create new child
            childNode = allocNode(true);
            node.children[childIdx] = childNode;
            m_nodes[childNode].parent = nodeIdx;
        }
        m_nodes[childNode].buffer.append(k);
        m_nodes[childNode].bufferSize++;
    }

    node.buffer.clear();
    node.bufferSize = 0;

    // Recursively flush children that exceed buffer threshold
    for (int c : node.children) {
        if (c >= 0 && needsFlush(c))
            flushBuffer(c);
    }
}

/* ---- Split a full child node ---- */

void BTree11::splitChild(int parentIdx, int childPos)
{
    auto& parent = m_nodes[parentIdx];
    int childIdx = parent.children[childPos];
    auto& child = m_nodes[childIdx];

    int mid = child.keys.size() / 2;
    double midKey = child.keys[mid];

    // Create new sibling node
    int sibIdx = allocNode(child.isLeaf);
    auto& sib = m_nodes[sibIdx];
    sib.parent = parentIdx;

    // Move upper half of keys to sibling
    for (int i = mid + 1; i < child.keys.size(); ++i)
        sib.keys.append(child.keys[i]);

    // Move upper half of children to sibling
    if (!child.isLeaf) {
        for (int i = mid + 1; i < child.children.size(); ++i) {
            sib.children.append(child.children[i]);
            if (child.children[i] >= 0)
                m_nodes[child.children[i]].parent = sibIdx;
        }
        child.children.resize(mid + 1);
    }

    // Trim child to lower half
    child.keys.resize(mid);

    // Insert middle key into parent
    parent.keys.insert(childPos, midKey);
    parent.children.insert(childPos + 1, sibIdx);
}

/* ---- Insert into a non-full node ---- */

void BTree11::insertNonFull(int nodeIdx, double key)
{
    auto& node = m_nodes[nodeIdx];

    if (node.isLeaf) {
        // Binary search insert position
        int pos = 0;
        while (pos < node.keys.size() && node.keys[pos] < key) ++pos;
        if (pos < node.keys.size() && node.keys[pos] == key) return; // duplicate
        node.keys.insert(pos, key);
        return;
    }

    // Find child
    int i = 0;
    while (i < node.keys.size() && node.keys[i] <= key) ++i;

    int childIdx = node.children[i];
    if (childIdx < 0) {
        childIdx = allocNode(true);
        node.children[i] = childIdx;
        m_nodes[childIdx].parent = nodeIdx;
    }

    // Split if child is full
    if (m_nodes[childIdx].keys.size() >= 2 * m_t - 1) {
        splitChild(nodeIdx, i);
        if (key > node.keys[i]) ++i;
        childIdx = node.children[i];
    }

    insertNonFull(childIdx, key);
}

/* ---- Insert (with buffer tree optimization) ---- */

bool BTree11::insert(double key)
{
    QElapsedTimer timer;
    timer.start();

    // Buffer-tree approach: add to root buffer first
    if (m_root < 0) {
        m_root = allocNode(true);
    }

    auto& root = m_nodes[m_root];
    root.buffer.append(key);
    root.bufferSize++;

    // Lazy flush: only flush when buffer exceeds threshold
    if (needsFlush(m_root)) {
        // Ensure root has room before flushing
        if (root.keys.size() >= 2 * m_t - 1) {
            // Create new root
            int newRoot = allocNode(false);
            m_nodes[newRoot].children.append(m_root);
            root.parent = newRoot;
            splitChild(newRoot, 0);
            m_root = newRoot;
        }
        flushBuffer(m_root);
    }

    m_stats.totalInserts++;
    m_stats.treeSize++;
    m_stats.bufferSize = 0;
    for (const auto& n : m_nodes)
        m_stats.bufferSize += n.buffer.size();

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalInserts;

    emit insertDone(key, m_stats.treeSize, elapsed);
    return true;
}

/* ---- Search helper ---- */

BTree11::SearchResult BTree11::searchHelper(int nodeIdx, double key, int depth) const
{
    SearchResult result;
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return result;

    const auto& node = m_nodes[nodeIdx];

    // Check buffer first (unsorted pending keys)
    for (double b : node.buffer) {
        if (b == key) {
            result.found = true;
            result.key = key;
            result.nodeIndex = nodeIdx;
            result.depth = depth;
            return result;
        }
    }

    // Binary search in sorted keys
    int lo = 0, hi = node.keys.size() - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (node.keys[mid] == key) {
            result.found = true;
            result.key = key;
            result.nodeIndex = nodeIdx;
            result.keyIndex = mid;
            result.depth = depth;
            return result;
        }
        if (node.keys[mid] < key) lo = mid + 1;
        else hi = mid - 1;
    }

    // Recurse into child
    if (!node.isLeaf && lo < node.children.size()) {
        int child = node.children[lo];
        if (child >= 0)
            return searchHelper(child, key, depth + 1);
    }

    return result;
}

/* ---- Search (triggers flush if needed) ---- */

BTree11::SearchResult BTree11::search(double key)
{
    // For correctness, flush buffers before search
    // (lazy flush means buffers may contain keys not yet in tree)
    // Only flush if we have buffered items
    if (m_root >= 0 && m_nodes[m_root].buffer.size() > 0) {
        flushAll();
    }

    auto result = searchHelper(m_root, key, 0);
    m_stats.totalSearches++;
    return result;
}

/* ---- Flush all pending buffers ---- */

void BTree11::flushAll()
{
    QElapsedTimer timer;
    timer.start();

    int totalFlushed = 0;

    // Flush all nodes top-down (BFS)
    if (m_root >= 0) {
        QVector<int> queue;
        queue.append(m_root);

        for (int qi = 0; qi < queue.size(); ++qi) {
            int idx = queue[qi];
            if (idx < 0) continue;

            // Ensure root has room
            if (idx == m_root && m_nodes[idx].keys.size() >= 2 * m_t - 1) {
                int newRoot = allocNode(false);
                m_nodes[newRoot].children.append(m_root);
                m_nodes[m_root].parent = newRoot;
                splitChild(newRoot, 0);
                m_root = newRoot;
            }

            totalFlushed += m_nodes[idx].buffer.size();
            flushBuffer(idx);

            for (int c : m_nodes[idx].children)
                if (c >= 0) queue.append(c);
        }
    }

    // Compute tree height
    m_stats.treeHeight = 0;
    int cur = m_root;
    while (cur >= 0) {
        m_stats.treeHeight++;
        if (m_nodes[cur].children.size() > 0 && m_nodes[cur].children[0] >= 0)
            cur = m_nodes[cur].children[0];
        else break;
    }

    m_stats.bufferSize = 0;
    double elapsed = timer.elapsed();
    emit flushDone(totalFlushed, elapsed);
}

/* ---- Range query ---- */

void BTree11::rangeHelper(int idx, double lo, double hi, QVector<double>& result) const
{
    if (idx < 0 || idx >= m_nodes.size()) return;
    const auto& node = m_nodes[idx];

    int i = 0;
    while (i < node.keys.size() && node.keys[i] < lo) ++i;

    // Collect keys in range
    for (; i < node.keys.size() && node.keys[i] <= hi; ++i) {
        result.append(node.keys[i]);
        if (!node.isLeaf && i < node.children.size())
            rangeHelper(node.children[i], lo, hi, result);
    }

    // Check last child
    if (!node.isLeaf && node.children.size() > i)
        rangeHelper(node.children[i], lo, hi, result);

    // Also check buffer
    for (double k : node.buffer)
        if (k >= lo && k <= hi) result.append(k);
}

BTree11::RangeResult BTree11::rangeQuery(double lo, double hi) const
{
    RangeResult result;
    rangeHelper(m_root, lo, hi, result.keys);
    std::sort(result.keys.begin(), result.keys.end());
    result.count = result.keys.size();
    return result;
}

/* ---- In-order traversal ---- */

void BTree11::inOrderHelper(int idx, QVector<double>& result) const
{
    if (idx < 0 || idx >= m_nodes.size()) return;
    const auto& node = m_nodes[idx];

    for (int i = 0; i < node.keys.size(); ++i) {
        if (!node.isLeaf && i < node.children.size())
            inOrderHelper(node.children[i], result);
        result.append(node.keys[i]);
    }
    if (!node.isLeaf && node.children.size() > node.keys.size())
        inOrderHelper(node.children[node.keys.size()], result);

    // Include buffered keys
    for (double k : node.buffer) result.append(k);
}

QVector<double> BTree11::inOrder() const
{
    QVector<double> result;
    inOrderHelper(m_root, result);
    std::sort(result.begin(), result.end());
    return result;
}

/* ---- Reset ---- */

void BTree11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_root = -1;
}

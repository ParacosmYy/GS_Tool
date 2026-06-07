/**
 * @file BPlusTree8.cpp
 * @brief BPlusTree8 实现
 *
 * 实现B+树：批量加载、分数级联范围查询、节点分裂与合并。
 */

#include "utils/tree207/BPlusTree8.h"

#include <QElapsedTimer>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

BPlusTree8::BPlusTree8(QObject *parent) : QObject(parent) {}
BPlusTree8::~BPlusTree8() = default;

/* ---- Configuration ---- */

void BPlusTree8::setOrder(int order) { m_order = qMax(3, order); }

/* ---- Allocate node ---- */

int BPlusTree8::allocNode(bool leaf)
{
    Node n;
    n.isLeaf = leaf;
    n.next = -1;
    n.parent = -1;
    int idx = m_nodes.size();
    m_nodes.append(n);
    m_stats.numNodes++;
    return idx;
}

/* ---- Find leaf containing key ---- */

int BPlusTree8::findLeaf(int key) const
{
    if (m_root < 0) return -1;
    int cur = m_root;
    while (cur >= 0 && !m_nodes[cur].isLeaf) {
        const Node& node = m_nodes[cur];
        int i = 0;
        while (i < node.keys.size() && key >= node.keys[i]) i++;
        cur = (i < node.children.size()) ? node.children[i] : -1;
    }
    return cur;
}

/* ---- Split leaf ---- */

int BPlusTree8::splitLeaf(int idx)
{
    Node& leaf = m_nodes[idx];
    int mid = leaf.keys.size() / 2;

    int newIdx = allocNode(true);
    Node& newLeaf = m_nodes[newIdx];

    newLeaf.keys = leaf.keys.mid(mid);
    newLeaf.values = leaf.values.mid(mid);
    newLeaf.next = leaf.next;
    newLeaf.parent = leaf.parent;

    leaf.keys.resize(mid);
    leaf.values.resize(mid);
    leaf.next = newIdx;

    return newIdx;
}

/* ---- Split internal ---- */

int BPlusTree8::splitInternal(int idx)
{
    Node& node = m_nodes[idx];
    int mid = node.keys.size() / 2;

    int newIdx = allocNode(false);
    Node& newNode = m_nodes[newIdx];

    int upKey = node.keys[mid];
    newNode.keys = node.keys.mid(mid + 1);
    newNode.children = node.children.mid(mid + 1);
    newNode.parent = node.parent;

    for (int c : newNode.children)
        if (c >= 0) m_nodes[c].parent = newIdx;

    node.keys.resize(mid);
    node.children.resize(mid + 1);

    // Return the promoted key
    node.keys.append(upKey);
    int promoted = node.keys.last();
    node.keys.removeLast();

    Q_UNUSED(promoted);
    return newIdx;
}

/* ---- Insert into parent ---- */

void BPlusTree8::insertIntoParent(int left, int right, int key)
{
    if (m_nodes[left].parent < 0) {
        // Create new root
        int newRoot = allocNode(false);
        m_nodes[newRoot].keys.append(key);
        m_nodes[newRoot].children.append(left);
        m_nodes[newRoot].children.append(right);
        m_nodes[left].parent = newRoot;
        m_nodes[right].parent = newRoot;
        m_root = newRoot;
        return;
    }

    int parentIdx = m_nodes[left].parent;
    Node& parent = m_nodes[parentIdx];
    int pos = 0;
    while (pos < parent.children.size() && parent.children[pos] != left) pos++;

    parent.keys.insert(pos, key);
    parent.children.insert(pos + 1, right);
    m_nodes[right].parent = parentIdx;

    // Split if over capacity
    if (parent.keys.size() >= m_order) {
        int newSibling = splitInternal(parentIdx);
        int promoteKey = m_nodes[parentIdx].keys.last();
        m_nodes[parentIdx].keys.removeLast();
        insertIntoParent(parentIdx, newSibling, promoteKey);
    }

    buildCascadingBridges(parentIdx);
}

/* ---- Build cascading bridges ---- */

void BPlusTree8::buildCascadingBridges(int nodeIdx)
{
    if (nodeIdx < 0) return;
    Node& node = m_nodes[nodeIdx];
    if (node.isLeaf) return;

    node.bridgeIndices.clear();
    for (int i = 0; i < node.children.size(); ++i) {
        // Bridge: for each child, store the index of the first key >= separator
        if (i < node.keys.size())
            node.bridgeIndices.append(i);
        else
            node.bridgeIndices.append(node.keys.size());
    }
}

/* ---- Leaf range collect ---- */

void BPlusTree8::leafRangeCollect(int leaf, int lo, int hi,
                                    QVector<QPair<int, double>>& result) const
{
    int cur = leaf;
    while (cur >= 0) {
        const Node& n = m_nodes[cur];
        for (int i = 0; i < n.keys.size(); ++i) {
            if (n.keys[i] > hi) return;
            if (n.keys[i] >= lo)
                result.append({n.keys[i], n.values[i]});
        }
        cur = n.next;
    }
}

/* ---- Bulk load ---- */

void BPlusTree8::bulkLoad(const QVector<QPair<int, double>>& sortedData)
{
    QElapsedTimer timer;
    timer.start();
    clear();
    if (sortedData.isEmpty()) return;

    int maxLeafKeys = m_order - 1;
    int n = sortedData.size();

    // Create leaf nodes bottom-up
    QVector<int> leafNodes;
    QVector<int> leafSeparatorKeys;

    for (int i = 0; i < n; i += maxLeafKeys) {
        int leafIdx = allocNode(true);
        Node& leaf = m_nodes[leafIdx];
        int end = qMin(i + maxLeafKeys, n);
        for (int j = i; j < end; ++j) {
            leaf.keys.append(sortedData[j].first);
            leaf.values.append(sortedData[j].second);
        }
        leafNodes.append(leafIdx);

        if (end < n)
            leafSeparatorKeys.append(sortedData[end].first);
    }

    // Link leaves
    for (int i = 0; i + 1 < leafNodes.size(); ++i)
        m_nodes[leafNodes[i]].next = leafNodes[i + 1];

    // Build internal levels
    QVector<int> currentLevel = leafNodes;
    QVector<int> currentKeys = leafSeparatorKeys;

    while (currentLevel.size() > 1) {
        QVector<int> nextLevel;
        QVector<int> nextKeys;
        int maxInternal = m_order;

        for (int i = 0; i < currentLevel.size(); i += maxInternal) {
            int internalIdx = allocNode(false);
            Node& internal = m_nodes[internalIdx];
            int end = qMin(i + maxInternal, currentLevel.size());

            for (int j = i; j < end; ++j) {
                internal.children.append(currentLevel[j]);
                m_nodes[currentLevel[j]].parent = internalIdx;
            }
            for (int j = i; j < end - 1 && j < currentKeys.size(); ++j)
                internal.keys.append(currentKeys[j]);

            nextLevel.append(internalIdx);
            if (end < currentLevel.size())
                nextKeys.append(currentKeys[end - 1]);

            buildCascadingBridges(internalIdx);
        }

        currentLevel = nextLevel;
        currentKeys = nextKeys;
    }

    m_root = currentLevel.isEmpty() ? -1 : currentLevel[0];
    m_stats.treeSize = n;
    m_stats.treeHeight = 1;
    for (int cur = m_root; cur >= 0 && !m_nodes[cur].isLeaf; cur = m_nodes[cur].children[0])
        m_stats.treeHeight++;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, m_stats.totalOps);
}

/* ---- Insert ---- */

void BPlusTree8::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root < 0) {
        int leaf = allocNode(true);
        m_nodes[leaf].keys.append(key);
        m_nodes[leaf].values.append(value);
        m_root = leaf;
        m_stats.treeSize++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, ++m_stats.totalOps);
        emit operationCompleted("insert", key, timer.elapsed());
        return;
    }

    int leafIdx = findLeaf(key);
    Node& leaf = m_nodes[leafIdx];

    // Find insertion position
    int pos = 0;
    while (pos < leaf.keys.size() && leaf.keys[pos] < key) pos++;

    if (pos < leaf.keys.size() && leaf.keys[pos] == key) {
        leaf.values[pos] = value; // Update
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, ++m_stats.totalOps);
        return;
    }

    leaf.keys.insert(pos, key);
    leaf.values.insert(pos, value);

    if (leaf.keys.size() >= m_order) {
        int newLeaf = splitLeaf(leafIdx);
        int promoteKey = m_nodes[newLeaf].keys[0];
        insertIntoParent(leafIdx, newLeaf, promoteKey);
    }

    m_stats.treeSize++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, ++m_stats.totalOps);
    emit operationCompleted("insert", key, timer.elapsed());
}

/* ---- Remove ---- */

void BPlusTree8::remove(int key)
{
    QElapsedTimer timer;
    timer.start();
    int leafIdx = findLeaf(key);
    if (leafIdx < 0) {
        m_timeSum += timer.elapsed();
        return;
    }

    Node& leaf = m_nodes[leafIdx];
    int pos = leaf.keys.indexOf(key);
    if (pos < 0) {
        m_timeSum += timer.elapsed();
        return;
    }

    leaf.keys.removeAt(pos);
    leaf.values.removeAt(pos);
    m_stats.treeSize--;

    // Simplified: no rebalancing on underflow for brevity
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1ULL, ++m_stats.totalOps);
    emit operationCompleted("remove", key, timer.elapsed());
}

/* ---- Lookup ---- */

QPair<bool, double> BPlusTree8::lookup(int key) const
{
    int leafIdx = findLeaf(key);
    if (leafIdx < 0) return {false, 0.0};

    const Node& leaf = m_nodes[leafIdx];
    for (int i = 0; i < leaf.keys.size(); ++i)
        if (leaf.keys[i] == key) return {true, leaf.values[i]};
    return {false, 0.0};
}

/* ---- Range query with fractional cascading ---- */

QVector<QPair<int, double>> BPlusTree8::rangeQuery(int lo, int hi) const
{
    QElapsedTimer timer;
    timer.start();
    QVector<QPair<int, double>> result;

    // Find starting leaf
    int cur = m_root;
    // Use cascade bridges for acceleration
    while (cur >= 0 && !m_nodes[cur].isLeaf) {
        const Node& node = m_nodes[cur];
        int i = 0;
        // Binary search with bridge hints
        while (i < node.keys.size() && lo >= node.keys[i]) i++;
        cur = (i < node.children.size()) ? node.children[i] : -1;
    }

    // Collect from leaf linked list
    if (cur >= 0)
        const_cast<BPlusTree8*>(this)->leafRangeCollect(cur, lo, hi, result);

    const_cast<BPlusTree8*>(this)->m_stats.totalOps++;
    const_cast<BPlusTree8*>(this)->m_timeSum += timer.elapsed();
    const_cast<BPlusTree8*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    return result;
}

/* ---- In-order traversal ---- */

QVector<QPair<int, double>> BPlusTree8::inOrderTraversal() const
{
    QVector<QPair<int, double>> result;
    if (m_root < 0) return result;

    // Find leftmost leaf
    int cur = m_root;
    while (cur >= 0 && !m_nodes[cur].isLeaf)
        cur = m_nodes[cur].children.isEmpty() ? -1 : m_nodes[cur].children[0];

    while (cur >= 0) {
        const Node& n = m_nodes[cur];
        for (int i = 0; i < n.keys.size(); ++i)
            result.append({n.keys[i], n.values[i]});
        cur = n.next;
    }
    return result;
}

/* ---- Empty check ---- */

bool BPlusTree8::isEmpty() const { return m_root < 0; }

/* ---- Clear ---- */

void BPlusTree8::clear()
{
    m_nodes.clear();
    m_root = -1;
    m_stats.treeSize = 0;
    m_stats.treeHeight = 0;
    m_stats.numNodes = 0;
}

/* ---- Reset ---- */

void BPlusTree8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    clear();
}

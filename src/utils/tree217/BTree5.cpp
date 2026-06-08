/**
 * @file BTree5.cpp
 * @brief BTree5 实现
 *
 * 实现B树：前缀压缩键、批量加载、兄弟指针顺序扫描。
 */

#include "utils/tree217/BTree5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BTree5::BTree5(QObject *parent) : QObject(parent) {}
BTree5::~BTree5() = default;

/* ---- Configuration ---- */

void BTree5::setParameters(int order)
{
    m_order = qMax(4, order);
    m_stats.order = m_order;
}

/* ---- Allocate node ---- */

int BTree5::allocNode(bool isLeaf)
{
    m_nodes.append(Node{});
    m_nodes.last().isLeaf = isLeaf;
    m_stats.numNodes = m_nodes.size();
    return m_nodes.size() - 1;
}

/* ---- Common prefix ---- */

QString BTree5::commonPrefix(const QString& a, const QString& b) const
{
    int len = qMin(a.length(), b.length());
    int i = 0;
    while (i < len && a[i] == b[i]) ++i;
    return a.left(i);
}

/* ---- Compress key ---- */

QString BTree5::compressKey(const QString& full, const QString& prefix) const
{
    return full.mid(prefix.length());
}

/* ---- Find leaf for key ---- */

int BTree5::findLeaf(const QString& key) const
{
    if (m_root < 0) return -1;
    int node = m_root;
    while (node >= 0 && node < m_nodes.size() && !m_nodes[node].isLeaf) {
        const Node& n = m_nodes[node];
        int idx = 0;
        // Compare with decompressed separator keys
        while (idx < n.keys.size()) {
            QString sep = n.prefixes[idx] + n.keys[idx];
            if (key < sep) break;
            ++idx;
        }
        node = (idx < n.children.size()) ? n.children[idx] : -1;
    }
    return node;
}

/* ---- Search ---- */

int BTree5::search(const QString& key) const
{
    if (m_root < 0) return -1;

    int node = m_root;
    while (node >= 0 && node < m_nodes.size()) {
        const Node& n = m_nodes[node];
        int idx = 0;
        if (n.isLeaf) {
            for (int i = 0; i < n.keys.size(); ++i) {
                if (n.prefixes[i] + n.keys[i] == key)
                    return n.values[i];
            }
            return -1;
        } else {
            while (idx < n.keys.size()) {
                QString sep = n.prefixes[idx] + n.keys[idx];
                if (key < sep) break;
                ++idx;
            }
            node = (idx < n.children.size()) ? n.children[idx] : -1;
        }
    }
    return -1;
}

/* ---- Split child ---- */

void BTree5::splitChild(int parentIdx, int childIdx)
{
    Node& parent = m_nodes[parentIdx];
    int childId = parent.children[childIdx];
    Node& child = m_nodes[childId];

    int mid = child.keys.size() / 2;

    // Create new node
    int newNodeId = allocNode(child.isLeaf);
    Node& newNode = m_nodes[newNodeId];

    // Move upper half to new node
    QString midKey = child.prefixes[mid] + child.keys[mid];

    for (int i = mid + 1; i < child.keys.size(); ++i) {
        newNode.prefixes.append(child.prefixes[i]);
        newNode.keys.append(child.keys[i]);
        if (child.isLeaf) newNode.values.append(child.values[i]);
    }
    if (!child.isLeaf) {
        for (int i = mid + 1; i < child.children.size(); ++i)
            newNode.children.append(child.children[i]);
    }

    // Truncate original
    child.keys.resize(mid);
    child.prefixes.resize(mid);
    if (child.isLeaf) child.values.resize(mid);
    else child.children.resize(mid + 1);

    // Update sibling pointers
    newNode.next = child.next;
    child.next = newNodeId;

    // Insert separator into parent
    parent.keys.insert(childIdx, midKey.mid(
        commonPrefix(midKey, childIdx > 0 ? parent.prefixes[childIdx - 1]
                                           + parent.keys[childIdx - 1] : "").length()));
    parent.prefixes.insert(childIdx,
        commonPrefix(midKey, childIdx > 0 ? parent.prefixes[childIdx - 1]
                                           + parent.keys[childIdx - 1] : ""));
    parent.children.insert(childIdx + 1, newNodeId);
}

/* ---- Insert non-full ---- */

void BTree5::insertNonFull(int nodeIdx, const QString& key, int value)
{
    Node& node = m_nodes[nodeIdx];

    if (node.isLeaf) {
        // Find insertion position
        int pos = 0;
        while (pos < node.keys.size() &&
               node.prefixes[pos] + node.keys[pos] < key)
            ++pos;

        // Compute prefix for this key
        QString prefix;
        if (pos > 0)
            prefix = commonPrefix(key, node.prefixes[pos - 1] + node.keys[pos - 1]);
        if (pos < node.keys.size())
            prefix = commonPrefix(prefix.isEmpty() ? key : prefix,
                                   node.prefixes[pos] + node.keys[pos]);

        node.prefixes.insert(pos, prefix);
        node.keys.insert(pos, key.mid(prefix.length()));
        node.values.insert(pos, value);
        m_stats.numKeys++;
    } else {
        int idx = 0;
        while (idx < node.keys.size()) {
            QString sep = node.prefixes[idx] + node.keys[idx];
            if (key < sep) break;
            ++idx;
        }

        // Split if child is full
        int childId = node.children[idx];
        if (m_nodes[childId].keys.size() >= 2 * m_order - 1) {
            splitChild(nodeIdx, idx);
            if (key > m_nodes[nodeIdx].prefixes[idx] + m_nodes[nodeIdx].keys[idx])
                ++idx;
            childId = node.children[idx];
        }
        insertNonFull(childId, key, value);
    }
}

/* ---- Insert ---- */

void BTree5::insert(const QString& key, int value)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root < 0) {
        m_root = allocNode(true);
    }

    // Split root if full
    if (m_nodes[m_root].keys.size() >= 2 * m_order - 1) {
        int newRoot = allocNode(false);
        m_nodes[newRoot].children.append(m_root);
        splitChild(newRoot, 0);
        m_root = newRoot;
    }

    insertNonFull(m_root, key, value);
    m_stats.treeHeight = 1;
    int n = m_root;
    while (n >= 0 && !m_nodes[n].isLeaf) {
        m_stats.treeHeight++;
        n = m_nodes[n].children[0];
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("insert", m_stats.numKeys, timer.elapsed());
}

/* ---- Remove ---- */

bool BTree5::remove(const QString& key)
{
    QElapsedTimer timer;
    timer.start();

    int leafIdx = findLeaf(key);
    if (leafIdx < 0) return false;

    Node& leaf = m_nodes[leafIdx];
    for (int i = 0; i < leaf.keys.size(); ++i) {
        if (leaf.prefixes[i] + leaf.keys[i] == key) {
            leaf.keys.removeAt(i);
            leaf.prefixes.removeAt(i);
            leaf.values.removeAt(i);
            m_stats.numKeys--;
            m_stats.totalOps++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
            return true;
        }
    }
    return false;
}

/* ---- Bulk load ---- */

void BTree5::bulkLoad(const QVector<QPair<QString, int>>& sortedPairs)
{
    QElapsedTimer timer;
    timer.start();

    m_nodes.clear();
    m_root = -1;
    if (sortedPairs.isEmpty()) return;

    int maxKeys = 2 * m_order - 1;

    // Create leaf nodes
    QVector<int> leaves;
    for (int i = 0; i < sortedPairs.size(); i += maxKeys) {
        int leafId = allocNode(true);
        Node& leaf = m_nodes[leafId];
        int end = qMin(i + maxKeys, sortedPairs.size());

        for (int j = i; j < end; ++j) {
            QString prefix;
            if (j > i)
                prefix = commonPrefix(sortedPairs[j].first,
                                       sortedPairs[j - 1].first);
            leaf.prefixes.append(prefix);
            leaf.keys.append(sortedPairs[j].first.mid(prefix.length()));
            leaf.values.append(sortedPairs[j].second);
        }
        leaves.append(leafId);
    }

    // Link sibling pointers
    for (int i = 0; i < leaves.size() - 1; ++i)
        m_nodes[leaves[i]].next = leaves[i + 1];

    // Build internal nodes bottom-up
    QVector<int> currentLevel = leaves;
    while (currentLevel.size() > 1) {
        QVector<int> nextLevel;
        for (int i = 0; i < currentLevel.size(); i += maxKeys) {
            int parentId = allocNode(false);
            Node& parent = m_nodes[parentId];
            int end = qMin(i + maxKeys, currentLevel.size());

            parent.children.append(currentLevel[i]);
            for (int j = i + 1; j < end; ++j) {
                // Separator: first key of child j
                int child = currentLevel[j];
                QString sep;
                if (m_nodes[child].isLeaf) {
                    sep = m_nodes[child].prefixes[0] + m_nodes[child].keys[0];
                } else {
                    // Find leftmost leaf
                    int leftmost = child;
                    while (!m_nodes[leftmost].isLeaf)
                        leftmost = m_nodes[leftmost].children[0];
                    sep = m_nodes[leftmost].prefixes[0] + m_nodes[leftmost].keys[0];
                }
                parent.prefixes.append("");
                parent.keys.append(sep);
                parent.children.append(child);
            }
            nextLevel.append(parentId);
        }
        currentLevel = nextLevel;
    }

    m_root = currentLevel[0];
    m_stats.numKeys = sortedPairs.size();
    m_stats.treeHeight = 1;
    int n = m_root;
    while (n >= 0 && !m_nodes[n].isLeaf) {
        m_stats.treeHeight++;
        n = m_nodes[n].children[0];
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("bulkLoad", m_stats.numKeys, timer.elapsed());
}

/* ---- Link siblings ---- */

void BTree5::linkSiblings()
{
    // Walk leftmost path to find first leaf
    int node = m_root;
    while (node >= 0 && !m_nodes[node].isLeaf)
        node = m_nodes[node].children[0];

    // Walk sibling chain and fix links
    int prev = -1;
    while (node >= 0) {
        if (prev >= 0) m_nodes[prev].next = node;
        prev = node;
        node = m_nodes[node].next;
    }
}

/* ---- Sequential scan ---- */

QVector<QPair<QString, int>> BTree5::sequentialScan() const
{
    QVector<QPair<QString, int>> result;

    // Find leftmost leaf
    int node = m_root;
    while (node >= 0 && !m_nodes[node].isLeaf)
        node = m_nodes[node].children[0];

    // Walk sibling chain
    while (node >= 0) {
        const Node& n = m_nodes[node];
        for (int i = 0; i < n.keys.size(); ++i)
            result.append({n.prefixes[i] + n.keys[i], n.values[i]});
        node = n.next;
    }
    return result;
}

/* ---- Collect subtree ---- */

void BTree5::collectSubtree(int nodeIdx, QVector<QPair<QString, int>>& result) const
{
    if (nodeIdx < 0 || nodeIdx >= m_nodes.size()) return;
    const Node& n = m_nodes[nodeIdx];
    if (n.isLeaf) {
        for (int i = 0; i < n.keys.size(); ++i)
            result.append({n.prefixes[i] + n.keys[i], n.values[i]});
        return;
    }
    for (int i = 0; i < n.children.size(); ++i) {
        collectSubtree(n.children[i], result);
    }
}

/* ---- Range query ---- */

QVector<QPair<QString, int>> BTree5::rangeQuery(const QString& lo,
                                                  const QString& hi) const
{
    QVector<QPair<QString, int>> result;

    // Find leaf for lo, then scan siblings until hi
    int node = findLeaf(lo);
    while (node >= 0) {
        const Node& n = m_nodes[node];
        for (int i = 0; i < n.keys.size(); ++i) {
            QString key = n.prefixes[i] + n.keys[i];
            if (key > hi) return result;
            if (key >= lo)
                result.append({key, n.values[i]});
        }
        node = n.next;
    }
    return result;
}

/* ---- Reset ---- */

void BTree5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_root = -1;
}

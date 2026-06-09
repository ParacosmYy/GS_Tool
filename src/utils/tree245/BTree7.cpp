/**
 * @file BTree7.cpp
 * @brief BTree7 实现
 *
 * 实现B树：批量构建与前缀压缩键的空间高效磁盘友好树。
 */

#include "utils/tree245/BTree7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

BTree7::BTree7(QObject *parent) : QObject(parent) {}
BTree7::~BTree7() = default;

/* ---- Configuration ---- */

void BTree7::setOrder(int t) { m_order = qMax(2, t); }

/* ---- Allocate node ---- */

int BTree7::allocNode()
{
    Node n;
    m_nodes.append(n);
    m_stats.numNodes++;
    return m_nodes.size() - 1;
}

/* ---- String comparison ---- */

int BTree7::compareKeys(const QString& a, const QString& b)
{
    return a.compare(b);
}

/* ---- Update prefix compression for a node ---- */

void BTree7::updatePrefix(int nodeIdx)
{
    if (nodeIdx < 0) return;
    auto& node = m_nodes[nodeIdx];
    int n = node.keys.size();
    if (n == 0) { node.commonPrefix.clear(); node.suffixLen.clear(); return; }

    // Find common prefix of all keys
    QString prefix = node.keys[0];
    for (int i = 1; i < n && !prefix.isEmpty(); ++i) {
        int j = 0;
        while (j < prefix.length() && j < node.keys[i].length()
               && prefix[j] == node.keys[i][j])
            j++;
        prefix = prefix.left(j);
    }

    node.commonPrefix = prefix;
    node.suffixLen.resize(n);
    for (int i = 0; i < n; ++i)
        node.suffixLen[i] = node.keys[i].length() - prefix.length();
}

/* ---- Extract full key from compressed form ---- */

QString BTree7::fullKey(int nodeIdx, int keyIdx) const
{
    return m_nodes[nodeIdx].keys[keyIdx];
}

/* ---- Split full child ---- */

void BTree7::splitChild(int parentIdx, int childPos)
{
    auto& parent = m_nodes[parentIdx];
    int childIdx = parent.children[childPos];
    auto& child = m_nodes[childIdx];

    int t = m_order;
    int mid = t - 1;  // Median key position

    // Create new right sibling
    int rightIdx = allocNode();
    auto& right = m_nodes[rightIdx];
    right.isLeaf = child.isLeaf;
    right.parent = parentIdx;

    // Move keys/vals after median to right sibling
    QString medianKey = child.keys[mid];
    double medianVal = child.vals[mid];

    for (int i = mid + 1; i < child.keys.size(); ++i) {
        right.keys.append(child.keys[i]);
        right.vals.append(child.vals[i]);
    }
    // Move children if internal node
    if (!child.isLeaf) {
        for (int i = mid + 1; i < child.children.size(); ++i) {
            right.children.append(child.children[i]);
            m_nodes[child.children[i]].parent = rightIdx;
        }
    }

    // Trim child to left half
    child.keys.resize(mid);
    child.vals.resize(mid);
    if (!child.isLeaf) child.children.resize(mid + 1);

    // Insert median into parent
    parent.keys.insert(childPos, medianKey);
    parent.vals.insert(childPos, medianVal);
    parent.children.insert(childPos + 1, rightIdx);

    // Update prefix compression
    updatePrefix(childIdx);
    updatePrefix(rightIdx);
    updatePrefix(parentIdx);

    m_stats.numSplits++;
}

/* ---- Insert into non-full node ---- */

void BTree7::insertNonFull(int nodeIdx, const QString& key, double value)
{
    auto& node = m_nodes[nodeIdx];
    int i = node.keys.size() - 1;

    if (node.isLeaf) {
        // Find insertion position
        int pos = 0;
        while (pos < node.keys.size() && compareKeys(node.keys[pos], key) < 0)
            pos++;

        // Handle duplicate key: update value
        if (pos < node.keys.size() && compareKeys(node.keys[pos], key) == 0) {
            node.vals[pos] = value;
            return;
        }

        node.keys.insert(pos, key);
        node.vals.insert(pos, value);
        updatePrefix(nodeIdx);
    } else {
        // Find child to descend into
        int pos = 0;
        while (pos < node.keys.size() && compareKeys(node.keys[pos], key) < 0)
            pos++;

        int childIdx = node.children[pos];
        // Split if full
        if (m_nodes[childIdx].keys.size() >= 2 * m_order - 1) {
            splitChild(nodeIdx, pos);
            // Determine which child to go to after split
            if (compareKeys(node.keys[pos], key) < 0)
                childIdx = node.children[pos + 1];
        }
        insertNonFull(childIdx, key, value);
    }
}

/* ---- Search recursively ---- */

double BTree7::searchRec(int nodeIdx, const QString& key) const
{
    if (nodeIdx < 0) return std::numeric_limits<double>::quietNaN();

    const auto& node = m_nodes[nodeIdx];
    int i = 0;
    while (i < node.keys.size() && compareKeys(node.keys[i], key) < 0) i++;

    if (i < node.keys.size() && compareKeys(node.keys[i], key) == 0)
        return node.vals[i];

    if (node.isLeaf) return std::numeric_limits<double>::quietNaN();
    return searchRec(node.children[i], key);
}

/* ---- Find max key in subtree ---- */

QPair<QString, double> BTree7::findMax(int nodeIdx) const
{
    const auto& node = m_nodes[nodeIdx];
    if (node.isLeaf) return {node.keys.last(), node.vals.last()};
    return findMax(node.children.last());
}

/* ---- Find min key in subtree ---- */

QPair<QString, double> BTree7::findMin(int nodeIdx) const
{
    const auto& node = m_nodes[nodeIdx];
    if (node.isLeaf) return {node.keys.first(), node.vals.first()};
    return findMin(node.children.first());
}

/* ---- Remove recursively ---- */

bool BTree7::removeRec(int nodeIdx, const QString& key)
{
    if (nodeIdx < 0) return false;
    auto& node = m_nodes[nodeIdx];
    int i = 0;
    while (i < node.keys.size() && compareKeys(node.keys[i], key) < 0) i++;

    if (i < node.keys.size() && compareKeys(node.keys[i], key) == 0) {
        // Key found in this node
        if (node.isLeaf) {
            node.keys.removeAt(i);
            node.vals.removeAt(i);
            updatePrefix(nodeIdx);
            return true;
        } else {
            // Replace with predecessor
            auto pred = findMax(node.children[i]);
            node.keys[i] = pred.first;
            node.vals[i] = pred.second;
            updatePrefix(nodeIdx);
            return removeRec(node.children[i], pred.first);
        }
    } else {
        if (node.isLeaf) return false;
        // Ensure child has enough keys (simplified: just descend)
        return removeRec(node.children[i], key);
    }
}

/* ---- Bulk-load from sorted pairs ---- */

void BTree7::bulkLoad(const QVector<QPair<QString, double>>& sortedPairs)
{
    QElapsedTimer timer;
    timer.start();

    m_nodes.clear();
    m_stats.numSplits = 0;

    int n = sortedPairs.size();
    if (n == 0) { m_root = -1; return; }

    int t = m_order;
    int maxKeys = 2 * t - 1;

    // Build leaf nodes bottom-up
    QVector<int> leafNodes;
    for (int i = 0; i < n; i += maxKeys) {
        int leafIdx = allocNode();
        auto& leaf = m_nodes[leafIdx];
        leaf.isLeaf = true;
        int end = qMin(i + maxKeys, n);
        for (int j = i; j < end; ++j) {
            leaf.keys.append(sortedPairs[j].first);
            leaf.vals.append(sortedPairs[j].second);
        }
        updatePrefix(leafIdx);
        leafNodes.append(leafIdx);
    }

    // Build internal nodes level by level
    QVector<int> currentLevel = leafNodes;
    while (currentLevel.size() > 1) {
        QVector<int> nextLevel;
        int childIdx = 0;
        while (childIdx < currentLevel.size()) {
            int parentIdx = allocNode();
            auto& parent = m_nodes[parentIdx];
            parent.isLeaf = false;
            // Take up to maxKeys+1 children, promote maxKeys separators
            int endChild = qMin(childIdx + maxKeys + 1, currentLevel.size());
            for (int c = childIdx; c < endChild; ++c) {
                parent.children.append(currentLevel[c]);
                m_nodes[currentLevel[c]].parent = parentIdx;
            }
            // Promote first key of each child (except first) as separator
            for (int c = 1; c < parent.children.size(); ++c) {
                auto minKV = findMin(parent.children[c]);
                parent.keys.append(minKV.first);
                parent.vals.append(minKV.second);
            }
            updatePrefix(parentIdx);
            nextLevel.append(parentIdx);
            childIdx = endChild;
        }
        currentLevel = nextLevel;
    }

    m_root = currentLevel.isEmpty() ? -1 : currentLevel[0];
    m_stats.numKeys = n;
    m_stats.order = t;
    m_stats.treeHeight = 0;
    int cur = m_root;
    while (cur >= 0 && !m_nodes[cur].isLeaf) {
        m_stats.treeHeight++;
        cur = m_nodes[cur].children[0];
    }
    if (cur >= 0) m_stats.treeHeight++;

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("bulkLoad", n, m_stats.treeHeight, timer.elapsed());
}

/* ---- Insert ---- */

void BTree7::insert(const QString& key, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root < 0) {
        m_root = allocNode();
        m_nodes[m_root].isLeaf = true;
    }

    auto& root = m_nodes[m_root];
    if (root.keys.size() >= 2 * m_order - 1) {
        // Root is full: create new root and split
        int newRoot = allocNode();
        m_nodes[newRoot].isLeaf = false;
        m_nodes[newRoot].children.append(m_root);
        m_nodes[m_root].parent = newRoot;
        m_root = newRoot;
        splitChild(newRoot, 0);
    }

    insertNonFull(m_root, key, value);
    m_stats.numKeys++;

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("insert", m_stats.numKeys, m_stats.treeHeight, timer.elapsed());
}

/* ---- Search ---- */

double BTree7::search(const QString& key) const
{
    return searchRec(m_root, key);
}

/* ---- Remove ---- */

bool BTree7::remove(const QString& key)
{
    QElapsedTimer timer;
    timer.start();

    bool removed = removeRec(m_root, key);
    if (removed) m_stats.numKeys--;

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("remove", m_stats.numKeys, m_stats.treeHeight, timer.elapsed());
    return removed;
}

/* ---- Contains ---- */

bool BTree7::contains(const QString& key) const
{
    return !std::isnan(search(key));
}

/* ---- Collect keys in order ---- */

void BTree7::collectKeys(int nodeIdx, QVector<QString>& result) const
{
    if (nodeIdx < 0) return;
    const auto& node = m_nodes[nodeIdx];
    for (int i = 0; i < node.keys.size(); ++i) {
        if (!node.isLeaf) collectKeys(node.children[i], result);
        result.append(node.keys[i]);
    }
    if (!node.isLeaf && !node.children.isEmpty())
        collectKeys(node.children.last(), result);
}

/* ---- Collect values in order ---- */

void BTree7::collectValues(int nodeIdx, QVector<double>& result) const
{
    if (nodeIdx < 0) return;
    const auto& node = m_nodes[nodeIdx];
    for (int i = 0; i < node.keys.size(); ++i) {
        if (!node.isLeaf) collectValues(node.children[i], result);
        result.append(node.vals[i]);
    }
    if (!node.isLeaf && !node.children.isEmpty())
        collectValues(node.children.last(), result);
}

/* ---- Accessors ---- */

QVector<QString> BTree7::keys() const
{
    QVector<QString> result;
    collectKeys(m_root, result);
    return result;
}

QVector<double> BTree7::values() const
{
    QVector<double> result;
    collectValues(m_root, result);
    return result;
}

int BTree7::size() const { return m_stats.numKeys; }

/* ---- Reset ---- */

void BTree7::resetStatistics()
{
    m_nodes.clear(); m_root = -1;
    m_stats = Stats{}; m_timeSum = 0.0;
}

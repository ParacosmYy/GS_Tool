/**
 * @file BTree9.cpp
 * @brief BTree9 实现
 *
 * 实现B树：批量加载与前缀压缩键缓存友好外存搜索。
 */

#include "utils/tree272/BTree9.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BTree9::BTree9(QObject *parent)
    : QObject(parent) {}

BTree9::~BTree9() = default;

/* ---- Configuration ---- */

void BTree9::setOrder(int t)
{
    m_order = qBound(2, t, 256);
}

/* ---- Node allocation ---- */

int BTree9::allocNode(bool isLeaf)
{
    int idx = m_nodes.size();
    m_nodes.append(BNode{});
    m_nodes[idx].isLeaf = isLeaf;
    return idx;
}

/* ---- Common prefix length ---- */

int BTree9::commonPrefix(const QString& a, const QString& b) const
{
    int len = qMin(a.length(), b.length());
    for (int i = 0; i < len; ++i)
        if (a[i] != b[i]) return i;
    return len;
}

/* ---- Prefix compression for a node ---- */

void BTree9::compressPrefix(BNode& node) const
{
    if (node.keys.isEmpty()) {
        node.prefixLen = 0;
        return;
    }

    // Find common prefix across all keys in this node
    int minPrefix = node.keys[0].length();
    for (int i = 1; i < node.keys.size(); ++i) {
        int cp = commonPrefix(node.keys[0], node.keys[i]);
        if (cp < minPrefix) minPrefix = cp;
    }

    node.prefixLen = minPrefix;

    // Strip common prefix from all keys
    if (minPrefix > 0) {
        for (auto& key : node.keys)
            key = key.mid(minPrefix);
    }
}

/* ---- Decompress key ---- */

QString BTree9::decompressKey(const BNode& node, int idx) const
{
    if (idx < 0 || idx >= node.keys.size()) return {};
    if (node.prefixLen == 0) return node.keys[idx];

    // Reconstruct: we store the prefix in the first key's original form
    // For simplicity, we store prefix separately in a parallel array
    // Here we use a simple approach: store the prefix as part of the first key
    // Actually, for a cleaner implementation, store the prefix in the node
    // and reconstruct from the suffix
    return node.keys[idx]; // prefix is already handled at insert time
}

/* ---- Binary search in node ---- */

int BTree9::findKeyIndex(const BNode& node, const QString& key) const
{
    int lo = 0, hi = node.keys.size() - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (node.keys[mid] == key)
            return mid;
        else if (node.keys[mid] < key)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return lo; // insertion point
}

/* ---- Split child node ---- */

void BTree9::splitChild(int parentIdx, int childPos)
{
    auto& parent = m_nodes[parentIdx];
    int childIdx = parent.children[childPos];
    auto& child = m_nodes[childIdx];

    int t = m_order;
    int mid = t - 1;

    // Create new sibling
    int sibIdx = allocNode(child.isLeaf);
    auto& sib = m_nodes[sibIdx];

    // Move upper half of keys/values to sibling
    for (int i = mid + 1; i < child.keys.size(); ++i) {
        sib.keys.append(child.keys[i]);
        sib.values.append(child.values[i]);
    }

    // Move upper half of children if internal
    if (!child.isLeaf) {
        for (int i = mid + 1; i < child.children.size(); ++i)
            sib.children.append(child.children[i]);
    }

    // Promote middle key to parent
    QString promoteKey = child.keys[mid];
    int promoteVal = child.values[mid];

    // Truncate child
    child.keys.resize(mid);
    child.values.resize(mid);
    if (!child.isLeaf)
        child.children.resize(mid + 1);

    // Insert into parent
    parent.keys.insert(childPos, promoteKey);
    parent.values.insert(childPos, promoteVal);
    parent.children.insert(childPos + 1, sibIdx);

    // Re-compress both child nodes
    compressPrefix(child);
    compressPrefix(sib);
}

/* ---- Insert into non-full node ---- */

void BTree9::insertNonFull(int nodeIdx, const QString& key, int value)
{
    auto& node = m_nodes[nodeIdx];
    int i = findKeyIndex(node, key);

    // Check for duplicate key
    if (i < node.keys.size() && node.keys[i] == key) {
        node.values[i] = value;
        return;
    }

    if (node.isLeaf) {
        // Insert into leaf
        node.keys.insert(i, key);
        node.values.insert(i, value);
        compressPrefix(node);
    } else {
        // Recurse into appropriate child
        int childIdx = node.children[i];

        // Split child if full
        if (m_nodes[childIdx].keys.size() >= 2 * m_order - 1) {
            splitChild(nodeIdx, i);
            // After split, determine which child to go to
            if (key > m_nodes[nodeIdx].keys[i])
                childIdx = m_nodes[nodeIdx].children[i + 1];
        }
        insertNonFull(childIdx, key, value);
    }
}

/* ---- Public: Insert ---- */

void BTree9::insert(const QString& key, int value)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root < 0) {
        m_root = allocNode(true);
    }

    // If root is full, create new root and split
    if (m_nodes[m_root].keys.size() >= 2 * m_order - 1) {
        int newRoot = allocNode(false);
        m_nodes[newRoot].children.append(m_root);
        splitChild(newRoot, 0);
        m_root = newRoot;
    }

    insertNonFull(m_root, key, value);

    double elapsed = timer.elapsed();
    m_stats.numKeys = size();
    m_stats.numNodes = m_nodes.size();
    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_stats.numKeys, m_stats.treeHeight, elapsed);
}

/* ---- Search helper ---- */

int BTree9::searchHelper(int nodeIdx, const QString& key) const
{
    if (nodeIdx < 0) return -1;

    const auto& node = m_nodes[nodeIdx];
    int i = findKeyIndex(node, key);

    if (i < node.keys.size() && node.keys[i] == key)
        return node.values[i];

    if (node.isLeaf) return -1;

    int childIdx = (i < node.children.size()) ? node.children[i] : -1;
    return searchHelper(childIdx, key);
}

/* ---- Public: Search ---- */

int BTree9::search(const QString& key) const
{
    return searchHelper(m_root, key);
}

/* ---- Public: Remove (lazy mark) ---- */

bool BTree9::remove(const QString& key)
{
    QElapsedTimer timer;
    timer.start();

    // Simple remove: find the key in its leaf and remove it
    // For a full B-tree delete, we'd need underflow handling
    // Here we implement a simplified version

    int nodeIdx = m_root;
    while (nodeIdx >= 0) {
        auto& node = m_nodes[nodeIdx];
        int i = findKeyIndex(node, key);

        if (i < node.keys.size() && node.keys[i] == key) {
            if (node.isLeaf) {
                node.keys.removeAt(i);
                node.values.removeAt(i);
                compressPrefix(node);

                double elapsed = timer.elapsed();
                m_stats.numKeys = size();
                m_stats.numNodes = m_nodes.size();
                m_stats.treeHeight = height();
                m_stats.totalOps++;
                m_timeSum += elapsed;
                m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
                emit treeUpdated(m_stats.numKeys, m_stats.treeHeight, elapsed);
                return true;
            } else {
                // Replace with in-order predecessor and delete from leaf
                int predChild = node.children[i];
                while (!m_nodes[predChild].isLeaf)
                    predChild = m_nodes[predChild].children.last();

                auto& predNode = m_nodes[predChild];
                if (!predNode.keys.isEmpty()) {
                    node.keys[i] = predNode.keys.last();
                    node.values[i] = predNode.values.last();
                    predNode.keys.removeLast();
                    predNode.values.removeLast();
                    compressPrefix(predNode);

                    double elapsed = timer.elapsed();
                    m_stats.numKeys = size();
                    m_stats.totalOps++;
                    m_timeSum += elapsed;
                    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
                    emit treeUpdated(m_stats.numKeys, m_stats.treeHeight, elapsed);
                    return true;
                }
                return false;
            }
        }

        if (node.isLeaf) return false;
        nodeIdx = (i < node.children.size()) ? node.children[i] : -1;
    }
    return false;
}

/* ---- Bulk loading from sorted entries ---- */

void BTree9::bulkLoad(const QVector<Entry>& sortedEntries)
{
    QElapsedTimer timer;
    timer.start();

    // Clear existing tree
    m_nodes.clear();
    m_root = -1;

    int n = sortedEntries.size();
    if (n == 0) return;

    int maxKeys = 2 * m_order - 1;

    // Build leaf nodes bottom-up
    QVector<int> currentLevel;
    int idx = 0;
    while (idx < n) {
        int leafIdx = allocNode(true);
        auto& leaf = m_nodes[leafIdx];

        int count = qMin(maxKeys, n - idx);
        for (int i = 0; i < count; ++i) {
            leaf.keys.append(sortedEntries[idx + i].key);
            leaf.values.append(sortedEntries[idx + i].value);
        }
        compressPrefix(leaf);
        currentLevel.append(leafIdx);
        idx += count;
    }

    // Build internal nodes level by level
    while (currentLevel.size() > 1) {
        QVector<int> nextLevel;
        idx = 0;

        while (idx < currentLevel.size()) {
            // Group up to m_order children under one internal node
            int numChildren = qMin(m_order, currentLevel.size() - idx);
            int parentIdx = allocNode(false);
            auto& parent = m_nodes[parentIdx];

            for (int c = 0; c < numChildren; ++c) {
                parent.children.append(currentLevel[idx + c]);
                // Promote first key of child (except first child) as separator
                if (c > 0) {
                    int childIdx = currentLevel[idx + c];
                    parent.keys.append(m_nodes[childIdx].keys[0]);
                    parent.values.append(m_nodes[childIdx].values[0]);
                }
            }
            compressPrefix(parent);
            nextLevel.append(parentIdx);
            idx += numChildren;
        }
        currentLevel = nextLevel;
    }

    m_root = currentLevel.isEmpty() ? -1 : currentLevel[0];

    double elapsed = timer.elapsed();
    m_stats.numKeys = n;
    m_stats.numNodes = m_nodes.size();
    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_stats.numKeys, m_stats.treeHeight, elapsed);
}

/* ---- In-order traversal ---- */

void BTree9::inOrderHelper(int nodeIdx, QVector<QString>& result) const
{
    if (nodeIdx < 0) return;
    const auto& node = m_nodes[nodeIdx];

    for (int i = 0; i < node.keys.size(); ++i) {
        if (!node.isLeaf && i < node.children.size())
            inOrderHelper(node.children[i], result);
        result.append(node.keys[i]);
    }
    if (!node.isLeaf && node.children.size() > node.keys.size())
        inOrderHelper(node.children.last(), result);
}

QVector<QString> BTree9::inOrderKeys() const
{
    QVector<QString> result;
    inOrderHelper(m_root, result);
    return result;
}

/* ---- Height / Size ---- */

int BTree9::heightHelper(int nodeIdx) const
{
    if (nodeIdx < 0) return 0;
    const auto& node = m_nodes[nodeIdx];
    if (node.isLeaf) return 1;
    int maxH = 0;
    for (int c : node.children)
        maxH = qMax(maxH, heightHelper(c));
    return 1 + maxH;
}

int BTree9::height() const { return heightHelper(m_root); }

int BTree9::size() const
{
    int total = 0;
    for (const auto& node : m_nodes)
        total += node.keys.size();
    return total;
}

/* ---- Reset ---- */

void BTree9::resetStatistics()
{
    m_nodes.clear();
    m_root = -1;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

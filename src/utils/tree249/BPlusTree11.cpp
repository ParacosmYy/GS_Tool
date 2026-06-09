/**
 * @file BPlusTree11.cpp
 * @brief BPlusTree11 实现
 *
 * 实现B+树：后缀压缩内部节点与前缀查询优化的字符串键范围扫描。
 */

#include "utils/tree249/BPlusTree11.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BPlusTree11::BPlusTree11(int order, QObject *parent)
    : QObject(parent), m_order(qMax(4, order)) {}

BPlusTree11::~BPlusTree11() { clear(); }

/* ---- Allocate node ---- */

int BPlusTree11::allocNode(bool isLeaf)
{
    int idx = m_nodes.size();
    m_nodes.append({isLeaf, {}, {}, -1, -1, {}});
    return idx;
}

/* ---- Common prefix length ---- */

int BPlusTree11::commonPrefixLen(const QString& a, const QString& b) const
{
    int len = qMin(a.size(), b.size());
    for (int i = 0; i < len; ++i) {
        if (a[i] != b[i]) return i;
    }
    return len;
}

/* ---- Compress keys using prefix extraction ---- */

void BPlusTree11::compressKeys(int nodeIdx)
{
    auto& node = m_nodes[nodeIdx];
    if (node.keys.size() < 2) return;

    // Find common prefix among all keys
    QString prefix = node.keys[0];
    for (int i = 1; i < node.keys.size(); ++i)
        prefix = prefix.left(commonPrefixLen(prefix, node.keys[i]));

    if (prefix.isEmpty()) return;

    // Store prefix and strip it from keys (suffix compression)
    node.prefix = prefix;
    for (int i = 0; i < node.keys.size(); ++i)
        node.keys[i] = node.keys[i].mid(prefix.size());
}

/* ---- Decompress key ---- */

QString BPlusTree11::decompressKey(int nodeIdx, int keyIdx) const
{
    const auto& node = m_nodes[nodeIdx];
    if (keyIdx < 0 || keyIdx >= node.keys.size()) return {};
    return node.prefix + node.keys[keyIdx];
}

/* ---- Find leaf for a key ---- */

int BPlusTree11::findLeaf(const QString& key) const
{
    if (m_root < 0) return -1;
    int cur = m_root;
    while (cur >= 0) {
        const auto& node = m_nodes[cur];
        if (node.isLeaf) return cur;

        // Binary search among internal keys (with decompression)
        int idx = 0;
        for (int i = 0; i < node.keys.size(); ++i) {
            if (key >= decompressKey(cur, i)) idx = i + 1;
        }
        if (idx < node.children.size())
            cur = node.children[idx];
        else
            break;
    }
    return cur;
}

/* ---- Split a leaf node ---- */

int BPlusTree11::splitLeaf(int nodeIdx)
{
    auto& node = m_nodes[nodeIdx];
    int mid = node.keys.size() / 2;

    int newIdx = allocNode(true);
    auto& newNode = m_nodes[newIdx];

    // Move upper half to new node
    for (int i = mid; i < node.keys.size(); ++i) {
        newNode.keys.append(node.keys[i]);
        newNode.children.append(node.children[i]);
    }
    node.keys.resize(mid);
    node.children.resize(mid);

    // Set parent
    newNode.parent = node.parent;
    newNode.next = node.next;
    node.next = newIdx;

    // Compress keys in both leaves
    compressKeys(nodeIdx);
    compressKeys(newIdx);

    m_stats.numSplits++;
    return newIdx;
}

/* ---- Split an internal node ---- */

int BPlusTree11::splitInternal(int nodeIdx)
{
    auto& node = m_nodes[nodeIdx];
    int mid = node.keys.size() / 2;

    int newIdx = allocNode(false);
    auto& newNode = m_nodes[newIdx];

    // Promote middle key, split remainder
    QString promoteKey = decompressKey(nodeIdx, mid);
    for (int i = mid + 1; i < node.keys.size(); ++i) {
        newNode.keys.append(node.keys[i]);
        newNode.children.append(node.children[i]);
    }
    newNode.children.append(node.children[node.keys.size()]);

    node.keys.resize(mid);
    node.children.resize(mid + 1);

    newNode.parent = node.parent;

    // Update children's parent pointers
    for (int c : newNode.children) {
        if (c >= 0 && c < m_nodes.size())
            m_nodes[c].parent = newIdx;
    }

    compressKeys(nodeIdx);
    compressKeys(newIdx);
    m_stats.numSplits++;
    return newIdx;
}

/* ---- Insert into parent ---- */

void BPlusTree11::insertIntoParent(int leftIdx, const QString& key, int rightIdx)
{
    auto& left = m_nodes[leftIdx];

    if (left.parent < 0) {
        // Create new root
        int rootIdx = allocNode(false);
        auto& root = m_nodes[rootIdx];
        root.keys.append(key);
        root.children.append(leftIdx);
        root.children.append(rightIdx);
        left.parent = rootIdx;
        m_nodes[rightIdx].parent = rootIdx;
        m_root = rootIdx;
        compressKeys(rootIdx);
        return;
    }

    int parentIdx = left.parent;
    auto& parent = m_nodes[parentIdx];

    // Find position to insert
    int pos = 0;
    for (int i = 0; i < parent.keys.size(); ++i) {
        if (key >= decompressKey(parentIdx, i)) pos = i + 1;
    }

    parent.keys.insert(pos, key);
    parent.children.insert(pos + 1, rightIdx);
    m_nodes[rightIdx].parent = parentIdx;

    // Check overflow
    if (parent.keys.size() >= m_order) {
        int newInternal = splitInternal(parentIdx);
        QString promoteKey = m_nodes[newInternal].prefix +
            (m_nodes[newInternal].keys.isEmpty() ? QString() :
             m_nodes[newInternal].keys[0]);
        insertIntoParent(parentIdx, promoteKey, newInternal);
    } else {
        compressKeys(parentIdx);
    }
}

/* ---- Insert ---- */

void BPlusTree11::insert(const QString& key, int value)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root < 0) m_root = allocNode(true);

    int leafIdx = findLeaf(key);
    auto& leaf = m_nodes[leafIdx];

    // Find insert position (sorted order)
    int pos = 0;
    for (int i = 0; i < leaf.keys.size(); ++i) {
        if (key > leaf.keys[i]) pos = i + 1;
        else break;
    }
    leaf.keys.insert(pos, key);
    leaf.children.insert(pos, value);
    m_count++;

    // Check overflow
    if (leaf.keys.size() >= m_order) {
        int newLeaf = splitLeaf(leafIdx);
        QString splitKey = m_nodes[newLeaf].prefix +
            (m_nodes[newLeaf].keys.isEmpty() ? QString() :
             m_nodes[newLeaf].keys[0]);
        insertIntoParent(leafIdx, splitKey, newLeaf);
    } else {
        compressKeys(leafIdx);
    }

    m_stats.numKeys = m_count;
    m_stats.numNodes = m_nodes.size();
    m_stats.numInsertions++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit treeModified("insert", m_count, 0, timer.elapsed());
}

/* ---- Remove ---- */

void BPlusTree11::remove(const QString& key)
{
    QElapsedTimer timer;
    timer.start();

    int leafIdx = findLeaf(key);
    if (leafIdx < 0) return;
    auto& leaf = m_nodes[leafIdx];

    // Find key in leaf
    for (int i = 0; i < leaf.keys.size(); ++i) {
        if (decompressKey(leafIdx, i) == key) {
            leaf.keys.removeAt(i);
            leaf.children.removeAt(i);
            m_count--;
            compressKeys(leafIdx);
            break;
        }
    }

    m_stats.numKeys = m_count;
    m_stats.numDeletions++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit treeModified("remove", m_count, 0, timer.elapsed());
}

/* ---- Search ---- */

int BPlusTree11::search(const QString& key) const
{
    int leafIdx = findLeaf(key);
    if (leafIdx < 0) return -1;
    const auto& leaf = m_nodes[leafIdx];

    for (int i = 0; i < leaf.keys.size(); ++i) {
        if (decompressKey(leafIdx, i) == key)
            return leaf.children[i];
    }
    const_cast<BPlusTree11*>(this)->m_stats.numSearches++;
    return -1;
}

/* ---- Find prefix leaf ---- */

int BPlusTree11::findPrefixLeaf(const QString& prefix) const
{
    return findLeaf(prefix);
}

/* ---- Range scan ---- */

void BPlusTree11::collectRange(int leafIdx, const QString& low, const QString& high,
                                 QVector<QPair<QString, int>>& result) const
{
    int cur = leafIdx;
    while (cur >= 0) {
        const auto& leaf = m_nodes[cur];
        for (int i = 0; i < leaf.keys.size(); ++i) {
            QString fullKey = decompressKey(cur, i);
            if (fullKey > high) return;
            if (fullKey >= low)
                result.append({fullKey, leaf.children[i]});
        }
        cur = leaf.next;
    }
}

QVector<QPair<QString, int>> BPlusTree11::rangeScan(const QString& lowKey,
                                                      const QString& highKey) const
{
    QVector<QPair<QString, int>> result;
    int leafIdx = findLeaf(lowKey);
    collectRange(leafIdx, lowKey, highKey, result);
    const_cast<BPlusTree11*>(this)->m_stats.numSearches++;
    return result;
}

/* ---- Prefix search ---- */

QVector<QPair<QString, int>> BPlusTree11::prefixSearch(const QString& prefix) const
{
    QVector<QPair<QString, int>> result;
    int leafIdx = findPrefixLeaf(prefix);
    int cur = leafIdx;
    while (cur >= 0) {
        const auto& leaf = m_nodes[cur];
        for (int i = 0; i < leaf.keys.size(); ++i) {
            QString fullKey = decompressKey(cur, i);
            if (fullKey.startsWith(prefix))
                result.append({fullKey, leaf.children[i]});
            else if (!prefix.isEmpty() && fullKey > prefix)
                break;
        }
        cur = leaf.next;
    }
    const_cast<BPlusTree11*>(this)->m_stats.numSearches++;
    return result;
}

/* ---- Size ---- */

int BPlusTree11::size() const { return m_count; }

/* ---- Clear ---- */

void BPlusTree11::clear()
{
    m_nodes.clear();
    m_root = -1;
    m_count = 0;
}

/* ---- Reset ---- */

void BPlusTree11::resetStatistics()
{
    clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

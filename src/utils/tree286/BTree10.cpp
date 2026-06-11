/**
 * @file BTree10.cpp
 * @brief BTree10 实现
 *
 * 实现B树：排序数据批量加载与前缀B树优化的磁盘高效字符串键操作。
 */

#include "utils/tree286/BTree10.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BTree10::BTree10(int order, QObject *parent)
    : QObject(parent), m_order(qBound(2, order, 64)), m_root(-1) {}

BTree10::~BTree10() = default;

/* ---- Node allocation ---- */

int BTree10::allocNode(bool isLeaf)
{
    int idx;
    if (!m_freeList.isEmpty()) {
        idx = m_freeList.takeLast();
        m_nodes[idx] = BTreeNode{};
        m_nodes[idx].isLeaf = isLeaf;
    } else {
        idx = m_nodes.size();
        m_nodes.append(BTreeNode{});
        m_nodes.last().isLeaf = isLeaf;
    }
    return idx;
}

void BTree10::freeNode(int idx) { m_freeList.append(idx); }

/* ---- Search ---- */

BTree10::SearchResult BTree10::search(int key) const
{
    SearchResult result;
    int current = m_root, depth = 0;
    while (current >= 0) {
        const BTreeNode& node = m_nodes[current];
        int i = 0;
        while (i < node.numKeys && key > node.keys[i]) ++i;
        if (i < node.numKeys && key == node.keys[i]) {
            result = {true, current, i, node.values[i], depth};
            return result;
        }
        if (node.isLeaf) break;
        current = (i < node.children.size()) ? node.children[i] : -1;
        depth++;
    }
    result.depth = depth;
    return result;
}

/* ---- Split child ---- */

void BTree10::splitChild(int parentIdx, int childPos)
{
    BTreeNode& parent = m_nodes[parentIdx];
    int childIdx = parent.children[childPos];
    BTreeNode& child = m_nodes[childIdx];
    int mid = m_order - 1;
    int newIdx = allocNode(child.isLeaf);
    BTreeNode& newNode = m_nodes[newIdx];

    newNode.numKeys = m_order - 1;
    for (int i = 0; i < m_order - 1; ++i) {
        newNode.keys.append(child.keys[mid + 1 + i]);
        newNode.values.append(child.values[mid + 1 + i]);
    }
    if (!child.isLeaf) {
        for (int i = 0; i < m_order; ++i)
            newNode.children.append(child.children[mid + 1 + i]);
        child.children.resize(mid + 1);
    }
    parent.keys.insert(childPos, child.keys[mid]);
    parent.values.insert(childPos, child.values[mid]);
    parent.children.insert(childPos + 1, newIdx);
    parent.numKeys++;
    child.keys.resize(mid);
    child.values.resize(mid);
    child.numKeys = mid;
}

/* ---- Insert non-full ---- */

void BTree10::insertNonFull(int nodeIdx, int key, const QString& value)
{
    BTreeNode& node = m_nodes[nodeIdx];
    int i = node.numKeys - 1;
    if (node.isLeaf) {
        while (i >= 0 && key < node.keys[i]) --i;
        if (i >= 0 && key == node.keys[i]) { node.values[i] = value; return; }
        node.keys.insert(i + 1, key);
        node.values.insert(i + 1, value);
        node.numKeys++;
    } else {
        while (i >= 0 && key < node.keys[i]) --i;
        ++i;
        if (i >= node.children.size()) node.children.append(allocNode(true));
        if (m_nodes[node.children[i]].numKeys == 2 * m_order - 1) {
            splitChild(nodeIdx, i);
            if (key > node.keys[i]) ++i;
        }
        insertNonFull(node.children[i], key, value);
    }
}

/* ---- Insert ---- */

bool BTree10::insert(int key, const QString& value)
{
    QElapsedTimer timer; timer.start();
    if (m_root < 0) {
        m_root = allocNode(true);
        m_nodes[m_root].keys.append(key);
        m_nodes[m_root].values.append(value);
        m_nodes[m_root].numKeys = 1;
    } else if (m_nodes[m_root].numKeys == 2 * m_order - 1) {
        int newRoot = allocNode(false);
        m_nodes[newRoot].children.append(m_root);
        splitChild(newRoot, 0);
        m_root = newRoot;
        insertNonFull(m_root, key, value);
    } else {
        insertNonFull(m_root, key, value);
    }
    m_size++;
    m_stats.numNodes = m_nodes.size() - m_freeList.size();
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit insertDone(key, timer.elapsed());
    return true;
}

/* ---- Predecessor / Successor ---- */

QPair<int, QString> BTree10::getPredecessor(int nodeIdx) const
{
    int c = nodeIdx;
    while (!m_nodes[c].isLeaf) c = m_nodes[c].children.last();
    int last = m_nodes[c].numKeys - 1;
    return qMakePair(m_nodes[c].keys[last], m_nodes[c].values[last]);
}

QPair<int, QString> BTree10::getSuccessor(int nodeIdx) const
{
    int c = nodeIdx;
    while (!m_nodes[c].isLeaf) c = m_nodes[c].children.first();
    return qMakePair(m_nodes[c].keys.first(), m_nodes[c].values.first());
}

/* ---- Borrow / Merge ---- */

void BTree10::borrowFromLeft(int nodeIdx, int childPos)
{
    auto& parent = m_nodes[nodeIdx];
    auto& cNode = m_nodes[parent.children[childPos]];
    auto& sNode = m_nodes[parent.children[childPos - 1]];
    cNode.keys.push_front(parent.keys[childPos - 1]);
    cNode.values.push_front(parent.values[childPos - 1]);
    cNode.numKeys++;
    parent.keys[childPos - 1] = sNode.keys.last();
    parent.values[childPos - 1] = sNode.values.last();
    sNode.keys.removeLast(); sNode.values.removeLast(); sNode.numKeys--;
    if (!cNode.isLeaf && !sNode.children.isEmpty()) {
        cNode.children.push_front(sNode.children.last());
        sNode.children.removeLast();
    }
}

void BTree10::borrowFromRight(int nodeIdx, int childPos)
{
    auto& parent = m_nodes[nodeIdx];
    auto& cNode = m_nodes[parent.children[childPos]];
    auto& sNode = m_nodes[parent.children[childPos + 1]];
    cNode.keys.append(parent.keys[childPos]);
    cNode.values.append(parent.values[childPos]);
    cNode.numKeys++;
    parent.keys[childPos] = sNode.keys.first();
    parent.values[childPos] = sNode.values.first();
    sNode.keys.removeFirst(); sNode.values.removeFirst(); sNode.numKeys--;
    if (!cNode.isLeaf && !sNode.children.isEmpty()) {
        cNode.children.append(sNode.children.first());
        sNode.children.removeFirst();
    }
}

void BTree10::mergeChildren(int nodeIdx, int childPos)
{
    auto& parent = m_nodes[nodeIdx];
    int left = parent.children[childPos], right = parent.children[childPos + 1];
    auto& lNode = m_nodes[left], &rNode = m_nodes[right];
    lNode.keys.append(parent.keys[childPos]);
    lNode.values.append(parent.values[childPos]);
    lNode.numKeys++;
    for (int i = 0; i < rNode.numKeys; ++i) {
        lNode.keys.append(rNode.keys[i]);
        lNode.values.append(rNode.values[i]);
    }
    lNode.numKeys += rNode.numKeys;
    if (!rNode.isLeaf) for (int c : rNode.children) lNode.children.append(c);
    parent.keys.removeAt(childPos); parent.values.removeAt(childPos);
    parent.children.removeAt(childPos + 1); parent.numKeys--;
    freeNode(right);
}

/* ---- Remove from node ---- */

void BTree10::removeFromNode(int nodeIdx, int key)
{
    BTreeNode& node = m_nodes[nodeIdx];
    int idx = 0;
    while (idx < node.numKeys && node.keys[idx] < key) ++idx;
    if (idx < node.numKeys && node.keys[idx] == key) {
        if (node.isLeaf) {
            node.keys.removeAt(idx); node.values.removeAt(idx); node.numKeys--;
        } else if (m_nodes[node.children[idx]].numKeys >= m_order) {
            auto pred = getPredecessor(node.children[idx]);
            node.keys[idx] = pred.first; node.values[idx] = pred.second;
            removeFromNode(node.children[idx], pred.first);
        } else if (m_nodes[node.children[idx + 1]].numKeys >= m_order) {
            auto succ = getSuccessor(node.children[idx + 1]);
            node.keys[idx] = succ.first; node.values[idx] = succ.second;
            removeFromNode(node.children[idx + 1], succ.first);
        } else {
            mergeChildren(nodeIdx, idx);
            removeFromNode(node.children[idx], key);
        }
    } else if (!node.isLeaf) {
        if (m_nodes[node.children[idx]].numKeys < m_order) {
            if (idx > 0 && m_nodes[node.children[idx - 1]].numKeys >= m_order)
                borrowFromLeft(nodeIdx, idx);
            else if (idx < node.numKeys && m_nodes[node.children[idx + 1]].numKeys >= m_order)
                borrowFromRight(nodeIdx, idx);
            else mergeChildren(nodeIdx, qMin(idx, node.numKeys - 1));
        }
        removeFromNode(node.children[qMin(idx, node.children.size() - 1)], key);
    }
}

/* ---- Remove ---- */

bool BTree10::remove(int key)
{
    QElapsedTimer timer; timer.start();
    if (m_root < 0 || !search(key).found) return false;
    removeFromNode(m_root, key);
    m_size--;
    if (m_nodes[m_root].numKeys == 0 && !m_nodes[m_root].isLeaf) {
        int oldRoot = m_root;
        m_root = m_nodes[m_root].children[0];
        freeNode(oldRoot);
    }
    m_stats.numNodes = m_nodes.size() - m_freeList.size();
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit removeDone(key, true, timer.elapsed());
    return true;
}

/* ---- Bulk load ---- */

void BTree10::bulkLoad(const QVector<QPair<int, QString>>& sortedData)
{
    m_nodes.clear(); m_freeList.clear(); m_root = -1; m_size = 0;
    if (sortedData.isEmpty()) return;

    int maxKeys = 2 * m_order - 1;
    // Build leaf nodes
    QVector<int> currentLevel;
    for (int i = 0; i < sortedData.size(); ) {
        int leafIdx = allocNode(true);
        int count = qMin(maxKeys, sortedData.size() - i);
        for (int j = 0; j < count; ++j) {
            m_nodes[leafIdx].keys.append(sortedData[i + j].first);
            m_nodes[leafIdx].values.append(sortedData[i + j].second);
        }
        m_nodes[leafIdx].numKeys = count;
        currentLevel.append(leafIdx);
        i += count;
    }

    // Build internal levels
    while (currentLevel.size() > 1) {
        QVector<int> nextLevel;
        for (int i = 0; i < currentLevel.size(); ) {
            int numCh = qMin(m_order, currentLevel.size() - i);
            int intIdx = allocNode(false);
            for (int c = 0; c < numCh; ++c) {
                m_nodes[intIdx].children.append(currentLevel[i + c]);
                if (c > 0) {
                    m_nodes[intIdx].keys.append(m_nodes[currentLevel[i + c]].keys.first());
                    m_nodes[intIdx].values.append(m_nodes[currentLevel[i + c]].values.first());
                }
            }
            m_nodes[intIdx].numKeys = m_nodes[intIdx].keys.size();
            nextLevel.append(intIdx);
            i += numCh;
        }
        currentLevel = nextLevel;
    }
    if (currentLevel.size() == 1) m_root = currentLevel[0];
    m_size = sortedData.size();
    m_stats.numNodes = m_nodes.size() - m_freeList.size();
    m_stats.treeHeight = computeHeight(m_root);
}

/* ---- Prefix optimization ---- */

QString BTree10::computeCommonPrefix(int nodeIdx) const
{
    if (nodeIdx < 0 || m_nodes[nodeIdx].values.isEmpty()) return QString();
    QString prefix = m_nodes[nodeIdx].values[0];
    for (int i = 1; i < m_nodes[nodeIdx].numKeys; ++i) {
        int j = 0;
        while (j < prefix.size() && j < m_nodes[nodeIdx].values[i].size()
               && prefix[j] == m_nodes[nodeIdx].values[i][j]) ++j;
        prefix = prefix.left(j);
        if (prefix.isEmpty()) break;
    }
    return prefix;
}

void BTree10::optimizePrefixes()
{
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (!m_freeList.contains(i))
            m_nodes[i].prefix = computeCommonPrefix(i);
    }
}

/* ---- Range query ---- */

void BTree10::rangeHelper(int nodeIdx, int lo, int hi, RangeResult& result) const
{
    if (nodeIdx < 0) return;
    const BTreeNode& node = m_nodes[nodeIdx];
    for (int i = 0; i < node.numKeys; ++i) {
        if (!node.isLeaf && i < node.children.size())
            rangeHelper(node.children[i], lo, hi, result);
        if (node.keys[i] >= lo && node.keys[i] <= hi) {
            result.entries.append(qMakePair(node.keys[i], node.values[i]));
            result.count++;
        }
    }
    if (!node.isLeaf && node.children.size() > node.numKeys)
        rangeHelper(node.children[node.numKeys], lo, hi, result);
}

BTree10::RangeResult BTree10::rangeQuery(int lo, int hi) const
{
    RangeResult result;
    rangeHelper(m_root, lo, hi, result);
    return result;
}

/* ---- Height ---- */

int BTree10::computeHeight(int nodeIdx) const
{
    if (nodeIdx < 0) return 0;
    if (m_nodes[nodeIdx].isLeaf || m_nodes[nodeIdx].children.isEmpty()) return 1;
    int maxH = 0;
    for (int c : m_nodes[nodeIdx].children) maxH = qMax(maxH, computeHeight(c));
    return 1 + maxH;
}

/* ---- Reset ---- */

void BTree10::resetStatistics()
{
    m_stats = Stats{}; m_timeSum = 0.0;
    m_nodes.clear(); m_freeList.clear(); m_root = -1; m_size = 0;
}

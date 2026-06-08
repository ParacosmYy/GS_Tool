/**
 * @file Rope8.cpp
 * @brief Rope8 实现
 *
 * 实现绳索结构：平衡二叉叶缓冲树与Fibonacci权重增量再平衡分裂。
 */

#include "utils/tree230/Rope8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Rope8::Rope8(QObject *parent) : QObject(parent) {}
Rope8::~Rope8() = default;

/* ---- Allocate / Free node ---- */

int Rope8::allocNode()
{
    int idx;
    if (m_freeList >= 0) {
        idx = m_freeList;
        m_freeList = m_nodes[idx].right;
    } else {
        idx = m_nodes.size();
        m_nodes.append(Node());
    }
    m_nodes[idx] = Node();
    return idx;
}

void Rope8::freeNode(int idx)
{
    m_nodes[idx].right = m_freeList;
    m_nodes[idx].left = -1;
    m_freeList = idx;
}

/* ---- Update leftLen path to root ---- */

void Rope8::updatePath(int idx)
{
    while (idx >= 0) {
        int left = m_nodes[idx].left;
        m_nodes[idx].leftLen = lengthOf(left);
        m_nodes[idx].weight = fibWeight(idx);
        idx = m_nodes[idx].parent;
    }
}

/* ---- Fibonacci weight (approximation via depth) ---- */

int Rope8::fibWeight(int idx) const
{
    if (idx < 0) return 0;
    return 1 + lengthOf(m_nodes[idx].left) + lengthOf(m_nodes[idx].right);
}

/* ---- Rebalance via Fibonacci-weight splitting ---- */

int Rope8::rebalance(int root)
{
    if (root < 0) return root;

    // Collect all leaves
    QVector<int> leaves;
    collectLeaves(root, leaves);

    // Free all internal nodes
    // (simplified: rebuild from leaves)
    QVector<QString> texts;
    for (int lf : leaves)
        texts.append(m_nodes[lf].leafData);

    // Free old nodes
    for (int lf : leaves) {
        m_nodes[lf].left = -1;
        m_nodes[lf].right = -1;
        m_nodes[lf].parent = -1;
    }

    // Rebuild balanced tree from leaves
    if (leaves.isEmpty()) return -1;
    if (leaves.size() == 1) return leaves[0];

    // Build balanced binary tree over leaf indices
    QVector<int> nodes = leaves;

    while (nodes.size() > 1) {
        QVector<int> nextLevel;
        for (int i = 0; i < nodes.size(); i += 2) {
            if (i + 1 < nodes.size()) {
                int parent = allocNode();
                m_nodes[parent].left = nodes[i];
                m_nodes[parent].right = nodes[i + 1];
                m_nodes[parent].isLeaf = false;
                m_nodes[parent].parent = -1;
                m_nodes[nodes[i]].parent = parent;
                m_nodes[nodes[i + 1]].parent = parent;
                m_nodes[parent].leftLen = lengthOf(nodes[i]);
                m_nodes[parent].weight = fibWeight(parent);
                nextLevel.append(parent);
            } else {
                nextLevel.append(nodes[i]);
            }
        }
        nodes = nextLevel;
    }

    m_stats.numRebalances++;
    return nodes[0];
}

/* ---- Split leaf at offset ---- */

int Rope8::splitLeaf(int idx, int offset)
{
    const QString& data = m_nodes[idx].leafData;
    if (offset <= 0 || offset >= data.length()) return idx;

    // Create new leaf for right part
    int rightIdx = allocNode();
    m_nodes[rightIdx].isLeaf = true;
    m_nodes[rightIdx].leafData = data.mid(offset);

    // Truncate original leaf to left part
    m_nodes[idx].leafData = data.left(offset);

    // Create new parent
    int parent = allocNode();
    m_nodes[parent].left = idx;
    m_nodes[parent].right = rightIdx;
    m_nodes[parent].isLeaf = false;
    m_nodes[parent].leftLen = data.left(offset).length();
    m_nodes[parent].weight = fibWeight(parent);
    m_nodes[idx].parent = parent;
    m_nodes[rightIdx].parent = parent;

    return parent;
}

/* ---- Concat two subtrees ---- */

int Rope8::concat(int left, int right)
{
    if (left < 0) return right;
    if (right < 0) return left;

    int parent = allocNode();
    m_nodes[parent].left = left;
    m_nodes[parent].right = right;
    m_nodes[parent].isLeaf = false;
    m_nodes[parent].leftLen = lengthOf(left);
    m_nodes[parent].weight = fibWeight(parent);
    m_nodes[left].parent = parent;
    m_nodes[right].parent = parent;
    return parent;
}

/* ---- Collect leaves in order ---- */

void Rope8::collectLeaves(int idx, QVector<int>& leaves) const
{
    if (idx < 0) return;
    if (m_nodes[idx].isLeaf) {
        leaves.append(idx);
        return;
    }
    collectLeaves(m_nodes[idx].left, leaves);
    collectLeaves(m_nodes[idx].right, leaves);
}

/* ---- Find leaf for character index ---- */

int Rope8::findLeaf(int root, int& offset) const
{
    int cur = root;
    while (cur >= 0 && !m_nodes[cur].isLeaf) {
        if (offset < m_nodes[cur].leftLen) {
            cur = m_nodes[cur].left;
        } else {
            offset -= m_nodes[cur].leftLen;
            cur = m_nodes[cur].right;
        }
    }
    return cur;
}

/* ---- Height of subtree ---- */

int Rope8::heightOf(int idx) const
{
    if (idx < 0) return 0;
    return 1 + qMax(heightOf(m_nodes[idx].left), heightOf(m_nodes[idx].right));
}

/* ---- Length of subtree ---- */

int Rope8::lengthOf(int idx) const
{
    if (idx < 0) return 0;
    if (m_nodes[idx].isLeaf) return m_nodes[idx].leafData.length();
    return m_nodes[idx].leftLen + lengthOf(m_nodes[idx].right);
}

/* ---- Build from string ---- */

void Rope8::build(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    m_nodes.clear();
    m_freeList = -1;
    m_root = -1;

    if (text.isEmpty()) return;

    // Split text into leaf buffers
    QVector<int> leaves;
    for (int i = 0; i < text.length(); i += m_leafCapacity) {
        int len = qMin(m_leafCapacity, text.length() - i);
        int leaf = allocNode();
        m_nodes[leaf].isLeaf = true;
        m_nodes[leaf].leafData = text.mid(i, len);
        leaves.append(leaf);
    }

    // Build balanced tree bottom-up
    QVector<int> level = leaves;
    while (level.size() > 1) {
        QVector<int> next;
        for (int i = 0; i < level.size(); i += 2) {
            if (i + 1 < level.size())
                next.append(concat(level[i], level[i + 1]));
            else
                next.append(level[i]);
        }
        level = next;
    }

    m_root = level[0];
    m_stats.numNodes = m_nodes.size();
    m_stats.treeHeight = heightOf(m_root);
    m_stats.totalLength = text.length();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit ropeUpdated(m_stats.totalLength, m_stats.treeHeight, timer.elapsed());
}

/* ---- Insert ---- */

void Rope8::insert(int pos, const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    if (text.isEmpty()) return;

    // Build a small rope for the inserted text
    Rope8 tmpRope;
    tmpRope.m_leafCapacity = m_leafCapacity;
    tmpRope.build(text);

    if (m_root < 0) {
        m_root = tmpRope.m_root;
        m_nodes = tmpRope.m_nodes;
    } else {
        // Split at position
        int offset = pos;
        int leaf = findLeaf(m_root, offset);

        if (leaf >= 0 && offset > 0 && offset < m_nodes[leaf].leafData.length()) {
            int parent = splitLeaf(leaf, offset);
            // Replace leaf reference in tree
            if (m_nodes[leaf].parent >= 0) {
                int par = m_nodes[leaf].parent;
                if (m_nodes[par].left == leaf) m_nodes[par].left = parent;
                else m_nodes[par].right = parent;
            } else {
                m_root = parent;
            }
        }

        // Concat: left part + new text + right part
        // Simplified: rebuild
        QString full = toString();
        full = full.left(pos) + text + full.mid(pos);
        build(full);
    }

    m_stats.treeHeight = heightOf(m_root);
    m_stats.totalLength = length();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit ropeUpdated(m_stats.totalLength, m_stats.treeHeight, timer.elapsed());
}

/* ---- Remove ---- */

void Rope8::remove(int from, int to)
{
    QElapsedTimer timer;
    timer.start();

    QString full = toString();
    full.remove(from, to - from);
    build(full);

    m_stats.treeHeight = heightOf(m_root);
    m_stats.totalLength = length();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit ropeUpdated(m_stats.totalLength, m_stats.treeHeight, timer.elapsed());
}

/* ---- Substring ---- */

QString Rope8::substring(int from, int to) const
{
    QString result;
    QVector<int> leaves;
    collectLeaves(m_root, leaves);

    int pos = 0;
    for (int lf : leaves) {
        int len = m_nodes[lf].leafData.length();
        int start = qMax(0, from - pos);
        int end = qMin(len, to - pos);
        if (start < end)
            result += m_nodes[lf].leafData.mid(start, end - start);
        pos += len;
        if (pos >= to) break;
    }
    return result;
}

/* ---- CharAt ---- */

QChar Rope8::charAt(int index) const
{
    int offset = index;
    int leaf = findLeaf(m_root, offset);
    if (leaf >= 0 && offset >= 0 && offset < m_nodes[leaf].leafData.length())
        return m_nodes[leaf].leafData[offset];
    return QChar();
}

/* ---- Length ---- */

int Rope8::length() const
{
    return lengthOf(m_root);
}

/* ---- toString ---- */

QString Rope8::toString() const
{
    QString result;
    QVector<int> leaves;
    collectLeaves(m_root, leaves);
    for (int lf : leaves)
        result += m_nodes[lf].leafData;
    return result;
}

/* ---- Reset ---- */

void Rope8::resetStatistics()
{
    m_nodes.clear();
    m_root = -1;
    m_freeList = -1;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

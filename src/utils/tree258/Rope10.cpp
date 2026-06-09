/**
 * @file Rope10.cpp
 * @brief Rope10 实现
 *
 * 实现绳索结构：平衡BST叶索引与字符级分裂合并增量再平衡。
 */

#include "utils/tree258/Rope10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Rope10::Rope10(QObject *parent)
    : QObject(parent) {}
Rope10::~Rope10() = default;

/* ---- Configuration ---- */

void Rope10::setLeafSize(int size) { m_leafSize = qMax(4, size); }
void Rope10::setRebalanceThreshold(int threshold) { m_rebalanceThreshold = qMax(2, threshold); }

/* ---- Node allocation ---- */

int Rope10::allocNode()
{
    int idx;
    if (m_freeList >= 0) {
        idx = m_freeList;
        m_freeList = m_nodes[idx].right;
    } else {
        idx = m_nodes.size();
        m_nodes.append(Node());
    }
    m_nodes[idx] = Node{};
    return idx;
}

void Rope10::freeNode(int idx)
{
    m_nodes[idx].right = m_freeList;
    m_freeList = idx;
}

/* ---- Create leaf node ---- */

int Rope10::createLeaf(const QString& data)
{
    int idx = allocNode();
    m_nodes[idx].isLeaf = true;
    m_nodes[idx].data = data;
    m_nodes[idx].weight = data.length();
    m_nodes[idx].height = 1;
    return idx;
}

/* ---- Update node weight and height ---- */

void Rope10::updateNode(int idx)
{
    if (idx < 0 || idx >= m_nodes.size()) return;
    Node& node = m_nodes[idx];
    if (node.isLeaf) {
        node.weight = node.data.length();
        node.height = 1;
        return;
    }
    int leftH = (node.left >= 0) ? m_nodes[node.left].height : 0;
    int rightH = (node.right >= 0) ? m_nodes[node.right].height : 0;
    node.height = 1 + qMax(leftH, rightH);
    node.weight = (node.left >= 0 ? m_nodes[node.left].weight : 0)
                  + (node.right >= 0 ? m_nodes[node.right].weight : 0);
}

/* ---- Balance factor ---- */

int Rope10::balanceFactor(int idx) const
{
    if (idx < 0) return 0;
    int lh = (m_nodes[idx].left >= 0) ? m_nodes[m_nodes[idx].left].height : 0;
    int rh = (m_nodes[idx].right >= 0) ? m_nodes[m_nodes[idx].right].height : 0;
    return lh - rh;
}

/* ---- Rotations ---- */

int Rope10::rotateLeft(int idx)
{
    int r = m_nodes[idx].right;
    m_nodes[idx].right = m_nodes[r].left;
    m_nodes[r].left = idx;
    updateNode(idx);
    updateNode(r);
    return r;
}

int Rope10::rotateRight(int idx)
{
    int l = m_nodes[idx].left;
    m_nodes[idx].left = m_nodes[l].right;
    m_nodes[l].right = idx;
    updateNode(idx);
    updateNode(l);
    return l;
}

/* ---- Rebalance ---- */

int Rope10::rebalance(int idx)
{
    if (idx < 0) return idx;
    updateNode(idx);
    int bf = balanceFactor(idx);

    if (bf > 1) {
        if (balanceFactor(m_nodes[idx].left) < 0)
            m_nodes[idx].left = rotateLeft(m_nodes[idx].left);
        return rotateRight(idx);
    }
    if (bf < -1) {
        if (balanceFactor(m_nodes[idx].right) > 0)
            m_nodes[idx].right = rotateRight(m_nodes[idx].right);
        return rotateLeft(idx);
    }
    return idx;
}

/* ---- Build from string ---- */

void Rope10::build(const QString& text)
{
    resetStatistics();
    if (text.isEmpty()) return;

    // Split text into leaf chunks
    QVector<int> leaves;
    for (int i = 0; i < text.length(); i += m_leafSize) {
        QString chunk = text.mid(i, m_leafSize);
        leaves.append(createLeaf(chunk));
    }

    // Build balanced BST from leaves bottom-up
    while (leaves.size() > 1) {
        QVector<int> nextLevel;
        for (int i = 0; i < leaves.size(); i += 2) {
            if (i + 1 < leaves.size()) {
                int parent = allocNode();
                m_nodes[parent].left = leaves[i];
                m_nodes[parent].right = leaves[i + 1];
                m_nodes[parent].isLeaf = false;
                updateNode(parent);
                nextLevel.append(rebalance(parent));
            } else {
                nextLevel.append(leaves[i]);
            }
        }
        leaves = nextLevel;
    }
    m_root = leaves.isEmpty() ? -1 : leaves[0];

    m_stats.numNodes = m_nodes.size();
    m_stats.numLeaves = (text.length() + m_leafSize - 1) / m_leafSize;
    m_stats.totalLength = text.length();
}

/* ---- Insert string at position ---- */

void Rope10::insert(int pos, const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    if (text.isEmpty()) return;
    int newLeaf = createLeaf(text);

    if (m_root < 0) {
        m_root = newLeaf;
    } else {
        int left, right;
        splitHelper(m_root, pos, left, right);
        int merged = mergeTrees(left, newLeaf);
        m_root = mergeTrees(merged, right);
        m_root = rebalance(m_root);
    }

    double elapsed = timer.elapsed();
    m_stats.numNodes = m_nodes.size();
    m_stats.totalLength = length();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit ropeUpdated(m_stats.numNodes, m_stats.totalLength, elapsed);
}

/* ---- Remove characters ---- */

void Rope10::remove(int pos, int len)
{
    QElapsedTimer timer;
    timer.start();

    if (len <= 0 || m_root < 0) return;

    int left, mid, right;
    splitHelper(m_root, pos, left, mid);
    splitHelper(mid, len, mid, right);
    // Discard mid (it gets leaked in the node pool for simplicity)
    m_root = mergeTrees(left, right);
    m_root = rebalance(m_root);

    double elapsed = timer.elapsed();
    m_stats.numNodes = m_nodes.size();
    m_stats.totalLength = length();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit ropeUpdated(m_stats.numNodes, m_stats.totalLength, elapsed);
}

/* ---- Character at position ---- */

QChar Rope10::charAt(int pos) const
{
    if (m_root < 0 || pos < 0 || pos >= length()) return QChar();
    return indexInto(m_root, pos);
}

QChar Rope10::indexInto(int idx, int pos) const
{
    if (idx < 0) return QChar();
    const Node& node = m_nodes[idx];
    if (node.isLeaf) return node.data[pos];

    int leftWeight = (node.left >= 0) ? m_nodes[node.left].weight : 0;
    if (pos < leftWeight)
        return indexInto(node.left, pos);
    return indexInto(node.right, pos - leftWeight);
}

/* ---- Substring ---- */

QString Rope10::substring(int start, int len) const
{
    QString result;
    result.reserve(len);
    for (int i = 0; i < len; ++i) {
        if (start + i >= length()) break;
        result.append(charAt(start + i));
    }
    return result;
}

/* ---- Split subtree at position ---- */

void Rope10::splitHelper(int root, int pos, int& left, int& right)
{
    if (root < 0) { left = -1; right = -1; return; }

    const Node& node = m_nodes[root];
    if (node.isLeaf) {
        if (pos <= 0) {
            left = -1;
            right = root;
        } else if (pos >= node.data.length()) {
            left = root;
            right = -1;
        } else {
            int lLeaf = createLeaf(node.data.left(pos));
            int rLeaf = createLeaf(node.data.mid(pos));
            left = lLeaf;
            right = rLeaf;
            freeNode(root);
        }
        return;
    }

    int leftWeight = (node.left >= 0) ? m_nodes[node.left].weight : 0;
    int origLeft = node.left;
    int origRight = node.right;

    if (pos < leftWeight) {
        int subLeft, subRight;
        splitHelper(origLeft, pos, subLeft, subRight);
        left = subLeft;
        right = allocNode();
        m_nodes[right].left = subRight;
        m_nodes[right].right = origRight;
        m_nodes[right].isLeaf = false;
        updateNode(right);
        right = rebalance(right);
    } else if (pos > leftWeight) {
        int subLeft, subRight;
        splitHelper(origRight, pos - leftWeight, subLeft, subRight);
        left = allocNode();
        m_nodes[left].left = origLeft;
        m_nodes[left].right = subLeft;
        m_nodes[left].isLeaf = false;
        updateNode(left);
        left = rebalance(left);
        right = subRight;
    } else {
        left = origLeft;
        right = origRight;
    }
    freeNode(root);
}

/* ---- Merge two subtrees ---- */

int Rope10::mergeTrees(int left, int right)
{
    if (left < 0) return right;
    if (right < 0) return left;

    int parent = allocNode();
    m_nodes[parent].left = left;
    m_nodes[parent].right = right;
    m_nodes[parent].isLeaf = false;
    updateNode(parent);
    return rebalance(parent);
}

/* ---- Report subtree ---- */

void Rope10::reportSubtree(int idx, QString& result) const
{
    if (idx < 0) return;
    const Node& node = m_nodes[idx];
    if (node.isLeaf) {
        result.append(node.data);
        return;
    }
    reportSubtree(node.left, result);
    reportSubtree(node.right, result);
}

/* ---- Split (public) ---- */

Rope10* Rope10::split(int pos)
{
    int left, right;
    splitHelper(m_root, pos, left, right);

    m_root = left;
    Rope10* rightRope = new Rope10(parent());
    if (right >= 0) {
        QString content;
        rightRope->reportSubtree(right, content);
        // Rebuild to avoid node pool conflicts
        rightRope->build(content);
    }
    m_stats.numSplits++;
    return rightRope;
}

/* ---- Concat ---- */

void Rope10::concat(Rope10* other)
{
    if (!other || other->m_root < 0) return;

    QString leftContent = toString();
    QString rightContent = other->toString();
    build(leftContent + rightContent);

    m_stats.numConcats++;
}

/* ---- To string ---- */

QString Rope10::toString() const
{
    QString result;
    result.reserve(length());
    reportSubtree(m_root, result);
    return result;
}

/* ---- Length ---- */

int Rope10::length() const
{
    return (m_root >= 0) ? m_nodes[m_root].weight : 0;
}

/* ---- Reset ---- */

void Rope10::resetStatistics()
{
    m_nodes.clear();
    m_root = -1;
    m_freeList = -1;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

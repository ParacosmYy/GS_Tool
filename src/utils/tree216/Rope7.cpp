/**
 * @file Rope7.cpp
 * @brief Rope7 实现
 *
 * 实现绳索数据结构：手指搜索、AVL增量重平衡、拼接分割。
 */

#include "utils/tree216/Rope7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Rope7::Rope7(QObject *parent) : QObject(parent) {}
Rope7::~Rope7() = default;

/* ---- Allocate node ---- */

int Rope7::allocNode()
{
    m_nodes.append(Node{});
    return m_nodes.size() - 1;
}

/* ---- Create leaf node ---- */

int Rope7::createLeaf(const QString& text)
{
    int n = allocNode();
    m_nodes[n].leaf = text;
    m_nodes[n].weight = text.length();
    m_nodes[n].totalLen = text.length();
    m_nodes[n].isLeaf = true;
    m_nodes[n].height = 1;
    return n;
}

/* ---- Update aggregates ---- */

void Rope7::update(int node)
{
    if (node < 0 || node >= m_nodes.size()) return;
    if (m_nodes[node].isLeaf) {
        m_nodes[node].totalLen = m_nodes[node].leaf.length();
        m_nodes[node].weight = m_nodes[node].totalLen;
        return;
    }

    m_nodes[node].totalLen = 0;
    m_nodes[node].weight = 0;
    m_nodes[node].height = 1;

    if (m_nodes[node].left >= 0) {
        m_nodes[node].weight = m_nodes[m_nodes[node].left].totalLen;
        m_nodes[node].totalLen += m_nodes[m_nodes[node].left].totalLen;
        m_nodes[node].height = qMax(m_nodes[node].height,
                                     m_nodes[m_nodes[node].left].height + 1);
    }
    if (m_nodes[node].right >= 0) {
        m_nodes[node].totalLen += m_nodes[m_nodes[node].right].totalLen;
        m_nodes[node].height = qMax(m_nodes[node].height,
                                     m_nodes[m_nodes[node].right].height + 1);
    }
}

/* ---- Balance factor ---- */

int Rope7::balanceFactor(int node) const
{
    if (node < 0 || node >= m_nodes.size()) return 0;
    int lh = (m_nodes[node].left >= 0) ? m_nodes[m_nodes[node].left].height : 0;
    int rh = (m_nodes[node].right >= 0) ? m_nodes[m_nodes[node].right].height : 0;
    return lh - rh;
}

/* ---- Rotate right ---- */

int Rope7::rotateRight(int y)
{
    int x = m_nodes[y].left;
    m_nodes[y].left = m_nodes[x].right;
    m_nodes[x].right = y;
    update(y);
    update(x);
    return x;
}

/* ---- Rotate left ---- */

int Rope7::rotateLeft(int x)
{
    int y = m_nodes[x].right;
    m_nodes[x].right = m_nodes[y].left;
    m_nodes[y].left = x;
    update(x);
    update(y);
    return y;
}

/* ---- Rebalance node ---- */

int Rope7::rebalanceNode(int node)
{
    update(node);
    int bf = balanceFactor(node);

    if (bf > 1) {
        if (balanceFactor(m_nodes[node].left) < 0)
            m_nodes[node].left = rotateLeft(m_nodes[node].left);
        return rotateRight(node);
    }
    if (bf < -1) {
        if (balanceFactor(m_nodes[node].right) > 0)
            m_nodes[node].right = rotateRight(m_nodes[node].right);
        return rotateLeft(node);
    }
    return node;
}

/* ---- Build ---- */

void Rope7::build(const QString& text, int leafSize)
{
    QElapsedTimer timer;
    timer.start();

    m_nodes.clear();
    m_root = -1;
    m_finger = -1;
    int ls = qMax(1, leafSize);

    if (text.isEmpty()) { m_stats.ropeLength = 0; return; }

    QVector<int> leaves;
    for (int i = 0; i < text.length(); i += ls) {
        int len = qMin(ls, text.length() - i);
        leaves.append(createLeaf(text.mid(i, len)));
    }

    // Build balanced tree bottom-up
    while (leaves.size() > 1) {
        QVector<int> next;
        for (int i = 0; i < leaves.size(); i += 2) {
            if (i + 1 < leaves.size()) {
                int parent = allocNode();
                m_nodes[parent].left = leaves[i];
                m_nodes[parent].right = leaves[i + 1];
                update(parent);
                next.append(parent);
            } else {
                next.append(leaves[i]);
            }
        }
        leaves = next;
    }
    m_root = leaves[0];

    m_stats.ropeLength = text.length();
    m_stats.numLeaves = (text.length() + ls - 1) / ls;
    m_stats.treeHeight = m_nodes[m_root].height;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Find position using finger search ---- */

QPair<int, int> Rope7::findPosition(int pos) const
{
    int node = m_root;
    int offset = pos;

    // Try finger search optimization
    if (m_finger >= 0 && m_finger < m_nodes.size()) {
        // Check if finger is close to target
        int fingerOff = pos - m_fingerPos;
        if (qAbs(fingerOff) < 32 && m_nodes[m_finger].isLeaf) {
            int leafLen = m_nodes[m_finger].leaf.length();
            if (offset >= m_fingerPos && offset < m_fingerPos + leafLen) {
                return {m_finger, offset - m_fingerPos};
            }
        }
    }

    // Standard search from root
    while (node >= 0 && node < m_nodes.size() && !m_nodes[node].isLeaf) {
        int leftLen = (m_nodes[node].left >= 0) ? m_nodes[m_nodes[node].left].totalLen : 0;
        if (offset < leftLen) {
            node = m_nodes[node].left;
        } else {
            offset -= leftLen;
            node = m_nodes[node].right;
        }
    }

    const_cast<Rope7*>(this)->m_finger = node;
    const_cast<Rope7*>(this)->m_fingerPos = pos - offset;
    return {node, offset};
}

/* ---- Character at index ---- */

QChar Rope7::at(int index) const
{
    if (m_root < 0 || index < 0 || index >= length()) return QChar();
    auto [node, off] = findPosition(index);
    if (node >= 0 && off < m_nodes[node].leaf.length())
        return m_nodes[node].leaf[off];
    return QChar();
}

/* ---- Substring ---- */

QString Rope7::mid(int start, int length) const
{
    if (m_root < 0 || start < 0) return {};
    QString result;
    result.reserve(length);
    for (int i = 0; i < length && (start + i) < Rope7::length(); ++i)
        result += at(start + i);
    return result;
}

/* ---- Split node ---- */

QPair<int, int> Rope7::splitNode(int node, int pos)
{
    if (node < 0 || node >= m_nodes.size()) return {-1, -1};
    if (m_nodes[node].isLeaf) {
        if (pos <= 0) return {-1, node};
        if (pos >= m_nodes[node].leaf.length()) return {node, -1};
        int l = createLeaf(m_nodes[node].leaf.left(pos));
        int r = createLeaf(m_nodes[node].leaf.mid(pos));
        return {l, r};
    }

    int leftLen = (m_nodes[node].left >= 0) ? m_nodes[m_nodes[node].left].totalLen : 0;

    if (pos <= leftLen) {
        auto [sl, sr] = splitNode(m_nodes[node].left, pos);
        int rightPart = mergeNodes(sr, m_nodes[node].right);
        return {sl, rightPart};
    } else {
        auto [sl, sr] = splitNode(m_nodes[node].right, pos - leftLen);
        int leftPart = mergeNodes(m_nodes[node].left, sl);
        return {leftPart, sr};
    }
}

/* ---- Merge nodes ---- */

int Rope7::mergeNodes(int left, int right)
{
    if (left < 0) return right;
    if (right < 0) return left;

    int parent = allocNode();
    m_nodes[parent].left = left;
    m_nodes[parent].right = right;
    return rebalanceNode(parent);
}

/* ---- Insert ---- */

void Rope7::insert(int pos, const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    if (text.isEmpty()) return;

    // Create a small rope for the inserted text
    int insertTree = -1;
    for (int i = 0; i < text.length(); i += MAX_LEAF) {
        int len = qMin(MAX_LEAF, text.length() - i);
        int leaf = createLeaf(text.mid(i, len));
        insertTree = mergeNodes(insertTree, leaf);
    }

    if (m_root < 0) {
        m_root = insertTree;
    } else {
        auto [left, right] = splitNode(m_root, pos);
        m_root = mergeNodes(mergeNodes(left, insertTree), right);
    }

    m_stats.ropeLength = length();
    m_stats.treeHeight = (m_root >= 0) ? m_nodes[m_root].height : 0;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("insert", m_stats.ropeLength, timer.elapsed());
}

/* ---- Remove ---- */

void Rope7::remove(int start, int length)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root < 0 || length <= 0) return;

    auto [left, midRight] = splitNode(m_root, start);
    auto [mid, right] = splitNode(midRight, length);
    // Discard mid
    m_root = mergeNodes(left, right);

    m_stats.ropeLength = Rope7::length();
    m_stats.treeHeight = (m_root >= 0) ? m_nodes[m_root].height : 0;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("remove", m_stats.ropeLength, timer.elapsed());
}

/* ---- Concat ---- */

void Rope7::concat(const Rope7& other)
{
    QElapsedTimer timer;
    timer.start();
    m_root = mergeNodes(m_root, other.m_root);
    m_stats.ropeLength = length();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Split ---- */

Rope7* Rope7::split(int pos)
{
    QElapsedTimer timer;
    timer.start();

    auto [left, right] = splitNode(m_root, pos);
    m_root = left;

    Rope7* rightRope = new Rope7(parent());
    rightRope->m_root = right;
    rightRope->m_nodes = m_nodes;  // Share node pool
    rightRope->m_stats.ropeLength = rightRope->length();

    m_stats.ropeLength = length();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("split", m_stats.ropeLength, timer.elapsed());
    return rightRope;
}

/* ---- Length ---- */

int Rope7::length() const
{
    return (m_root >= 0 && m_root < m_nodes.size())
        ? m_nodes[m_root].totalLen : 0;
}

/* ---- Collect leaves ---- */

void Rope7::collectLeaves(int node, QVector<QString>& leaves) const
{
    if (node < 0 || node >= m_nodes.size()) return;
    if (m_nodes[node].isLeaf) {
        leaves.append(m_nodes[node].leaf);
        return;
    }
    collectLeaves(m_nodes[node].left, leaves);
    collectLeaves(m_nodes[node].right, leaves);
}

/* ---- To string ---- */

QString Rope7::toString() const
{
    QVector<QString> leaves;
    collectLeaves(m_root, leaves);
    QString result;
    for (const auto& l : leaves) result += l;
    return result;
}

/* ---- Rebalance ---- */

void Rope7::rebalance()
{
    QElapsedTimer timer;
    timer.start();

    QVector<QString> leaves;
    collectLeaves(m_root, leaves);

    // Rebuild balanced
    QVector<int> nodes;
    for (const auto& l : leaves) nodes.append(createLeaf(l));

    while (nodes.size() > 1) {
        QVector<int> next;
        for (int i = 0; i < nodes.size(); i += 2) {
            if (i + 1 < nodes.size()) {
                int p = allocNode();
                m_nodes[p].left = nodes[i];
                m_nodes[p].right = nodes[i + 1];
                update(p);
                next.append(p);
            } else {
                next.append(nodes[i]);
            }
        }
        nodes = next;
    }
    m_root = nodes.isEmpty() ? -1 : nodes[0];

    m_stats.rebalanceCount++;
    m_stats.treeHeight = (m_root >= 0) ? m_nodes[m_root].height : 0;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("rebalance", m_stats.ropeLength, timer.elapsed());
}

/* ---- Reset ---- */

void Rope7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_root = -1;
    m_finger = -1;
    m_fingerPos = -1;
}

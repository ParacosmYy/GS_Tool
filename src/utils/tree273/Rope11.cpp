/**
 * @file Rope11.cpp
 * @brief Rope11 实现
 *
 * 实现绳索结构：平衡指树与惰性连接高效持久化字符串操作。
 */

#include "utils/tree273/Rope11.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Rope11::Rope11(QObject *parent)
    : QObject(parent) {}

Rope11::~Rope11() = default;

/* ---- Configuration ---- */

void Rope11::setLeafSize(int maxSize)
{
    m_leafSize = qBound(8, maxSize, 4096);
}

/* ---- Node allocation ---- */

int Rope11::allocLeaf(const QString& data)
{
    int idx = m_nodes.size();
    m_nodes.append(RopeNode{});
    m_nodes[idx].isLeaf = true;
    m_nodes[idx].data = data;
    m_nodes[idx].weight = data.size();
    m_nodes[idx].height = 1;
    return idx;
}

int Rope11::allocInternal(int left, int right)
{
    int idx = m_nodes.size();
    m_nodes.append(RopeNode{});
    m_nodes[idx].isLeaf = false;
    m_nodes[idx].left = left;
    m_nodes[idx].right = right;
    m_nodes[idx].weight = computeWeight(left);
    m_nodes[idx].height = 1 + qMax(nodeHeight(left), nodeHeight(right));

    if (left >= 0) m_nodes[left].parent = idx;
    if (right >= 0) m_nodes[right].parent = idx;
    return idx;
}

/* ---- Weight and height helpers ---- */

int Rope11::computeWeight(int nodeIdx) const
{
    if (nodeIdx < 0) return 0;
    const auto& node = m_nodes[nodeIdx];
    if (node.isLeaf) return node.data.size();

    int w = 0;
    int cur = node.left;
    while (cur >= 0) {
        const auto& n = m_nodes[cur];
        if (n.isLeaf) { w += n.data.size(); break; }
        w += n.weight;
        cur = n.left; // full left weight
    }
    // Simpler: recursively compute left subtree
    w = 0;
    QVector<int> stack;
    stack.append(node.left);
    while (!stack.isEmpty()) {
        int ci = stack.takeLast();
        if (ci < 0) continue;
        if (m_nodes[ci].isLeaf) { w += m_nodes[ci].data.size(); }
        else { w += m_nodes[ci].weight; }
    }
    return w;
}

int Rope11::nodeHeight(int nodeIdx) const
{
    return (nodeIdx >= 0) ? m_nodes[nodeIdx].height : 0;
}

int Rope11::balanceFactor(int nodeIdx) const
{
    if (nodeIdx < 0 || m_nodes[nodeIdx].isLeaf) return 0;
    return nodeHeight(m_nodes[nodeIdx].left) - nodeHeight(m_nodes[nodeIdx].right);
}

/* ---- Path update (height + weight) ---- */

void Rope11::updatePath(int nodeIdx)
{
    int cur = nodeIdx;
    while (cur >= 0) {
        auto& node = m_nodes[cur];
        if (!node.isLeaf) {
            node.height = 1 + qMax(nodeHeight(node.left), nodeHeight(node.right));
            node.weight = computeWeight(cur);
        }
        cur = node.parent;
    }
}

/* ---- AVL rotations ---- */

int Rope11::rotateRight(int y)
{
    int x = m_nodes[y].left;
    m_nodes[y].left = m_nodes[x].right;
    if (m_nodes[x].right >= 0) m_nodes[m_nodes[x].right].parent = y;
    m_nodes[x].right = y;
    m_nodes[x].parent = m_nodes[y].parent;
    m_nodes[y].parent = x;

    m_nodes[y].height = 1 + qMax(nodeHeight(m_nodes[y].left), nodeHeight(m_nodes[y].right));
    m_nodes[x].height = 1 + qMax(nodeHeight(m_nodes[x].left), nodeHeight(m_nodes[x].right));
    m_nodes[y].weight = computeWeight(y);
    m_nodes[x].weight = computeWeight(x);
    return x;
}

int Rope11::rotateLeft(int x)
{
    int y = m_nodes[x].right;
    m_nodes[x].right = m_nodes[y].left;
    if (m_nodes[y].left >= 0) m_nodes[m_nodes[y].left].parent = x;
    m_nodes[y].left = x;
    m_nodes[y].parent = m_nodes[x].parent;
    m_nodes[x].parent = y;

    m_nodes[x].height = 1 + qMax(nodeHeight(m_nodes[x].left), nodeHeight(m_nodes[x].right));
    m_nodes[y].height = 1 + qMax(nodeHeight(m_nodes[y].left), nodeHeight(m_nodes[y].right));
    m_nodes[x].weight = computeWeight(x);
    m_nodes[y].weight = computeWeight(y);
    return y;
}

/* ---- Rebalance ---- */

int Rope11::rebalance(int nodeIdx)
{
    int bf = balanceFactor(nodeIdx);

    if (bf > 1) {
        if (balanceFactor(m_nodes[nodeIdx].left) < 0)
            m_nodes[nodeIdx].left = rotateLeft(m_nodes[nodeIdx].left);
        return rotateRight(nodeIdx);
    }
    if (bf < -1) {
        if (balanceFactor(m_nodes[nodeIdx].right) > 0)
            m_nodes[nodeIdx].right = rotateRight(m_nodes[nodeIdx].right);
        return rotateLeft(nodeIdx);
    }
    return nodeIdx;
}

/* ---- Force evaluate lazy node ---- */

void Rope11::forceEval(int nodeIdx)
{
    if (nodeIdx < 0) return;
    auto& node = m_nodes[nodeIdx];
    if (!node.isLazy) return;
    node.isLazy = false;
    // Lazy concat already has left/right set; nothing extra needed
}

/* ---- Character access ---- */

QChar Rope11::at(int index) const
{
    if (index < 0 || m_root < 0) return QChar();

    int cur = m_root;
    int pos = index;

    while (cur >= 0) {
        const auto& node = m_nodes[cur];
        if (node.isLeaf) {
            if (pos < node.data.size()) return node.data[pos];
            return QChar();
        }

        forceEval(cur);

        if (pos < node.weight) {
            cur = node.left;
        } else {
            pos -= node.weight;
            cur = node.right;
        }
    }
    return QChar();
}

/* ---- Substring extraction ---- */

QString Rope11::mid(int start, int length) const
{
    if (start < 0 || length <= 0 || m_root < 0) return {};

    QString result;
    result.reserve(length);

    for (int i = 0; i < length; ++i) {
        QChar ch = at(start + i);
        if (ch.isNull()) break;
        result.append(ch);
    }
    return result;
}

/* ---- Split rope at position ---- */

QPair<int, int> Rope11::splitAt(int nodeIdx, int pos)
{
    if (nodeIdx < 0) return {-1, -1};
    auto& node = m_nodes[nodeIdx];
    forceEval(nodeIdx);

    if (node.isLeaf) {
        int leftIdx = allocLeaf(node.data.left(pos));
        int rightIdx = allocLeaf(node.data.mid(pos));
        return {leftIdx, rightIdx};
    }

    if (pos <= node.weight) {
        auto [l1, l2] = splitAt(node.left, pos);
        int newLeft = l1;
        int newRight = allocInternal(l2, node.right);
        return {newLeft, newRight};
    } else {
        auto [r1, r2] = splitAt(node.right, pos - node.weight);
        int newLeft = allocInternal(node.left, r1);
        int newRight = r2;
        return {newLeft, newRight};
    }
}

/* ---- Build from string ---- */

void Rope11::build(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    m_nodes.clear();
    m_root = -1;

    int n = text.size();
    if (n == 0) return;

    // Split into leaf-sized chunks and build balanced tree bottom-up
    QVector<int> leaves;
    for (int i = 0; i < n; i += m_leafSize) {
        int len = qMin(m_leafSize, n - i);
        leaves.append(allocLeaf(text.mid(i, len)));
    }

    // Build balanced binary tree bottom-up
    while (leaves.size() > 1) {
        QVector<int> nextLevel;
        for (int i = 0; i < leaves.size(); i += 2) {
            if (i + 1 < leaves.size())
                nextLevel.append(allocInternal(leaves[i], leaves[i + 1]));
            else
                nextLevel.append(leaves[i]);
        }
        leaves = nextLevel;
    }
    m_root = leaves[0];

    double elapsed = timer.elapsed();
    m_stats.totalLength = n;
    m_stats.numLeaves = 0;
    m_stats.numNodes = m_nodes.size();
    for (const auto& nd : m_nodes) { if (nd.isLeaf) m_stats.numLeaves++; }
    m_stats.treeDepth = nodeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationDone(n, m_stats.numNodes, m_stats.treeDepth, elapsed);
}

/* ---- Insert ---- */

void Rope11::insert(int pos, const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    if (text.isEmpty()) return;
    if (m_root < 0) { build(text); return; }

    // Build a small rope from inserted text
    Rope11 tempRope;
    tempRope.setLeafSize(m_leafSize);
    tempRope.build(text);

    auto [left, right] = splitAt(m_root, pos);
    int merged = allocInternal(left, tempRope.m_root);
    m_root = allocInternal(merged, right);
    m_root = rebalance(m_root);
    updatePath(m_root);

    double elapsed = timer.elapsed();
    m_stats.totalLength = size();
    m_stats.numNodes = m_nodes.size();
    m_stats.treeDepth = nodeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationDone(m_stats.totalLength, m_stats.numNodes, m_stats.treeDepth, elapsed);
}

/* ---- Remove ---- */

void Rope11::remove(int start, int length)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root < 0 || length <= 0) return;

    auto [left, midRight] = splitAt(m_root, start);
    auto [mid, right] = splitAt(midRight, length);
    Q_UNUSED(mid);

    if (left >= 0 && right >= 0) {
        m_root = allocInternal(left, right);
        m_root = rebalance(m_root);
        updatePath(m_root);
    } else {
        m_root = (left >= 0) ? left : right;
    }

    double elapsed = timer.elapsed();
    m_stats.totalLength = size();
    m_stats.numNodes = m_nodes.size();
    m_stats.treeDepth = nodeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationDone(m_stats.totalLength, m_stats.numNodes, m_stats.treeDepth, elapsed);
}

/* ---- Concat (lazy) ---- */

void Rope11::concat(Rope11& other)
{
    QElapsedTimer timer;
    timer.start();

    if (other.m_root < 0) return;
    if (m_root < 0) { m_root = other.m_root; return; }

    // Lazy concatenation: just create a new root without rebalancing immediately
    int newRoot = allocInternal(m_root, other.m_root);
    m_nodes[newRoot].isLazy = true;
    m_root = newRoot;

    // Rebalance on demand
    forceEval(m_root);
    m_root = rebalance(m_root);
    updatePath(m_root);

    double elapsed = timer.elapsed();
    m_stats.totalLength = size();
    m_stats.numNodes = m_nodes.size();
    m_stats.treeDepth = nodeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationDone(m_stats.totalLength, m_stats.numNodes, m_stats.treeDepth, elapsed);
}

/* ---- Collect leaves ---- */

void Rope11::collectLeaves(int nodeIdx, QString& result) const
{
    if (nodeIdx < 0) return;
    const auto& node = m_nodes[nodeIdx];
    if (node.isLeaf) {
        result.append(node.data);
        return;
    }
    collectLeaves(node.left, result);
    collectLeaves(node.right, result);
}

/* ---- Size / toString ---- */

int Rope11::size() const
{
    if (m_root < 0) return 0;
    const auto& node = m_nodes[m_root];
    if (node.isLeaf) return node.data.size();
    return node.weight + (node.right >= 0 ? computeWeight(node.right) : 0);
}

QString Rope11::toString() const
{
    QString result;
    collectLeaves(m_root, result);
    return result;
}

/* ---- Reset ---- */

void Rope11::resetStatistics()
{
    m_nodes.clear();
    m_root = -1;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

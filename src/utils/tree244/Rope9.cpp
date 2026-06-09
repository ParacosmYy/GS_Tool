/**
 * @file Rope9.cpp
 * @brief Rope9 实现
 *
 * 实现绳索数据结构：叶节点字符串拼接与Fibonacci权重分裂准则再平衡。
 */

#include "utils/tree244/Rope9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Rope9::Rope9(QObject *parent) : QObject(parent) {}
Rope9::~Rope9() = default;

/* ---- Allocate node ---- */

int Rope9::allocNode()
{
    Node n;
    m_nodes.append(n);
    return m_nodes.size() - 1;
}

/* ---- Update weight from children ---- */

void Rope9::updateWeight(int idx)
{
    if (idx < 0 || m_nodes[idx].isLeaf) return;
    m_nodes[idx].weight = subtreeLen(m_nodes[idx].left);
}

/* ---- Subtree total length ---- */

int Rope9::subtreeLen(int idx) const
{
    if (idx < 0) return 0;
    if (m_nodes[idx].isLeaf) return m_nodes[idx].leafData.length();
    return subtreeLen(m_nodes[idx].left) + subtreeLen(m_nodes[idx].right);
}

/* ---- Fibonacci number ---- */

int Rope9::fibonacci(int n)
{
    // Precomputed Fibonacci numbers for balance checking
    static const int fibs[] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89,
        144, 233, 377, 610, 987, 1597, 2584, 4181, 6765, 10946, 17711,
        28657, 46368, 75025, 121393, 196418, 317811, 514229, 832040};
    if (n >= 0 && n < 31) return fibs[n];
    // Fallback for larger values
    int a = 0, b = 1;
    for (int i = 2; i <= n; ++i) { int c = a + b; a = b; b = c; }
    return b;
}

/* ---- Build recursive ---- */

int Rope9::buildRecursive(const QString& text, int start, int end)
{
    int len = end - start;
    if (len <= 0) return -1;

    if (len <= m_leafSize) {
        int idx = allocNode();
        m_nodes[idx].isLeaf = true;
        m_nodes[idx].leafData = text.mid(start, len);
        m_nodes[idx].weight = len;
        return idx;
    }

    int mid = start + len / 2;
    int leftChild = buildRecursive(text, start, mid);
    int rightChild = buildRecursive(text, mid, end);

    int idx = allocNode();
    m_nodes[idx].isLeaf = false;
    m_nodes[idx].left = leftChild;
    m_nodes[idx].right = rightChild;
    m_nodes[idx].weight = subtreeLen(leftChild);

    if (leftChild >= 0) m_nodes[leftChild].parent = idx;
    if (rightChild >= 0) m_nodes[rightChild].parent = idx;

    return idx;
}

/* ---- Collect leaves in order ---- */

void Rope9::collectLeaves(int idx, QVector<QString>& leaves) const
{
    if (idx < 0) return;
    if (m_nodes[idx].isLeaf) {
        if (!m_nodes[idx].leafData.isEmpty())
            leaves.append(m_nodes[idx].leafData);
        return;
    }
    collectLeaves(m_nodes[idx].left, leaves);
    collectLeaves(m_nodes[idx].right, leaves);
}

/* ---- Check Fibonacci balance ---- */

bool Rope9::isBalanced(int idx) const
{
    if (idx < 0 || m_nodes[idx].isLeaf) return true;
    int len = subtreeLen(idx);
    // Find h such that fib(h+2) >= len
    int h = 0;
    while (fibonacci(h + 2) < len) h++;
    // A rope of length len should have height <= h
    // Check recursively
    int leftH = 0, rightH = 0;
    int cur = m_nodes[idx].left;
    while (cur >= 0 && !m_nodes[cur].isLeaf) { leftH++; cur = m_nodes[cur].right; }
    cur = m_nodes[idx].right;
    while (cur >= 0 && !m_nodes[cur].isLeaf) { rightH++; cur = m_nodes[cur].right; }

    // Fibonacci-weighted criterion: max(leftH, rightH) should be bounded
    int maxHeight = h + 2;  // allow some slack
    return qMax(leftH, rightH) <= maxHeight;
}

/* ---- Build from leaves ---- */

int Rope9::buildFromLeaves(const QVector<QString>& leaves, int start, int end)
{
    int count = end - start;
    if (count <= 0) return -1;
    if (count == 1) {
        int idx = allocNode();
        m_nodes[idx].isLeaf = true;
        m_nodes[idx].leafData = leaves[start];
        m_nodes[idx].weight = leaves[start].length();
        return idx;
    }
    int mid = start + count / 2;
    int leftChild = buildFromLeaves(leaves, start, mid);
    int rightChild = buildFromLeaves(leaves, mid, end);

    int idx = allocNode();
    m_nodes[idx].isLeaf = false;
    m_nodes[idx].left = leftChild;
    m_nodes[idx].right = rightChild;
    m_nodes[idx].weight = subtreeLen(leftChild);
    if (leftChild >= 0) m_nodes[leftChild].parent = idx;
    if (rightChild >= 0) m_nodes[rightChild].parent = idx;
    return idx;
}

/* ---- Build ---- */

void Rope9::build(const QString& text, int leafSize)
{
    QElapsedTimer timer;
    timer.start();

    m_leafSize = qMax(4, leafSize);
    m_nodes.clear();

    if (text.isEmpty()) { m_root = -1; return; }
    m_root = buildRecursive(text, 0, text.length());

    m_stats.numNodes = m_nodes.size();
    m_stats.totalLength = text.length();
    m_stats.treeHeight = 0;
    int cur = m_root;
    while (cur >= 0 && !m_nodes[cur].isLeaf) { m_stats.treeHeight++; cur = m_nodes[cur].left; }
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Concat ---- */

void Rope9::concat(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    if (text.isEmpty()) return;

    int rightIdx = buildRecursive(text, 0, text.length());
    if (m_root < 0) {
        m_root = rightIdx;
    } else {
        int newRoot = allocNode();
        m_nodes[newRoot].isLeaf = false;
        m_nodes[newRoot].left = m_root;
        m_nodes[newRoot].right = rightIdx;
        m_nodes[newRoot].weight = subtreeLen(m_root);
        if (m_root >= 0) m_nodes[m_root].parent = newRoot;
        if (rightIdx >= 0) m_nodes[rightIdx].parent = newRoot;
        m_root = newRoot;
    }

    // Check balance and rebalance if needed
    if (!isBalanced(m_root)) rebalance();

    m_stats.totalLength = subtreeLen(m_root);
    m_stats.numNodes = m_nodes.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("concat", text.length(), m_stats.treeHeight, timer.elapsed());
}

/* ---- Split ---- */

QString Rope9::split(int pos)
{
    QElapsedTimer timer;
    timer.start();

    QString fullStr = toString();
    if (pos <= 0 || pos >= fullStr.length()) return QString();

    QString rightPart = fullStr.mid(pos);
    // Rebuild with left part
    build(fullStr.left(pos), m_leafSize);

    emit operationCompleted("split", pos, m_stats.treeHeight, timer.elapsed());
    return rightPart;
}

/* ---- Insert ---- */

void Rope9::insert(int pos, const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    if (text.isEmpty()) return;
    QString fullStr = toString();
    pos = qBound(0, pos, fullStr.length());
    fullStr = fullStr.left(pos) + text + fullStr.mid(pos);
    build(fullStr, m_leafSize);

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("insert", text.length(), m_stats.treeHeight, timer.elapsed());
}

/* ---- Remove ---- */

void Rope9::remove(int from, int to)
{
    QElapsedTimer timer;
    timer.start();

    QString fullStr = toString();
    from = qBound(0, from, fullStr.length());
    to = qBound(from, to, fullStr.length());
    fullStr = fullStr.left(from) + fullStr.mid(to);
    build(fullStr, m_leafSize);

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("remove", to - from, m_stats.treeHeight, timer.elapsed());
}

/* ---- Character at position ---- */

QChar Rope9::at(int pos) const
{
    int cur = m_root;
    while (cur >= 0) {
        if (m_nodes[cur].isLeaf) {
            if (pos >= 0 && pos < m_nodes[cur].leafData.length())
                return m_nodes[cur].leafData[pos];
            return QChar();
        }
        int w = m_nodes[cur].weight;
        if (pos < w) {
            cur = m_nodes[cur].left;
        } else {
            pos -= w;
            cur = m_nodes[cur].right;
        }
    }
    return QChar();
}

/* ---- Substring ---- */

QString Rope9::substring(int from, int to) const
{
    QString result;
    for (int i = from; i < to && i < length(); ++i)
        result += at(i);
    return result;
}

/* ---- To string ---- */

QString Rope9::toString() const
{
    QVector<QString> leaves;
    collectLeaves(m_root, leaves);
    QString result;
    for (const auto& s : leaves) result += s;
    return result;
}

/* ---- Length ---- */

int Rope9::length() const { return subtreeLen(m_root); }

/* ---- Rebalance ---- */

void Rope9::rebalance()
{
    QElapsedTimer timer;
    timer.start();

    QVector<QString> leaves;
    collectLeaves(m_root, leaves);
    m_nodes.clear();
    m_root = buildFromLeaves(leaves, 0, leaves.size());

    m_stats.numNodes = m_nodes.size();
    m_stats.numRebalances++;
    m_stats.totalLength = subtreeLen(m_root);

    m_timeSum += timer.elapsed();
}

/* ---- Reset ---- */

void Rope9::resetStatistics()
{
    m_nodes.clear();
    m_root = -1;
    m_stats = Stats{}; m_timeSum = 0.0;
}

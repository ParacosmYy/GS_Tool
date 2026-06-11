/**
 * @file Rope12.cpp
 * @brief Rope12 实现
 *
 * 实现绳索数据结构：权重平衡树与分裂-拼接优化的撤销支持文本编辑。
 */

#include "utils/tree287/Rope12.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Rope12::Rope12(int leafCapacity, QObject *parent)
    : QObject(parent), m_leafCap(qBound(8, leafCapacity, 4096))
{}

Rope12::~Rope12() = default;

/* ---- Configuration ---- */

void Rope12::setLeafCapacity(int cap) { m_leafCap = qBound(8, cap, 4096); }
void Rope12::setMaxUndoLevels(int levels) { m_maxUndo = qBound(0, levels, 10000); }

/* ---- Node allocation ---- */

int Rope12::allocNode(bool isLeaf)
{
    int idx;
    if (!m_freeList.isEmpty()) {
        idx = m_freeList.takeLast();
        m_nodes[idx] = RopeNode{};
        m_nodes[idx].isLeaf = isLeaf;
    } else {
        idx = m_nodes.size();
        m_nodes.append(RopeNode{});
        m_nodes.last().isLeaf = isLeaf;
    }
    return idx;
}

void Rope12::freeNode(int idx) { m_freeList.append(idx); }

/* ---- Subtree length ---- */

int Rope12::subtreeLen(int nodeIdx) const
{
    if (nodeIdx == NULL_NODE) return 0;
    const RopeNode& n = m_nodes[nodeIdx];
    if (n.isLeaf) return n.leafData.size();
    return n.weight + subtreeLen(n.right);
}

/* ---- Balance ratio ---- */

double Rope12::balanceRatio(int nodeIdx) const
{
    if (nodeIdx == NULL_NODE) return 1.0;
    const RopeNode& n = m_nodes[nodeIdx];
    if (n.isLeaf) return 1.0;
    int leftLen = subtreeLen(n.left);
    int rightLen = subtreeLen(n.right);
    int total = leftLen + rightLen;
    if (total == 0) return 1.0;
    return static_cast<double>(qMax(leftLen, rightLen)) / total;
}

/* ---- Rotate left ---- */

int Rope12::rotateLeft(int nodeIdx)
{
    const RopeNode& n = m_nodes[nodeIdx];
    int rChild = n.right;
    if (rChild == NULL_NODE) return nodeIdx;

    RopeNode& r = m_nodes[rChild];
    int newRoot = rChild;
    m_nodes[nodeIdx].right = r.left;
    r.left = nodeIdx;

    // Update weights
    r.weight = subtreeLen(nodeIdx);
    m_nodes[nodeIdx].weight = subtreeLen(m_nodes[nodeIdx].left);

    return newRoot;
}

/* ---- Rotate right ---- */

int Rope12::rotateRight(int nodeIdx)
{
    const RopeNode& n = m_nodes[nodeIdx];
    int lChild = n.left;
    if (lChild == NULL_NODE) return nodeIdx;

    RopeNode& l = m_nodes[lChild];
    int newRoot = lChild;
    m_nodes[nodeIdx].left = l.right;
    l.right = nodeIdx;

    // Update weights
    l.weight = subtreeLen(m_nodes[lChild].left);

    return newRoot;
}

/* ---- Rebalance ---- */

int Rope12::rebalance(int nodeIdx)
{
    if (nodeIdx == NULL_NODE) return NULL_NODE;
    RopeNode& n = m_nodes[nodeIdx];
    if (n.isLeaf) return nodeIdx;

    // Recursively rebalance children
    n.left = rebalance(n.left);
    n.right = rebalance(n.right);

    double ratio = balanceRatio(nodeIdx);
    if (ratio > 0.7) {
        int leftLen = subtreeLen(n.left);
        int rightLen = subtreeLen(n.right);
        if (leftLen > rightLen)
            return rotateRight(nodeIdx);
        else
            return rotateLeft(nodeIdx);
    }

    // Update weight
    n.weight = subtreeLen(n.left);
    return nodeIdx;
}

/* ---- Build balanced rope ---- */

int Rope12::buildBalanced(const QString& text, int start, int end)
{
    int len = end - start;
    if (len <= 0) return NULL_NODE;

    if (len <= m_leafCap) {
        int idx = allocNode(true);
        m_nodes[idx].leafData = text.mid(start, len);
        return idx;
    }

    int mid = start + len / 2;
    int nodeIdx = allocNode(false);
    m_nodes[nodeIdx].left = buildBalanced(text, start, mid);
    m_nodes[nodeIdx].right = buildBalanced(text, mid, end);
    m_nodes[nodeIdx].weight = subtreeLen(m_nodes[nodeIdx].left);
    return nodeIdx;
}

/* ---- Build ---- */

void Rope12::build(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    m_nodes.clear();
    m_freeList.clear();
    m_undoStack.clear();
    m_length = text.size();

    m_root = buildBalanced(text, 0, text.size());
    m_root = rebalance(m_root);

    m_stats.numNodes = m_nodes.size() - m_freeList.size();
    m_stats.ropeLength = m_length;
    m_stats.undoStackSize = 0;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Insert ---- */

void Rope12::insert(int pos, const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    if (text.isEmpty() || pos < 0 || pos > m_length) return;

    // Push undo
    EditOp op{EditOp::Insert, pos, text};
    m_undoStack.append(op);
    if (m_undoStack.size() > m_maxUndo)
        m_undoStack.removeFirst();

    // Split at pos, create new leaf, concat
    int rightNode = NULL_NODE;
    int leftNode = m_root;

    // Simple approach: rebuild the affected leaf
    // Build a new leaf for the inserted text
    int insNode = buildBalanced(text, 0, text.size());

    if (m_root == NULL_NODE) {
        m_root = insNode;
    } else {
        // Split at pos
        // For simplicity, rebuild entirely with new text inserted
        QString fullText = toString();
        fullText.insert(pos, text);
        m_nodes.clear();
        m_freeList.clear();
        m_root = buildBalanced(fullText, 0, fullText.size());
    }

    m_length += text.size();
    m_root = rebalance(m_root);

    m_stats.numNodes = m_nodes.size() - m_freeList.size();
    m_stats.ropeLength = m_length;
    m_stats.undoStackSize = m_undoStack.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit editDone(QStringLiteral("insert"), pos, timer.elapsed());
}

/* ---- Remove ---- */

void Rope12::remove(int pos, int len)
{
    QElapsedTimer timer;
    timer.start();

    if (pos < 0 || len <= 0 || pos + len > m_length) return;

    // Push undo
    QString removed = mid(pos, len);
    EditOp op{EditOp::Remove, pos, removed};
    m_undoStack.append(op);
    if (m_undoStack.size() > m_maxUndo)
        m_undoStack.removeFirst();

    // Rebuild
    QString fullText = toString();
    fullText.remove(pos, len);
    m_nodes.clear();
    m_freeList.clear();
    m_root = buildBalanced(fullText, 0, fullText.size());
    m_length -= len;
    m_root = rebalance(m_root);

    m_stats.numNodes = m_nodes.size() - m_freeList.size();
    m_stats.ropeLength = m_length;
    m_stats.undoStackSize = m_undoStack.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit editDone(QStringLiteral("remove"), pos, timer.elapsed());
}

/* ---- Char at ---- */

QChar Rope12::charAt(int nodeIdx, int index) const
{
    if (nodeIdx == NULL_NODE) return QLatin1Char('\0');
    const RopeNode& n = m_nodes[nodeIdx];

    if (n.isLeaf) {
        if (index >= 0 && index < n.leafData.size())
            return n.leafData[index];
        return QLatin1Char('\0');
    }

    if (index < n.weight)
        return charAt(n.left, index);
    else
        return charAt(n.right, index - n.weight);
}

QChar Rope12::at(int index) const
{
    if (index < 0 || index >= m_length) return QLatin1Char('\0');
    return charAt(m_root, index);
}

/* ---- Substring ---- */

QString Rope12::mid(int pos, int len) const
{
    QString result;
    result.reserve(len);
    for (int i = pos; i < pos + len && i < m_length; ++i)
        result.append(at(i));
    return result;
}

/* ---- To string ---- */

void Rope12::toStringHelper(int nodeIdx, QString& result) const
{
    if (nodeIdx == NULL_NODE) return;
    const RopeNode& n = m_nodes[nodeIdx];
    if (n.isLeaf) {
        result.append(n.leafData);
        return;
    }
    toStringHelper(n.left, result);
    toStringHelper(n.right, result);
}

QString Rope12::toString() const
{
    QString result;
    result.reserve(m_length);
    toStringHelper(m_root, result);
    return result;
}

/* ---- Concat ---- */

void Rope12::concat(Rope12& other)
{
    if (other.m_root == NULL_NODE) return;

    int newNode = allocNode(false);
    m_nodes[newNode].left = m_root;
    m_nodes[newNode].right = other.m_root;
    m_nodes[newNode].weight = subtreeLen(m_root);
    m_root = rebalance(newNode);
    m_length += other.m_length;

    m_stats.numNodes = m_nodes.size() - m_freeList.size();
    m_stats.ropeLength = m_length;
}

/* ---- Split ---- */

Rope12* Rope12::split(int pos)
{
    Q_UNUSED(pos)
    // Simplified: rebuild both halves
    QString leftText = mid(0, pos);
    QString rightText = mid(pos, m_length - pos);

    Rope12* rightRope = new Rope12(m_leafCap, parent());
    rightRope->build(rightText);

    // Rebuild this rope as left half
    build(leftText);

    return rightRope;
}

/* ---- Undo ---- */

bool Rope12::undo()
{
    if (m_undoStack.isEmpty()) return false;

    EditOp op = m_undoStack.takeLast();

    if (op.type == EditOp::Insert) {
        // Undo insert = remove
        QString fullText = toString();
        fullText.remove(op.position, op.text.size());
        m_nodes.clear();
        m_freeList.clear();
        m_root = buildBalanced(fullText, 0, fullText.size());
        m_length -= op.text.size();
    } else {
        // Undo remove = insert
        QString fullText = toString();
        fullText.insert(op.position, op.text);
        m_nodes.clear();
        m_freeList.clear();
        m_root = buildBalanced(fullText, 0, fullText.size());
        m_length += op.text.size();
    }

    m_root = rebalance(m_root);
    m_stats.undoStackSize = m_undoStack.size();
    m_stats.numNodes = m_nodes.size() - m_freeList.size();
    m_stats.ropeLength = m_length;
    return true;
}

/* ---- Reset ---- */

void Rope12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_freeList.clear();
    m_undoStack.clear();
    m_root = NULL_NODE;
    m_length = 0;
}

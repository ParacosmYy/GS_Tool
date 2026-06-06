/**
 * @file Rope4.cpp
 * @brief Rope4 实现
 *
 * 实现Rope数据结构：叶子分裂/拼接平衡、子串提取、字符统计。
 */

#include "utils/tree181/Rope4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Rope4::Rope4(QObject *parent) : QObject(parent) {}

Rope4::~Rope4()
{
    deleteTree(m_root);
}

void Rope4::deleteTree(Node* node)
{
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

/* ---- Build balanced rope ---- */

Rope4::Node* Rope4::buildRecursive(const QString& text, int start, int end)
{
    int len = end - start;
    if (len <= m_leafSize) {
        Node* leaf = new Node();
        leaf->data = text.mid(start, len);
        leaf->weight = len;
        return leaf;
    }

    int mid = start + len / 2;
    Node* internal = new Node();
    internal->left = buildRecursive(text, start, mid);
    internal->right = buildRecursive(text, mid, end);
    internal->weight = computeWeight(internal->left);
    return internal;
}

void Rope4::build(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    deleteTree(m_root);
    if (text.isEmpty()) {
        m_root = nullptr;
    } else {
        m_root = buildRecursive(text, 0, text.size());
    }

    int total = 0, leafC = 0;
    countNodes(m_root, total, leafC);

    m_stats.totalOperations++;
    m_stats.totalLength = text.size();
    m_stats.numNodes = total;
    m_stats.numLeaves = leafC;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("build", text.size());
}

/* ---- Compute weight ---- */

int Rope4::computeWeight(Node* node) const
{
    if (!node) return 0;
    if (node->isLeaf()) return node->data.size();
    return computeWeight(node->left);
}

/* ---- Concat ---- */

void Rope4::concat(const Rope4& other)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) {
        if (other.m_root) {
            // Deep copy other's tree
            QString otherText = other.toString();
            m_root = buildRecursive(otherText, 0, otherText.size());
        }
    } else if (other.m_root) {
        Node* newRoot = new Node();
        newRoot->left = m_root;

        // Deep copy other's tree
        QString otherText = other.toString();
        newRoot->right = buildRecursive(otherText, 0, otherText.size());
        newRoot->weight = computeWeight(newRoot->left);
        m_root = newRoot;
    }

    int total = 0, leafC = 0;
    countNodes(m_root, total, leafC);

    m_stats.totalOperations++;
    m_stats.totalLength = length();
    m_stats.numNodes = total;
    m_stats.numLeaves = leafC;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("concat", length());
}

/* ---- Split ---- */

Rope4* Rope4::split(int pos)
{
    QElapsedTimer timer;
    timer.start();

    QString fullText = toString();
    QString leftText = fullText.left(pos);
    QString rightText = fullText.mid(pos);

    // Rebuild left part
    deleteTree(m_root);
    m_root = leftText.isEmpty() ? nullptr : buildRecursive(leftText, 0, leftText.size());

    // Create right rope
    Rope4* rightRope = new Rope4(parent());
    if (!rightText.isEmpty())
        rightRope->m_root = rightRope->buildRecursive(rightText, 0, rightText.size());

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();

    emit operationCompleted("split", length());
    return rightRope;
}

/* ---- Substring ---- */

void Rope4::substringHelper(Node* node, int start, int length,
                               QString& result, int& consumed) const
{
    if (!node || length <= 0) return;

    if (node->isLeaf()) {
        int leafStart = qMax(0, start - consumed);
        int takeLen = qMin(length, node->data.size() - leafStart);
        if (takeLen > 0 && leafStart < node->data.size())
            result.append(node->data.mid(leafStart, takeLen));
        consumed += node->data.size();
        return;
    }

    // Check if we need to go into left subtree
    if (start < consumed + node->weight) {
        substringHelper(node->left, start, length, result, consumed);
    } else {
        consumed += node->weight;
    }

    // Continue into right subtree if needed
    if (result.size() < length) {
        substringHelper(node->right, start, length, result, consumed);
    }
}

QString Rope4::substring(int start, int len) const
{
    if (!m_root || len <= 0) return {};

    QString result;
    int consumed = 0;
    // Use a simpler approach: extract via index
    for (int i = start; i < start + len && i < length(); ++i)
        result.append(charAt(i));
    return result;
}

/* ---- CharAt ---- */

QChar Rope4::charAt(int index) const
{
    if (index < 0) return QChar();

    Node* current = m_root;
    while (current && !current->isLeaf()) {
        if (index < current->weight) {
            current = current->left;
        } else {
            index -= current->weight;
            current = current->right;
        }
    }

    if (current && index >= 0 && index < current->data.size())
        return current->data[index];
    return QChar();
}

/* ---- Insert ---- */

void Rope4::insert(int pos, const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    if (text.isEmpty()) return;

    Rope4 rightRope(nullptr);
    if (m_root) {
        // Split at pos, concat left + new + right
        QString full = toString();
        QString leftStr = full.left(pos);
        QString rightStr = full.mid(pos);

        deleteTree(m_root);
        m_root = leftStr.isEmpty() ? nullptr : buildRecursive(leftStr, 0, leftStr.size());

        // Build new middle
        Node* mid = buildRecursive(text, 0, text.size());

        // Concat left + mid
        if (m_root && mid) {
            Node* combined = new Node();
            combined->left = m_root;
            combined->right = mid;
            combined->weight = computeWeight(combined->left);
            m_root = combined;
        } else if (mid) {
            m_root = mid;
        }

        // Concat with right
        if (!rightStr.isEmpty()) {
            Node* right = buildRecursive(rightStr, 0, rightStr.size());
            Node* combined = new Node();
            combined->left = m_root;
            combined->right = right;
            combined->weight = computeWeight(combined->left);
            m_root = combined;
        }
    } else {
        m_root = buildRecursive(text, 0, text.size());
    }

    m_stats.totalOperations++;
    m_stats.totalLength = length();
    m_timeSum += timer.elapsed();

    emit operationCompleted("insert", length());
}

/* ---- Remove ---- */

void Rope4::remove(int start, int len)
{
    QElapsedTimer timer;
    timer.start();

    if (len <= 0 || !m_root) return;

    QString full = toString();
    full.remove(start, len);

    deleteTree(m_root);
    m_root = full.isEmpty() ? nullptr : buildRecursive(full, 0, full.size());

    m_stats.totalOperations++;
    m_stats.totalLength = length();
    m_timeSum += timer.elapsed();

    emit operationCompleted("remove", length());
}

/* ---- toString ---- */

void Rope4::collectText(Node* node, QString& result) const
{
    if (!node) return;
    if (node->isLeaf()) {
        result.append(node->data);
        return;
    }
    collectText(node->left, result);
    collectText(node->right, result);
}

QString Rope4::toString() const
{
    QString result;
    collectText(m_root, result);
    return result;
}

/* ---- Char frequency ---- */

void Rope4::charFreqHelper(Node* node, QVector<int>& freq) const
{
    if (!node) return;
    if (node->isLeaf()) {
        for (const QChar& c : node->data) {
            int idx = c.unicode();
            if (idx >= 0 && idx < 65536) freq[idx]++;
        }
        return;
    }
    charFreqHelper(node->left, freq);
    charFreqHelper(node->right, freq);
}

QVector<int> Rope4::charFrequency() const
{
    QVector<int> freq(65536, 0);
    charFreqHelper(m_root, freq);
    return freq;
}

/* ---- Length ---- */

int Rope4::length() const
{
    if (!m_root) return 0;
    if (m_root->isLeaf()) return m_root->data.size();

    // Sum all leaf data lengths
    int total = 0;
    QString text;
    collectText(m_root, text);
    return text.size();
}

bool Rope4::isEmpty() const { return m_root == nullptr; }

/* ---- Rebalance ---- */

void Rope4::collectLeaves(Node* node, QVector<QString>& leaves) const
{
    if (!node) return;
    if (node->isLeaf()) {
        leaves.append(node->data);
        return;
    }
    collectLeaves(node->left, leaves);
    collectLeaves(node->right, leaves);
}

Rope4::Node* Rope4::buildFromLeaves(const QVector<QString>& leaves, int start, int end)
{
    int count = end - start;
    if (count == 1) {
        Node* leaf = new Node();
        leaf->data = leaves[start];
        leaf->weight = leaf->data.size();
        return leaf;
    }
    if (count == 0) return nullptr;

    int mid = start + count / 2;
    Node* internal = new Node();
    internal->left = buildFromLeaves(leaves, start, mid);
    internal->right = buildFromLeaves(leaves, mid, end);
    internal->weight = computeWeight(internal->left);
    return internal;
}

void Rope4::rebalance()
{
    QElapsedTimer timer;
    timer.start();

    QVector<QString> leaves;
    collectLeaves(m_root, leaves);

    // Merge small leaves
    QVector<QString> merged;
    QString current;
    for (const QString& leaf : leaves) {
        current.append(leaf);
        if (current.size() >= m_leafSize) {
            merged.append(current);
            current.clear();
        }
    }
    if (!current.isEmpty()) merged.append(current);

    deleteTree(m_root);
    m_root = merged.isEmpty() ? nullptr : buildFromLeaves(merged, 0, merged.size());

    int total = 0, leafC = 0;
    countNodes(m_root, total, leafC);

    m_stats.totalOperations++;
    m_stats.numNodes = total;
    m_stats.numLeaves = leafC;
    m_timeSum += timer.elapsed();

    emit operationCompleted("rebalance", length());
}

/* ---- Count nodes ---- */

void Rope4::countNodes(Node* node, int& total, int& leafCount) const
{
    if (!node) return;
    total++;
    if (node->isLeaf()) { leafCount++; return; }
    countNodes(node->left, total, leafCount);
    countNodes(node->right, total, leafCount);
}

/* ---- Reset ---- */

void Rope4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

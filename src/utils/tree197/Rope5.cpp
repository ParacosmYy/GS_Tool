/**
 * @file Rope5.cpp
 * @brief Rope5 实现
 *
 * 实现Rope数据结构：手指树平衡拼接、子串反转、编辑距离。
 */

#include "utils/tree197/Rope5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Rope5::Rope5(QObject *parent) : QObject(parent) {}
Rope5::~Rope5() { delete m_root; }

/* ---- Node operations ---- */

Rope5::Node* Rope5::makeLeaf(const QString& s) const
{
    Node* n = new Node;
    n->data = s;
    n->weight = s.size();
    n->isLeaf = true;
    return n;
}

Rope5::Node* Rope5::makeInternal(Node* l, Node* r) const
{
    Node* n = new Node;
    n->left = l;
    n->right = r;
    n->weight = nodeWeight(l);
    n->isLeaf = false;
    return n;
}

int Rope5::nodeWeight(const Node* n)
{
    if (!n) return 0;
    if (n->isLeaf) return n->data.size();
    return n->weight + nodeWeight(n->right);
}

int Rope5::depth(const Node* n)
{
    if (!n) return 0;
    return 1 + qMax(depth(n->left), depth(n->right));
}

int Rope5::countNodes(const Node* n)
{
    if (!n) return 0;
    return 1 + countNodes(n->left) + countNodes(n->right);
}

/* ---- Build ---- */

void Rope5::build(const QString& str)
{
    QElapsedTimer timer;
    timer.start();

    delete m_root;
    m_root = buildBalanced(str, 0, str.size());

    m_stats.totalOperations++;
    m_stats.ropeLength = length();
    m_stats.nodeCount = countNodes(m_root);
    m_stats.depth = depth(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
}

Rope5::Node* Rope5::buildBalanced(const QString& s, int start, int len) const
{
    if (len <= 0) return nullptr;
    if (len <= m_leafMax)
        return makeLeaf(s.mid(start, len));

    int mid = len / 2;
    return makeInternal(buildBalanced(s, start, mid),
                        buildBalanced(s, start + mid, len - mid));
}

/* ---- Concatenate ---- */

void Rope5::concat(const Rope5& other)
{
    QElapsedTimer timer;
    timer.start();

    // Deep copy other's tree
    QString otherStr = other.toString();
    Node* otherRoot = buildBalanced(otherStr, 0, otherStr.size());

    m_root = concatNodes(m_root, otherRoot);

    m_stats.totalOperations++;
    m_stats.ropeLength = length();
    m_stats.nodeCount = countNodes(m_root);
    m_stats.depth = depth(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("concat", m_stats.ropeLength, timer.elapsed());
}

Node* Rope5::concatNodes(Node* a, Node* b) const
{
    if (!a) return b;
    if (!b) return a;

    // Balance: if depth difference > 3, rebalance
    int dA = depth(a), dB = depth(b);
    if (qAbs(dA - dB) > 3) {
        // Flatten both and rebuild balanced
        QString flat;
        collectChars(a, flat);
        collectChars(b, flat);
        return buildBalanced(flat, 0, flat.size());
    }

    return makeInternal(a, b);
}

/* ---- Split ---- */

void Rope5::splitAt(Node* node, int pos, Node*& left, Node*& right) const
{
    if (!node) { left = right = nullptr; return; }
    if (node->isLeaf) {
        left = makeLeaf(node->data.left(pos));
        right = makeLeaf(node->data.mid(pos));
        return;
    }

    if (pos < node->weight) {
        Node *rl = nullptr, *rr = nullptr;
        splitAt(node->left, pos, rl, rr);
        left = rl;
        right = concatNodes(rr, node->right);
    } else {
        Node *rl = nullptr, *rr = nullptr;
        splitAt(node->right, pos - node->weight, rl, rr);
        left = concatNodes(node->left, rl);
        right = rr;
    }
}

/* ---- Insert ---- */

void Rope5::insert(int pos, const QString& str)
{
    QElapsedTimer timer;
    timer.start();

    if (str.isEmpty()) return;
    Node* newPart = buildBalanced(str, 0, str.size());

    Node *left = nullptr, *right = nullptr;
    splitAt(m_root, pos, left, right);
    m_root = concatNodes(concatNodes(left, newPart), right);

    m_stats.totalOperations++;
    m_stats.ropeLength = length();
    m_stats.nodeCount = countNodes(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
}

/* ---- Remove ---- */

void Rope5::remove(int pos, int len)
{
    QElapsedTimer timer;
    timer.start();

    if (len <= 0) return;
    Node *left = nullptr, *midRight = nullptr;
    splitAt(m_root, pos, left, midRight);

    Node *mid = nullptr, *right = nullptr;
    splitAt(midRight, len, mid, right);

    delete mid;
    m_root = concatNodes(left, right);

    m_stats.totalOperations++;
    m_stats.ropeLength = length();
    m_stats.nodeCount = countNodes(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
}

/* ---- At ---- */

QChar Rope5::at(int pos) const
{
    if (pos < 0 || pos >= length()) return QChar();
    return charAt(m_root, pos);
}

QChar Rope5::charAt(const Node* n, int pos) const
{
    if (!n) return QChar();
    if (n->isLeaf) {
        if (pos >= 0 && pos < n->data.size()) return n->data[pos];
        return QChar();
    }
    if (pos < n->weight) return charAt(n->left, pos);
    return charAt(n->right, pos - n->weight);
}

/* ---- Substring ---- */

QString Rope5::substring(int pos, int len) const
{
    if (len <= 0) return {};
    QString result;
    result.reserve(len);
    for (int i = pos; i < pos + len && i < length(); ++i)
        result += at(i);
    return result;
}

/* ---- Reverse ---- */

void Rope5::reverse(int pos, int len)
{
    QElapsedTimer timer;
    timer.start();

    QString sub = substring(pos, len);
    std::reverse(sub.begin(), sub.end());

    // Replace: split, rebuild middle, concat
    Node *left = nullptr, *midRight = nullptr;
    splitAt(m_root, pos, left, midRight);
    Node *mid = nullptr, *right = nullptr;
    splitAt(midRight, len, mid, right);
    delete mid;

    Node* revNode = buildBalanced(sub, 0, sub.size());
    m_root = concatNodes(concatNodes(left, revNode), right);

    m_stats.totalOperations++;
    m_stats.ropeLength = length();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("reverse", m_stats.ropeLength, timer.elapsed());
}

/* ---- Edit distance (Wagner-Fischer) ---- */

int Rope5::editDistance(const QString& other) const
{
    QElapsedTimer timer;
    timer.start();

    QString a = toString();
    int m = a.size(), n = other.size();

    QVector<QVector<int>> dp(m + 1, QVector<int>(n + 1, 0));
    for (int i = 0; i <= m; ++i) dp[i][0] = i;
    for (int j = 0; j <= n; ++j) dp[0][j] = j;

    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            int cost = (a[i-1] == other[j-1]) ? 0 : 1;
            dp[i][j] = qMin({dp[i-1][j] + 1,
                             dp[i][j-1] + 1,
                             dp[i-1][j-1] + cost});
        }
    }

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    return dp[m][n];
}

/* ---- Edit path ---- */

QVector<QString> Rope5::editPath(const QString& other) const
{
    QString a = toString();
    int m = a.size(), n = other.size();

    QVector<QVector<int>> dp(m + 1, QVector<int>(n + 1, 0));
    for (int i = 0; i <= m; ++i) dp[i][0] = i;
    for (int j = 0; j <= n; ++j) dp[0][j] = j;

    for (int i = 1; i <= m; ++i)
        for (int j = 1; j <= n; ++j) {
            int cost = (a[i-1] == other[j-1]) ? 0 : 1;
            dp[i][j] = qMin({dp[i-1][j] + 1, dp[i][j-1] + 1, dp[i-1][j-1] + cost});
        }

    // Backtrack
    QVector<QString> ops;
    int i = m, j = n;
    while (i > 0 || j > 0) {
        if (i > 0 && j > 0 && a[i-1] == other[j-1]) {
            ops.prepend("match '" + QString(a[i-1]) + "'");
            i--; j--;
        } else if (i > 0 && j > 0 && dp[i][j] == dp[i-1][j-1] + 1) {
            ops.prepend("replace '" + QString(a[i-1]) + "' -> '" + QString(other[j-1]) + "'");
            i--; j--;
        } else if (j > 0 && dp[i][j] == dp[i][j-1] + 1) {
            ops.prepend("insert '" + QString(other[j-1]) + "'");
            j--;
        } else {
            ops.prepend("delete '" + QString(a[i-1]) + "'");
            i--;
        }
    }
    return ops;
}

/* ---- ToString ---- */

void Rope5::collectChars(const Node* n, QString& out) const
{
    if (!n) return;
    if (n->isLeaf) { out += n->data; return; }
    collectChars(n->left, out);
    collectChars(n->right, out);
}

QString Rope5::toString() const
{
    QString result;
    result.reserve(length());
    collectChars(m_root, result);
    return result;
}

/* ---- Length ---- */

int Rope5::length() const { return nodeWeight(m_root); }

/* ---- Rebalance ---- */

void Rope5::rebalance()
{
    QElapsedTimer timer;
    timer.start();

    QString flat = toString();
    delete m_root;
    m_root = buildBalanced(flat, 0, flat.size());

    m_stats.totalOperations++;
    m_stats.nodeCount = countNodes(m_root);
    m_stats.depth = depth(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
}

/* ---- Reset ---- */

void Rope5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    delete m_root;
    m_root = nullptr;
}

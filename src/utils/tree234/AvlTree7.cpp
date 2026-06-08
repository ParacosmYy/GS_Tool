/**
 * @file AvlTree7.cpp
 * @brief AvlTree7 实现
 *
 * 实现AVL树：秩增强节点顺序统计查询与加权平衡范围报告。
 */

#include "utils/tree234/AvlTree7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

AvlTree7::AvlTree7(QObject *parent) : QObject(parent) {}
AvlTree7::~AvlTree7() { deleteTree(m_root); }

/* ---- Delete entire subtree ---- */

void AvlTree7::deleteTree(Node* node)
{
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

/* ---- Update height and rank ---- */

void AvlTree7::update(Node* node)
{
    if (!node) return;
    node->height = 1 + qMax(height(node->left), height(node->right));
    node->subtreeSize = 1 + size(node->left) + size(node->right);
    // Weighted balance: weight = 1 + sum of children weights
    double lw = node->left ? node->left->weight : 0.0;
    double rw = node->right ? node->right->weight : 0.0;
    node->weight = 1.0 + lw + rw;
}

/* ---- Balance factor ---- */

int AvlTree7::balanceFactor(Node* node) const
{
    return node ? height(node->left) - height(node->right) : 0;
}

/* ---- Right rotation ---- */

AvlTree7::Node* AvlTree7::rotateRight(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    x->right = y;
    update(y);
    update(x);
    m_stats.numRotations++;
    return x;
}

/* ---- Left rotation ---- */

AvlTree7::Node* AvlTree7::rotateLeft(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    y->left = x;
    update(x);
    update(y);
    m_stats.numRotations++;
    return y;
}

/* ---- Rebalance ---- */

AvlTree7::Node* AvlTree7::rebalance(Node* node)
{
    update(node);
    int bf = balanceFactor(node);

    if (bf > 1) {
        // Left-heavy
        if (balanceFactor(node->left) < 0)
            node->left = rotateLeft(node->left);  // LR case
        return rotateRight(node);  // LL case
    }
    if (bf < -1) {
        // Right-heavy
        if (balanceFactor(node->right) > 0)
            node->right = rotateRight(node->right);  // RL case
        return rotateLeft(node);  // RR case
    }
    return node;
}

/* ---- Insert ---- */

void AvlTree7::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertNode(m_root, key, value);
    m_stats.numNodes++;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit nodeInserted(key);
}

AvlTree7::Node* AvlTree7::insertNode(Node* node, int key, double value)
{
    if (!node) {
        Node* n = new Node;
        n->key = key;
        n->value = value;
        return n;
    }
    if (key < node->key)
        node->left = insertNode(node->left, key, value);
    else if (key > node->key)
        node->right = insertNode(node->right, key, value);
    else {
        node->value = value;  // Update existing
        return node;
    }
    return rebalance(node);
}

/* ---- Remove ---- */

void AvlTree7::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    m_root = removeNode(m_root, key);
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit nodeRemoved(key);
}

AvlTree7::Node* AvlTree7::removeNode(Node* node, int key)
{
    if (!node) return nullptr;

    if (key < node->key) {
        node->left = removeNode(node->left, key);
    } else if (key > node->key) {
        node->right = removeNode(node->right, key);
    } else {
        m_stats.numNodes--;
        if (!node->left || !node->right) {
            Node* child = node->left ? node->left : node->right;
            delete node;
            return child;
        }
        // Two children: replace with in-order successor
        Node* succ = findMin(node->right);
        node->key = succ->key;
        node->value = succ->value;
        node->right = removeNode(node->right, succ->key);
        m_stats.numNodes++;  // Correct for double-count
    }
    return rebalance(node);
}

/* ---- Find minimum ---- */

AvlTree7::Node* AvlTree7::findMin(Node* node) const
{
    while (node && node->left) node = node->left;
    return node;
}

/* ---- Search ---- */

double AvlTree7::search(int key) const
{
    return searchNode(m_root, key);
}

double AvlTree7::searchNode(Node* node, int key) const
{
    while (node) {
        if (key < node->key) node = node->left;
        else if (key > node->key) node = node->right;
        else return node->value;
    }
    return std::numeric_limits<double>::quiet_NaN();
}

/* ---- Contains ---- */

bool AvlTree7::contains(int key) const
{
    Node* node = m_root;
    while (node) {
        if (key < node->key) node = node->left;
        else if (key > node->key) node = node->right;
        else return true;
    }
    return false;
}

/* ---- Select k-th smallest (order statistic) ---- */

int AvlTree7::select(int k) const
{
    return selectNode(m_root, k);
}

int AvlTree7::selectNode(Node* node, int k) const
{
    if (!node) return -1;
    int leftSize = size(node->left);
    if (k <= leftSize) return selectNode(node->left, k);
    if (k == leftSize + 1) return node->key;
    return selectNode(node->right, k - leftSize - 1);
}

/* ---- Rank of key ---- */

int AvlTree7::rank(int key) const
{
    return rankNode(m_root, key);
}

int AvlTree7::rankNode(Node* node, int key) const
{
    if (!node) return 0;
    if (key < node->key) return rankNode(node->left, key);
    if (key == node->key) return size(node->left) + 1;
    return size(node->left) + 1 + rankNode(node->right, key);
}

/* ---- Range query ---- */

QVector<QPair<int, double>> AvlTree7::rangeQuery(int lo, int hi) const
{
    QVector<QPair<int, double>> result;
    rangeHelper(m_root, lo, hi, result);

    const_cast<AvlTree7*>(this)->m_stats.numRangeQueries++;
    const_cast<AvlTree7*>(this)->emit rangeQueryCompleted(lo, hi, result.size());
    return result;
}

void AvlTree7::rangeHelper(Node* node, int lo, int hi,
                             QVector<QPair<int, double>>& result) const
{
    if (!node) return;
    if (lo < node->key) rangeHelper(node->left, lo, hi, result);
    if (lo <= node->key && node->key <= hi)
        result.append({node->key, node->value});
    if (hi > node->key) rangeHelper(node->right, lo, hi, result);
}

/* ---- Range count ---- */

int AvlTree7::rangeCount(int lo, int hi) const
{
    return rangeCountHelper(m_root, lo, hi);
}

int AvlTree7::rangeCountHelper(Node* node, int lo, int hi) const
{
    if (!node) return 0;
    if (node->key < lo) return rangeCountHelper(node->right, lo, hi);
    if (node->key > hi) return rangeCountHelper(node->left, lo, hi);
    // node->key is in [lo, hi]
    return 1 + rangeCountHelper(node->left, lo, hi)
           + rangeCountHelper(node->right, lo, hi);
}

/* ---- In-order traversal ---- */

QVector<QPair<int, double>> AvlTree7::inorderTraversal() const
{
    QVector<QPair<int, double>> result;
    inorderHelper(m_root, result);
    return result;
}

void AvlTree7::inorderHelper(Node* node, QVector<QPair<int, double>>& result) const
{
    if (!node) return;
    inorderHelper(node->left, result);
    result.append({node->key, node->value});
    inorderHelper(node->right, result);
}

/* ---- Compute height ---- */

int AvlTree7::computeHeight(Node* node) const
{
    if (!node) return 0;
    return 1 + qMax(computeHeight(node->left), computeHeight(node->right));
}

/* ---- Reset ---- */

void AvlTree7::resetStatistics()
{
    deleteTree(m_root);
    m_root = nullptr;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

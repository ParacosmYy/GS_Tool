/**
 * @file WeightBalancedTree5.cpp
 * @brief WeightBalancedTree5 实现
 *
 * 实现权重平衡树：δ-γ平衡条件、旋转重平衡、秩/选择、join2拼接。
 */

#include "utils/tree187/WeightBalancedTree5.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WeightBalancedTree5::WeightBalancedTree5(QObject *parent) : QObject(parent) {}
WeightBalancedTree5::~WeightBalancedTree5() { deleteTree(m_root); }

void WeightBalancedTree5::deleteTree(Node* n)
{
    if (!n) return;
    deleteTree(n->left);
    deleteTree(n->right);
    delete n;
}

/* ---- Configuration ---- */

void WeightBalancedTree5::setDelta(double delta)
{
    m_delta = qBound(0.1, delta, 0.5);
    m_gamma = qMax(m_gamma, m_delta / (1.0 - m_delta));
}

void WeightBalancedTree5::setGamma(double gamma)
{
    m_gamma = qMax(gamma, m_delta / (1.0 - m_delta));
}

/* ---- Weight helpers ---- */

void WeightBalancedTree5::updateWeight(Node* n)
{
    if (n) n->weight = w(n->left) + w(n->right) + 1;
}

/* ---- Rotations ---- */

WeightBalancedTree5::Node* WeightBalancedTree5::rotateLeft(Node* n)
{
    Node* r = n->right;
    n->right = r->left;
    r->left = n;
    updateWeight(n);
    updateWeight(r);
    m_stats.numRotations++;
    return r;
}

WeightBalancedTree5::Node* WeightBalancedTree5::rotateRight(Node* n)
{
    Node* l = n->left;
    n->left = l->right;
    l->right = n;
    updateWeight(n);
    updateWeight(l);
    m_stats.numRotations++;
    return l;
}

WeightBalancedTree5::Node* WeightBalancedTree5::rotateLeftRight(Node* n)
{
    n->left = rotateLeft(n->left);
    return rotateRight(n);
}

WeightBalancedTree5::Node* WeightBalancedTree5::rotateRightLeft(Node* n)
{
    n->right = rotateRight(n->right);
    return rotateLeft(n);
}

/* ---- Balance check and rebalance ---- */

WeightBalancedTree5::Node* WeightBalancedTree5::balance(Node* n)
{
    if (!n) return nullptr;
    updateWeight(n);

    int lw = w(n->left), rw = w(n->right), tw = n->weight;
    if (tw <= 1) return n;

    // Left-heavy: left weight too large
    if (lw > m_delta * tw) {
        int llw = w(n->left->left), lrw = w(n->left->right);
        if (llw > m_gamma * lw)
            return rotateRight(n);
        else
            return rotateLeftRight(n);
    }

    // Right-heavy: right weight too large
    if (rw > m_delta * tw) {
        int rrw = w(n->right->right), rlw = w(n->right->left);
        if (rrw > m_gamma * rw)
            return rotateLeft(n);
        else
            return rotateRightLeft(n);
    }

    return n;
}

/* ---- Insert ---- */

WeightBalancedTree5::Node* WeightBalancedTree5::insertNode(Node* n, int key)
{
    if (!n) return new Node{key, 1, nullptr, nullptr};

    if (key < n->key)
        n->left = insertNode(n->left, key);
    else if (key > n->key)
        n->right = insertNode(n->right, key);
    // Duplicate: ignore

    return balance(n);
}

void WeightBalancedTree5::insert(int key)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertNode(m_root, key);

    m_stats.totalOperations++;
    m_stats.numNodes = w(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("insert", key);
}

/* ---- Remove ---- */

WeightBalancedTree5::Node* WeightBalancedTree5::removeNode(Node* n, int key)
{
    if (!n) return nullptr;

    if (key < n->key) {
        n->left = removeNode(n->left, key);
    } else if (key > n->key) {
        n->right = removeNode(n->right, key);
    } else {
        // Found node to delete
        if (!n->left || !n->right) {
            Node* child = n->left ? n->left : n->right;
            delete n;
            return child;
        }
        // Replace with in-order successor
        Node* succ = n->right;
        while (succ->left) succ = succ->left;
        n->key = succ->key;
        n->right = removeNode(n->right, succ->key);
    }
    return balance(n);
}

void WeightBalancedTree5::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    m_root = removeNode(m_root, key);

    m_stats.totalOperations++;
    m_stats.numNodes = w(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("remove", key);
}

/* ---- Contains ---- */

bool WeightBalancedTree5::contains(int key) const
{
    Node* n = m_root;
    while (n) {
        if (key < n->key) n = n->left;
        else if (key > n->key) n = n->right;
        else return true;
    }
    return false;
}

/* ---- Rank: number of elements < key ---- */

int WeightBalancedTree5::rankHelper(Node* n, int key) const
{
    if (!n) return 0;
    if (key <= n->key) return rankHelper(n->left, key);
    return w(n->left) + 1 + rankHelper(n->right, key);
}

int WeightBalancedTree5::rank(int key) const
{
    return rankHelper(m_root, key);
}

/* ---- Select: k-th smallest (1-indexed) ---- */

int WeightBalancedTree5::selectHelper(Node* n, int k) const
{
    if (!n) return -1;
    int leftSize = w(n->left);
    if (k <= leftSize) return selectHelper(n->left, k);
    if (k == leftSize + 1) return n->key;
    return selectHelper(n->right, k - leftSize - 1);
}

int WeightBalancedTree5::select(int k) const
{
    if (k < 1 || k > w(m_root)) return -1;
    return selectHelper(m_root, k);
}

/* ---- join2: concatenate two trees (all keys in this < all keys in other) ---- */

WeightBalancedTree5::Node* WeightBalancedTree5::join2Nodes(Node* left, Node* right)
{
    if (!left) return right;
    if (!right) return left;

    // Find rightmost node in left tree
    Node* parent = nullptr;
    Node* maxNode = left;
    while (maxNode->right) {
        parent = maxNode;
        maxNode = maxNode->right;
    }

    // Remove max from left tree
    if (parent) {
        parent->right = maxNode->left;
        // Rebalance up the path
        Node* newLeft = left;
        // Simple approach: re-insert max as root
        maxNode->left = newLeft;
        maxNode->right = right;
        updateWeight(maxNode);
        return balance(maxNode);
    } else {
        maxNode->left = nullptr;
        maxNode->right = right;
        updateWeight(maxNode);
        return balance(maxNode);
    }
}

void WeightBalancedTree5::join2(WeightBalancedTree5& other)
{
    QElapsedTimer timer;
    timer.start();

    m_root = join2Nodes(m_root, other.m_root);
    other.m_root = nullptr;

    m_stats.totalOperations++;
    m_stats.numNodes = w(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("join2", 0);
}

/* ---- Inorder ---- */

void WeightBalancedTree5::inorderHelper(Node* n, QVector<int>& result) const
{
    if (!n) return;
    inorderHelper(n->left, result);
    result.append(n->key);
    inorderHelper(n->right, result);
}

QVector<int> WeightBalancedTree5::inorder() const
{
    QVector<int> result;
    inorderHelper(m_root, result);
    return result;
}

int WeightBalancedTree5::size() const { return w(m_root); }

void WeightBalancedTree5::clear()
{
    deleteTree(m_root);
    m_root = nullptr;
}

/* ---- Reset ---- */

void WeightBalancedTree5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

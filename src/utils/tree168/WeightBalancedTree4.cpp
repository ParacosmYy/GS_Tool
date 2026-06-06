/**
 * @file WeightBalancedTree4.cpp
 * @brief WeightBalancedTree4 实现
 *
 * 实现重量平衡树：权重检测不平衡、单/双旋转、排名查询。
 */

#include "utils/tree168/WeightBalancedTree4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

WeightBalancedTree4::WeightBalancedTree4(QObject* parent)
    : QObject(parent)
{
}

WeightBalancedTree4::~WeightBalancedTree4()
{
    clearRec(m_root);
}

void WeightBalancedTree4::setAlpha(double alpha)
{
    m_alpha = qBound(0.2, alpha, 0.5);
}

int WeightBalancedTree4::wt(Node* node)
{
    return node ? node->weight : 0;
}

void WeightBalancedTree4::updateWeight(Node* node)
{
    if (node) node->weight = 1 + wt(node->left) + wt(node->right);
}

int WeightBalancedTree4::height(Node* node)
{
    if (!node) return 0;
    return 1 + qMax(height(node->left), height(node->right));
}

WeightBalancedTree4::Node* WeightBalancedTree4::rotateRight(Node* node)
{
    Node* newRoot = node->left;
    node->left = newRoot->right;
    newRoot->right = node;
    updateWeight(node);
    updateWeight(newRoot);
    m_stats.totalRotations++;
    emit rotationPerformed(SingleRight);
    return newRoot;
}

WeightBalancedTree4::Node* WeightBalancedTree4::rotateLeft(Node* node)
{
    Node* newRoot = node->right;
    node->right = newRoot->left;
    newRoot->left = node;
    updateWeight(node);
    updateWeight(newRoot);
    m_stats.totalRotations++;
    emit rotationPerformed(SingleLeft);
    return newRoot;
}

WeightBalancedTree4::Node* WeightBalancedTree4::rebalance(Node* node)
{
    if (!node) return nullptr;
    updateWeight(node);

    int lw = wt(node->left);
    int rw = wt(node->right);
    int tw = node->weight;

    if (tw <= 1) return node;

    /* Check left-heavy: left weight > alpha * total */
    if (lw > m_alpha * tw + 1) {
        /* Is left child right-heavy? => double rotation */
        if (wt(node->left->right) > wt(node->left->left)) {
            node->left = rotateLeft(node->left);
            emit rotationPerformed(DoubleLeftRight);
        }
        return rotateRight(node);
    }

    /* Check right-heavy */
    if (rw > m_alpha * tw + 1) {
        /* Is right child left-heavy? => double rotation */
        if (wt(node->right->left) > wt(node->right->right)) {
            node->right = rotateRight(node->right);
            emit rotationPerformed(DoubleRightLeft);
        }
        return rotateLeft(node);
    }

    return node;
}

WeightBalancedTree4::Node* WeightBalancedTree4::insertRec(Node* node, double key,
                                                            bool& inserted)
{
    if (!node) {
        inserted = true;
        return new Node(key);
    }

    if (key < node->key) {
        node->left = insertRec(node->left, key, inserted);
    } else if (key > node->key) {
        node->right = insertRec(node->right, key, inserted);
    } else {
        /* Duplicate: ignore */
        inserted = false;
        return node;
    }

    return rebalance(node);
}

bool WeightBalancedTree4::insert(double key)
{
    QElapsedTimer timer;
    timer.start();

    if (contains(key)) return false;

    bool inserted = false;
    m_root = insertRec(m_root, key, inserted);

    if (inserted) {
        m_stats.currentSize++;
        m_stats.treeHeight = height(m_root);
    }

    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalInserts + m_stats.totalDeletes;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit insertCompleted(key, m_stats.currentSize);
    return inserted;
}

WeightBalancedTree4::Node* WeightBalancedTree4::removeRec(Node* node, double key,
                                                            bool& removed)
{
    if (!node) return nullptr;

    if (key < node->key) {
        node->left = removeRec(node->left, key, removed);
    } else if (key > node->key) {
        node->right = removeRec(node->right, key, removed);
    } else {
        removed = true;
        if (!node->left && !node->right) {
            delete node;
            return nullptr;
        }
        if (!node->left) {
            Node* r = node->right;
            node->left = node->right = nullptr;
            delete node;
            return r;
        }
        if (!node->right) {
            Node* l = node->left;
            node->left = node->right = nullptr;
            delete node;
            return l;
        }
        /* Two children: replace with in-order successor */
        Node* succ = node->right;
        while (succ->left) succ = succ->left;
        node->key = succ->key;
        node->right = removeRec(node->right, succ->key, removed);
    }

    return rebalance(node);
}

bool WeightBalancedTree4::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    bool removed = false;
    m_root = removeRec(m_root, key, removed);

    if (removed) {
        m_stats.currentSize--;
        m_stats.treeHeight = height(m_root);
    }

    m_stats.totalDeletes++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalInserts + m_stats.totalDeletes;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit deleteCompleted(key);
    return removed;
}

bool WeightBalancedTree4::containsRec(Node* node, double key) const
{
    if (!node) return false;
    if (key < node->key) return containsRec(node->left, key);
    if (key > node->key) return containsRec(node->right, key);
    return true;
}

bool WeightBalancedTree4::contains(double key) const
{
    return containsRec(m_root, key);
}

int WeightBalancedTree4::rankRec(Node* node, double key) const
{
    if (!node) return 0;
    if (key <= node->key) return rankRec(node->left, key);
    return 1 + wt(node->left) + rankRec(node->right, key);
}

int WeightBalancedTree4::rank(double key) const
{
    return rankRec(m_root, key);
}

double WeightBalancedTree4::kthRec(Node* node, int k) const
{
    if (!node) return 0.0;
    int lw = wt(node->left);
    if (k <= lw) return kthRec(node->left, k);
    if (k == lw + 1) return node->key;
    return kthRec(node->right, k - lw - 1);
}

double WeightBalancedTree4::kth(int k) const
{
    if (k < 1 || k > m_stats.currentSize) return 0.0;
    return kthRec(m_root, k);
}

void WeightBalancedTree4::inorderRec(Node* node, QVector<double>& result) const
{
    if (!node) return;
    inorderRec(node->left, result);
    result.append(node->key);
    inorderRec(node->right, result);
}

QVector<double> WeightBalancedTree4::inorder() const
{
    QVector<double> result;
    inorderRec(m_root, result);
    return result;
}

bool WeightBalancedTree4::isEmpty() const
{
    return m_root == nullptr;
}

void WeightBalancedTree4::clearRec(Node* node)
{
    if (!node) return;
    clearRec(node->left);
    clearRec(node->right);
    delete node;
}

void WeightBalancedTree4::clear()
{
    clearRec(m_root);
    m_root = nullptr;
    m_stats.currentSize = 0;
    m_stats.treeHeight = 0;
}

void WeightBalancedTree4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file AA7.cpp
 * @brief AA7 实现
 *
 * 实现AA树：自顶向下skew-split删除与epoch并发读垃圾回收。
 */

#include "utils/tree233/AA7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

AA7::AA7(QObject *parent) : QObject(parent), m_currentEpoch(1) {}
AA7::~AA7() { deleteTree(m_root); }

/* ---- Delete entire subtree ---- */

void AA7::deleteTree(Node* node)
{
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

/* ---- Skew (right rotation for left-horizontal link) ---- */

AA7::Node* AA7::skew(Node* node)
{
    if (!node || !node->left) return node;
    if (node->left->level == node->level) {
        // Left-horizontal link detected: rotate right
        Node* left = node->left;
        node->left = left->right;
        left->right = node;
        m_stats.numSkews++;
        return left;
    }
    return node;
}

/* ---- Split (left rotation for consecutive right-horizontal links) ---- */

AA7::Node* AA7::split(Node* node)
{
    if (!node || !node->right || !node->right->right) return node;
    if (node->right->right->level == node->level) {
        // Two consecutive right-horizontal links: rotate left
        Node* right = node->right;
        node->right = right->left;
        right->left = node;
        right->level++;
        m_stats.numSplits++;
        return right;
    }
    return node;
}

/* ---- Insert ---- */

void AA7::insert(int key, double value)
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

/* ---- Recursive insert ---- */

AA7::Node* AA7::insertNode(Node* node, int key, double value)
{
    if (!node) {
        Node* n = new Node;
        n->key = key;
        n->value = value;
        n->level = 1;
        n->left = n->right = nullptr;
        n->epoch = m_currentEpoch.load();
        n->marked = false;
        return n;
    }

    if (key < node->key) {
        node->left = insertNode(node->left, key, value);
    } else if (key > node->key) {
        node->right = insertNode(node->right, key, value);
    } else {
        node->value = value;  // Update existing
        return node;
    }

    // Bottom-up rebalance: skew then split
    node = skew(node);
    node = split(node);
    return node;
}

/* ---- Remove ---- */

void AA7::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    m_root = removeNode(m_root, key);

    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit nodeRemoved(key);

    // Check if GC needed
    if (m_retiredNodes.size() >= m_gcThreshold)
        collectGarbage();
}

/* ---- Top-down skew-split deletion ---- */

AA7::Node* AA7::removeNode(Node* node, int key)
{
    if (!node) return nullptr;

    if (key < node->key) {
        node->left = removeNode(node->left, key);
    } else if (key > node->key) {
        node->right = removeNode(node->right, key);
    } else {
        // Found node to remove
        if (!node->left && !node->right) {
            // Leaf: retire for epoch-based GC
            retireNode(node);
            m_stats.numNodes--;
            return nullptr;
        }
        if (!node->left) {
            Node* succ = findMin(node->right);
            node->key = succ->key;
            node->value = succ->value;
            node->right = removeNode(node->right, succ->key);
        } else {
            // Find predecessor (max of left subtree)
            Node* pred = node->left;
            while (pred->right) pred = pred->right;
            node->key = pred->key;
            node->value = pred->value;
            node->left = removeNode(node->left, pred->key);
        }
    }

    // Fix levels: decrease level if needed
    int leftLevel = node->left ? node->left->level : 0;
    int rightLevel = node->right ? node->right->level : 0;
    int shouldBe = qMin(leftLevel, rightLevel) + 1;
    if (shouldBe < node->level) {
        node->level = shouldBe;
        if (node->right && node->right->level > shouldBe)
            node->right->level = shouldBe;
    }

    // Rebalance: three skews and two splits (top-down)
    node = skew(node);
    if (node->right) {
        node->right = skew(node->right);
        if (node->right->right)
            node->right->right = skew(node->right->right);
    }
    node = split(node);
    if (node->right)
        node->right = split(node->right);

    return node;
}

/* ---- Find minimum ---- */

AA7::Node* AA7::findMin(Node* node) const
{
    while (node && node->left) node = node->left;
    return node;
}

/* ---- Search ---- */

double AA7::search(int key) const
{
    return searchNode(m_root, key);
}

double AA7::searchNode(Node* node, int key) const
{
    while (node) {
        if (key < node->key) node = node->left;
        else if (key > node->key) node = node->right;
        else return node->value;
    }
    return std::numeric_limits<double>::quiet_NaN();
}

/* ---- Contains ---- */

bool AA7::contains(int key) const
{
    Node* node = m_root;
    while (node) {
        if (key < node->key) node = node->left;
        else if (key > node->key) node = node->right;
        else return true;
    }
    return false;
}

/* ---- In-order traversal ---- */

QVector<QPair<int, double>> AA7::inorderTraversal() const
{
    QVector<QPair<int, double>> result;
    inorderHelper(m_root, result);
    return result;
}

void AA7::inorderHelper(Node* node, QVector<QPair<int, double>>& result) const
{
    if (!node) return;
    inorderHelper(node->left, result);
    result.append({node->key, node->value});
    inorderHelper(node->right, result);
}

/* ---- Compute height ---- */

int AA7::computeHeight(Node* node) const
{
    if (!node) return 0;
    return 1 + qMax(computeHeight(node->left), computeHeight(node->right));
}

/* ---- Retire node (epoch-based GC) ---- */

void AA7::retireNode(Node* node)
{
    node->marked = true;
    node->epoch = m_currentEpoch.load();
    // Sever links to prevent traversal into retired nodes
    node->left = nullptr;
    node->right = nullptr;
    m_retiredNodes.append(node);
}

/* ---- Garbage collection ---- */

void AA7::collectGarbage()
{
    int safeEpoch = m_currentEpoch.load();
    int reclaimed = 0;
    QVector<Node*> stillRetired;

    for (Node* node : m_retiredNodes) {
        // Only reclaim nodes from old enough epochs
        if (node->epoch < safeEpoch) {
            delete node;
            reclaimed++;
        } else {
            stillRetired.append(node);
        }
    }

    m_retiredNodes = stillRetired;
    m_stats.numGCycles++;
    m_stats.numNodesReclaimed += reclaimed;

    // Advance epoch
    m_currentEpoch.fetchAndAddRelaxed(1);

    if (reclaimed > 0)
        emit garbageCollected(reclaimed);
}

/* ---- Reset ---- */

void AA7::resetStatistics()
{
    deleteTree(m_root);
    m_root = nullptr;
    for (Node* n : m_retiredNodes) delete n;
    m_retiredNodes.clear();
    m_currentEpoch.store(1);
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file AA4.cpp
 * @brief AA4 实现
 *
 * 实现AA树：skew/split重平衡、level不变量维护、rank操作、插入/删除/查找。
 */

#include "utils/tree182/AA4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

AA4::AA4(QObject *parent) : QObject(parent) {}
AA4::~AA4() { deleteTree(m_root); }

void AA4::deleteTree(Node* node)
{
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

/* ---- Size update ---- */

void AA4::updateSize(Node* node)
{
    if (!node) return;
    node->size = 1;
    if (node->left) node->size += node->left->size;
    if (node->right) node->size += node->right->size;
}

/* ---- Skew (right rotation) ---- */

AA4::Node* AA4::skew(Node* node)
{
    if (!node || !node->left) return node;
    if (node->left->level != node->level) return node;

    // Left child has same level -> horizontal left link
    Node* left = node->left;
    node->left = left->right;
    left->right = node;

    updateSize(node);
    updateSize(left);
    return left;
}

/* ---- Split (left rotation) ---- */

AA4::Node* AA4::split(Node* node)
{
    if (!node || !node->right || !node->right->right) return node;
    if (node->right->right->level != node->level) return node;

    // Two consecutive right links at same level
    Node* right = node->right;
    node->right = right->left;
    right->left = node;
    right->level++;

    updateSize(node);
    updateSize(right);
    return right;
}

/* ---- Insert ---- */

AA4::Node* AA4::insertNode(Node* node, int key)
{
    if (!node) return new Node{key, 1, 1, nullptr, nullptr};

    if (key < node->key)
        node->left = insertNode(node->left, key);
    else if (key > node->key)
        node->right = insertNode(node->right, key);
    else
        return node; // Duplicate: no-op

    // Rebalance
    node = skew(node);
    node = split(node);

    updateSize(node);
    return node;
}

/* ---- Find minimum ---- */

AA4::Node* AA4::findMin(Node* node) const
{
    while (node && node->left) node = node->left;
    return node;
}

/* ---- Decrease level ---- */

AA4::Node* AA4::decreaseLevel(Node* node)
{
    int shouldBe = 1;
    if (node->left) shouldBe = qMin(shouldBe, node->left->level);
    if (node->right) shouldBe = qMin(shouldBe, node->right->level);
    shouldBe++;

    if (shouldBe < node->level) {
        node->level = shouldBe;
        if (node->right && node->right->level > shouldBe)
            node->right->level = shouldBe;
    }
    return node;
}

/* ---- Remove ---- */

AA4::Node* AA4::removeNode(Node* node, int key)
{
    if (!node) return nullptr;

    if (key < node->key) {
        node->left = removeNode(node->left, key);
    } else if (key > node->key) {
        node->right = removeNode(node->right, key);
    } else {
        // Found the node to remove
        if (!node->left && !node->right) {
            delete node;
            return nullptr;
        }
        if (!node->left) {
            Node* succ = findMin(node->right);
            node->key = succ->key;
            node->right = removeNode(node->right, succ->key);
        } else {
            // Find predecessor (max of left subtree)
            Node* pred = node->left;
            while (pred->right) pred = pred->right;
            node->key = pred->key;
            node->left = removeNode(node->left, pred->key);
        }
    }

    // Rebalance after deletion
    node = decreaseLevel(node);
    node = skew(node);
    if (node->right) {
        node->right = skew(node->right);
        if (node->right->right)
            node->right->right = skew(node->right->right);
    }
    node = split(node);
    if (node->right)
        node->right = split(node->right);

    updateSize(node);
    return node;
}

/* ---- Search ---- */

bool AA4::searchNode(Node* node, int key) const
{
    while (node) {
        if (key < node->key) node = node->left;
        else if (key > node->key) node = node->right;
        else return true;
    }
    return false;
}

/* ---- Rank ---- */

int AA4::rankNode(Node* node, int key) const
{
    if (!node) return 1;
    if (key <= node->key) {
        return rankNode(node->left, key);
    } else {
        int leftSize = node->left ? node->left->size : 0;
        return leftSize + 1 + rankNode(node->right, key);
    }
}

/* ---- Kth ---- */

int AA4::kthNode(Node* node, int k) const
{
    if (!node) return -1;
    int leftSize = node->left ? node->left->size : 0;

    if (k <= leftSize) return kthNode(node->left, k);
    if (k == leftSize + 1) return node->key;
    return kthNode(node->right, k - leftSize - 1);
}

/* ---- Public API ---- */

void AA4::insert(int key)
{
    QElapsedTimer timer;
    timer.start();
    m_root = insertNode(m_root, key);

    m_stats.totalOperations++;
    m_stats.numNodes = size();
    m_stats.treeHeight = treeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("insert", key);
}

void AA4::remove(int key)
{
    QElapsedTimer timer;
    timer.start();
    m_root = removeNode(m_root, key);

    m_stats.totalOperations++;
    m_stats.numNodes = size();
    m_stats.treeHeight = treeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("remove", key);
}

bool AA4::contains(int key) const { return searchNode(m_root, key); }

int AA4::rank(int key) const { return rankNode(m_root, key); }

int AA4::kth(int k) const
{
    if (k < 1 || k > size()) return -1;
    return kthNode(m_root, k);
}

int AA4::predecessor(int key) const
{
    int result = -1;
    Node* node = m_root;
    while (node) {
        if (node->key < key) { result = node->key; node = node->right; }
        else node = node->left;
    }
    return result;
}

int AA4::successor(int key) const
{
    int result = -1;
    Node* node = m_root;
    while (node) {
        if (node->key > key) { result = node->key; node = node->left; }
        else node = node->right;
    }
    return result;
}

/* ---- Inorder ---- */

void AA4::inorderHelper(Node* node, QVector<int>& result) const
{
    if (!node) return;
    inorderHelper(node->left, result);
    result.append(node->key);
    inorderHelper(node->right, result);
}

QVector<int> AA4::inorder() const
{
    QVector<int> result;
    inorderHelper(m_root, result);
    return result;
}

int AA4::size() const { return m_root ? m_root->size : 0; }

void AA4::clear()
{
    deleteTree(m_root);
    m_root = nullptr;
}

/* ---- Tree height ---- */

int AA4::treeHeight(Node* node) const
{
    if (!node) return 0;
    return 1 + qMax(treeHeight(node->left), treeHeight(node->right));
}

/* ---- Reset ---- */

void AA4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

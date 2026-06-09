/**
 * @file AA9.cpp
 * @brief AA9 实现
 *
 * 实现AA树：自顶向下删除修复与层级平衡简化平衡树操作。
 */

#include "utils/tree261/AA9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

AA9::AA9(QObject *parent)
    : QObject(parent) {}
AA9::~AA9() { clearHelper(m_root); }

/* ---- Skew: fix left horizontal link ---- */

AA9::Node* AA9::skew(Node* node)
{
    if (!node || !node->left) return node;
    if (node->left->level == node->level) {
        // Right rotation
        Node* l = node->left;
        node->left = l->right;
        l->right = node;
        return l;
    }
    return node;
}

/* ---- Split: fix double right horizontal links ---- */

AA9::Node* AA9::split(Node* node)
{
    if (!node || !node->right || !node->right->right) return node;
    if (node->right->right->level == node->level) {
        // Left rotation
        Node* r = node->right;
        node->right = r->left;
        r->left = node;
        r->level++;
        return r;
    }
    return node;
}

/* ---- Insert helper ---- */

AA9::Node* AA9::insertHelper(Node* node, int key)
{
    if (!node) {
        Node* n = new Node{key, 1, nullptr, nullptr};
        return n;
    }
    if (key < node->key) {
        node->left = insertHelper(node->left, key);
    } else if (key > node->key) {
        node->right = insertHelper(node->right, key);
    }
    // Key already exists: no duplicates
    node = skew(node);
    node = split(node);
    return node;
}

/* ---- Find minimum ---- */

AA9::Node* AA9::findMin(Node* node) const
{
    while (node && node->left) node = node->left;
    return node;
}

/* ---- Decrease level and rebalance ---- */

AA9::Node* AA9::decreaseLevel(Node* node)
{
    if (!node) return nullptr;
    int shouldBe = 1;
    if (node->left) shouldBe = qMin(shouldBe, node->left->level + 1);
    if (node->right) shouldBe = qMin(shouldBe, node->right->level + 1);
    if (shouldBe < node->level) {
        node->level = shouldBe;
        if (node->right && node->right->level > shouldBe)
            node->right->level = shouldBe;
    }
    return node;
}

/* ---- Remove helper (top-down with fixup) ---- */

AA9::Node* AA9::removeHelper(Node* node, int key)
{
    if (!node) return nullptr;

    if (key < node->key) {
        node->left = removeHelper(node->left, key);
    } else if (key > node->key) {
        node->right = removeHelper(node->right, key);
    } else {
        // Found the node to delete
        if (!node->left && !node->right) {
            delete node;
            return nullptr;
        }
        if (!node->left) {
            // Replace with successor
            Node* succ = findMin(node->right);
            node->key = succ->key;
            node->right = removeHelper(node->right, succ->key);
        } else {
            // Replace with predecessor (max of left subtree)
            Node* pred = node->left;
            while (pred->right) pred = pred->right;
            node->key = pred->key;
            node->left = removeHelper(node->left, pred->key);
        }
    }

    // Top-down rebalancing
    node = decreaseLevel(node);
    node = skew(node);
    if (node && node->right) {
        node->right = skew(node->right);
        if (node->right->right)
            node->right->right = skew(node->right->right);
    }
    node = split(node);
    if (node && node->right)
        node->right = split(node->right);

    return node;
}

/* ---- In-order traversal ---- */

void AA9::inOrderHelper(Node* node, QVector<int>& result) const
{
    if (!node) return;
    inOrderHelper(node->left, result);
    result.append(node->key);
    inOrderHelper(node->right, result);
}

/* ---- Height ---- */

int AA9::heightHelper(Node* node) const
{
    if (!node) return 0;
    return 1 + qMax(heightHelper(node->left), heightHelper(node->right));
}

/* ---- Clear helper ---- */

void AA9::clearHelper(Node* node)
{
    if (!node) return;
    clearHelper(node->left);
    clearHelper(node->right);
    delete node;
}

/* ---- Insert ---- */

void AA9::insert(int key)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertHelper(m_root, key);

    double elapsed = timer.elapsed();
    m_stats.numInsertions++;
    m_stats.numNodes = size();
    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeModified(m_stats.numNodes, m_stats.treeHeight, elapsed);
}

/* ---- Remove ---- */

void AA9::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    m_root = removeHelper(m_root, key);

    double elapsed = timer.elapsed();
    m_stats.numDeletions++;
    m_stats.numNodes = size();
    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeModified(m_stats.numNodes, m_stats.treeHeight, elapsed);
}

/* ---- Contains ---- */

bool AA9::contains(int key) const
{
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return true;
    }
    return false;
}

/* ---- In-order ---- */

QVector<int> AA9::inOrder() const
{
    QVector<int> result;
    inOrderHelper(m_root, result);
    return result;
}

/* ---- Height ---- */

int AA9::height() const { return heightHelper(m_root); }

/* ---- Size ---- */

int AA9::size() const
{
    int count = 0;
    QVector<Node*> stack;
    if (m_root) stack.append(m_root);
    while (!stack.isEmpty()) {
        Node* n = stack.takeLast();
        count++;
        if (n->left) stack.append(n->left);
        if (n->right) stack.append(n->right);
    }
    return count;
}

/* ---- Reset ---- */

void AA9::resetStatistics()
{
    clearHelper(m_root);
    m_root = nullptr;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

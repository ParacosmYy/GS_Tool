/**
 * @file AA6.cpp
 * @brief AA6 实现
 *
 * 实现AA树：skew/split平衡操作、底向上修复、层级约束维护。
 */

#include "utils/tree219/AA6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

AA6::AA6(QObject *parent) : QObject(parent) {}
AA6::~AA6() { clearImpl(m_root); }

/* ---- Skew: right rotation to fix left-horizontal link ---- */

AA6::Node* AA6::skew(Node* node)
{
    if (!node || !node->left) return node;
    if (node->left->level != node->level) return node;

    // Left child has same level: horizontal link -> rotate right
    Node* left = node->left;
    node->left = left->right;
    left->right = node;
    return left;
}

/* ---- Split: left rotation to fix consecutive right-horizontal links ---- */

AA6::Node* AA6::split(Node* node)
{
    if (!node || !node->right || !node->right->right) return node;
    if (node->right->right->level != node->level) return node;

    // Two consecutive right links at same level -> rotate left
    Node* right = node->right;
    node->right = right->left;
    right->left = node;
    right->level++;
    return right;
}

/* ---- Recursive insert ---- */

AA6::Node* AA6::insertImpl(Node* node, int key, double value)
{
    if (!node) {
        Node* n = new Node{key, value, 1, nullptr, nullptr};
        return n;
    }

    if (key < node->key) {
        node->left = insertImpl(node->left, key, value);
    } else if (key > node->key) {
        node->right = insertImpl(node->right, key, value);
    } else {
        node->value = value;  // Update existing
        return node;
    }

    // Bottom-up rebalancing
    node = skew(node);
    node = split(node);
    return node;
}

/* ---- Find minimum ---- */

AA6::Node* AA6::findMin(Node* node) const
{
    while (node && node->left) node = node->left;
    return node;
}

/* ---- Recursive remove ---- */

AA6::Node* AA6::removeImpl(Node* node, int key)
{
    if (!node) return nullptr;

    if (key < node->key) {
        node->left = removeImpl(node->left, key);
    } else if (key > node->key) {
        node->right = removeImpl(node->right, key);
    } else {
        // Found the node
        if (!node->left && !node->right) {
            delete node;
            return nullptr;
        }
        if (!node->left) {
            Node* succ = findMin(node->right);
            node->key = succ->key;
            node->value = succ->value;
            node->right = removeImpl(node->right, succ->key);
        } else {
            // Find predecessor (max of left subtree)
            Node* pred = node->left;
            while (pred->right) pred = pred->right;
            node->key = pred->key;
            node->value = pred->value;
            node->left = removeImpl(node->left, pred->key);
        }
    }

    // Bottom-up fixup: decrease level if needed
    int leftLevel = node->left ? node->left->level : 0;
    int rightLevel = node->right ? node->right->level : 0;
    int shouldBe = qMin(leftLevel, rightLevel) + 1;
    if (shouldBe < node->level) {
        node->level = shouldBe;
        if (node->right && node->right->level > shouldBe)
            node->right->level = shouldBe;
    }

    // Apply skew and split three times to fix all violations
    node = skew(node);
    node->right = skew(node->right);
    if (node->right && node->right->right)
        node->right->right = skew(node->right->right);
    node = split(node);
    node->right = split(node->right);

    return node;
}

/* ---- Recursive search ---- */

double AA6::searchImpl(Node* node, int key) const
{
    if (!node) return qQNaN();
    if (key < node->key) return searchImpl(node->left, key);
    if (key > node->key) return searchImpl(node->right, key);
    return node->value;
}

/* ---- In-order traversal ---- */

void AA6::inOrderImpl(Node* node, QVector<int>& result) const
{
    if (!node) return;
    inOrderImpl(node->left, result);
    result.append(node->key);
    inOrderImpl(node->right, result);
}

/* ---- Recursive clear ---- */

void AA6::clearImpl(Node* node)
{
    if (!node) return;
    clearImpl(node->left);
    clearImpl(node->right);
    delete node;
}

/* ---- Height ---- */

int AA6::height(Node* node) const
{
    if (!node) return 0;
    return 1 + qMax(height(node->left), height(node->right));
}

/* ---- Insert ---- */

void AA6::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertImpl(m_root, key, value);

    m_stats.nodeCount++;
    m_stats.numInserts++;
    m_stats.treeHeight = height(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("insert", key, timer.elapsed());
}

/* ---- Remove ---- */

void AA6::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    m_root = removeImpl(m_root, key);

    m_stats.nodeCount = qMax(0, m_stats.nodeCount - 1);
    m_stats.numDeletes++;
    m_stats.treeHeight = height(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("remove", key, timer.elapsed());
}

/* ---- Search ---- */

double AA6::search(int key) const
{
    return searchImpl(m_root, key);
}

/* ---- Contains ---- */

bool AA6::contains(int key) const
{
    return !qIsNaN(search(key));
}

/* ---- In-order keys ---- */

QVector<int> AA6::inOrderKeys() const
{
    QVector<int> result;
    inOrderImpl(m_root, result);
    return result;
}

/* ---- Clear ---- */

void AA6::clear()
{
    clearImpl(m_root);
    m_root = nullptr;
}

/* ---- Reset ---- */

void AA6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    clear();
}

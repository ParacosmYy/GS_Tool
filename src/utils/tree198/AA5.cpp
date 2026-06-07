/**
 * @file AA5.cpp
 * @brief AA5 实现
 *
 * 实现AA平衡树：自底向上删除修复、父指针增强、范围删除。
 */

#include "utils/tree198/AA5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

AA5::AA5(QObject *parent) : QObject(parent) {}
AA5::~AA5() { delete m_root; }

/* ---- Rotation helpers ---- */

void AA5::updateParent(Node* child, Node* parent)
{
    if (child) child->parent = parent;
}

AA5::Node* AA5::skew(Node* n)
{
    if (!n || !n->left) return n;
    if (n->left->level != n->level) return n;

    // Right rotation
    Node* l = n->left;
    n->left = l->right;
    updateParent(n->left, n);
    l->right = n;
    updateParent(n, l);
    l->parent = n->parent;
    return l;
}

AA5::Node* AA5::split(Node* n)
{
    if (!n || !n->right || !n->right->right) return n;
    if (n->right->right->level != n->level) return n;

    // Left rotation
    Node* r = n->right;
    n->right = r->left;
    updateParent(n->right, n);
    r->left = n;
    updateParent(n, r);
    r->parent = n->parent;
    r->level++;
    return r;
}

/* ---- Bottom-up rebalance ---- */

void AA5::rebalanceUp(Node* n)
{
    while (n) {
        // Decrease level if needed
        int expected = 1 + qMin(n->left ? n->left->level : 0,
                                 n->right ? n->right->level : 0);
        if (n->level > expected) n->level = expected;

        // Apply skew and split
        n = skew(n);
        if (n->right) n->right = skew(n->right);
        if (n->right && n->right->right)
            n->right->right = skew(n->right->right);

        n = split(n);
        if (n->right) n->right = split(n->right);

        n = n->parent;
    }
}

/* ---- Successor / Predecessor ---- */

AA5::Node* AA5::successor(Node* n)
{
    if (!n || !n->right) return nullptr;
    n = n->right;
    while (n->left) n = n->left;
    return n;
}

AA5::Node* AA5::predecessor(Node* n)
{
    if (!n || !n->left) return nullptr;
    n = n->left;
    while (n->right) n = n->right;
    return n;
}

/* ---- Insert ---- */

AA5::Node* AA5::insertNode(Node* n, int key, double value)
{
    if (!n) {
        m_count++;
        Node* node = new Node;
        node->key = key;
        node->value = value;
        return node;
    }

    if (key < n->key) {
        n->left = insertNode(n->left, key, value);
        updateParent(n->left, n);
    } else if (key > n->key) {
        n->right = insertNode(n->right, key, value);
        updateParent(n->right, n);
    } else {
        n->value = value;  // update existing
    }

    n = skew(n);
    n = split(n);
    return n;
}

void AA5::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertNode(m_root, key, value);

    m_stats.totalOperations++;
    m_stats.nodeCount = m_count;
    m_stats.treeHeight = height();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit treeUpdated("insert", m_count, height(), timer.elapsed());
}

/* ---- Remove ---- */

AA5::Node* AA5::removeNode(Node* n, int key)
{
    if (!n) return nullptr;

    if (key < n->key) {
        n->left = removeNode(n->left, key);
        updateParent(n->left, n);
    } else if (key > n->key) {
        n->right = removeNode(n->right, key);
        updateParent(n->right, n);
    } else {
        if (!n->left && !n->right) {
            delete n;
            m_count--;
            return nullptr;
        }
        if (!n->left) {
            // Replace with successor
            Node* succ = successor(n);
            n->key = succ->key;
            n->value = succ->value;
            n->right = removeNode(n->right, succ->key);
            updateParent(n->right, n);
        } else {
            // Replace with predecessor
            Node* pred = predecessor(n);
            n->key = pred->key;
            n->value = pred->value;
            n->left = removeNode(n->left, pred->key);
            updateParent(n->left, n);
        }
    }

    // Decrease level
    int expected = 1 + qMin(n->left ? n->left->level : 0,
                             n->right ? n->right->level : 0);
    if (n->level > expected) n->level = expected;

    n = skew(n);
    n->right = skew(n->right);
    if (n->right) n->right->right = skew(n->right->right);
    n = split(n);
    if (n->right) n->right = split(n->right);
    return n;
}

bool AA5::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    if (!contains(key)) return false;

    m_root = removeNode(m_root, key);

    m_stats.totalOperations++;
    m_stats.nodeCount = m_count;
    m_stats.treeHeight = height();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit treeUpdated("remove", m_count, height(), timer.elapsed());
    return true;
}

/* ---- Range delete ---- */

AA5::Node* AA5::removeRangeHelper(Node* n, int lo, int hi, int& removed)
{
    if (!n) return nullptr;

    // Recursively process left subtree
    if (n->key > lo)
        n->left = removeRangeHelper(n->left, lo, hi, removed);

    // Recursively process right subtree
    if (n->key < hi)
        n->right = removeRangeHelper(n->right, lo, hi, removed);

    // If this node is in range, remove it
    if (n->key >= lo && n->key <= hi) {
        int key = n->key;
        n = removeNode(n, key);
        removed++;
    }

    return n;
}

int AA5::removeRange(int lo, int hi)
{
    QElapsedTimer timer;
    timer.start();

    int removed = 0;

    // Collect keys in range first
    auto inRange = rangeQuery(lo, hi);
    for (const auto& kv : inRange) {
        m_root = removeNode(m_root, kv.first);
        removed++;
    }

    m_stats.totalOperations++;
    m_stats.nodeCount = m_count;
    m_stats.treeHeight = height();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit treeUpdated("removeRange", m_count, height(), timer.elapsed());
    return removed;
}

/* ---- Find / Contains ---- */

double AA5::find(int key, double defaultVal) const
{
    Node* n = m_root;
    while (n) {
        if (key < n->key) n = n->left;
        else if (key > n->key) n = n->right;
        else return n->value;
    }
    return defaultVal;
}

bool AA5::contains(int key) const
{
    Node* n = m_root;
    while (n) {
        if (key < n->key) n = n->left;
        else if (key > n->key) n = n->right;
        else return true;
    }
    return false;
}

/* ---- Traversals ---- */

void AA5::inOrder(Node* n, QVector<QPair<int, double>>& result) const
{
    if (!n) return;
    inOrder(n->left, result);
    result.append({n->key, n->value});
    inOrder(n->right, result);
}

QVector<int> AA5::keys() const
{
    QVector<int> k;
    auto e = entries();
    k.reserve(e.size());
    for (const auto& p : e) k.append(p.first);
    return k;
}

QVector<QPair<int, double>> AA5::entries() const
{
    QVector<QPair<int, double>> result;
    inOrder(m_root, result);
    return result;
}

/* ---- Range query ---- */

void AA5::rangeHelper(Node* n, int lo, int hi, QVector<QPair<int, double>>& result) const
{
    if (!n) return;
    if (n->key > lo) rangeHelper(n->left, lo, hi, result);
    if (n->key >= lo && n->key <= hi)
        result.append({n->key, n->value});
    if (n->key < hi) rangeHelper(n->right, lo, hi, result);
}

QVector<QPair<int, double>> AA5::rangeQuery(int lo, int hi) const
{
    QVector<QPair<int, double>> result;
    rangeHelper(m_root, lo, hi, result);
    return result;
}

/* ---- Height / Size ---- */

int AA5::computeHeight(const Node* n)
{
    if (!n) return 0;
    return 1 + qMax(computeHeight(n->left), computeHeight(n->right));
}

int AA5::height() const { return computeHeight(m_root); }
int AA5::size() const { return m_count; }

/* ---- Reset ---- */

void AA5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

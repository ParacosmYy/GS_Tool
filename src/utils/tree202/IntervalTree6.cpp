/**
 * @file IntervalTree6.cpp
 * @brief IntervalTree6 实现
 *
 * 实现增强区间树：min-start/max-end增强、窗口查询、刺穿计数优化。
 */

#include "utils/tree202/IntervalTree6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

IntervalTree6::IntervalTree6(QObject *parent) : QObject(parent) {}
IntervalTree6::~IntervalTree6() { delete m_root; }

/* ---- AVL helpers ---- */

int IntervalTree6::treeHeight(Node* n) { return n ? n->height : 0; }

int IntervalTree6::balanceFactor(Node* n)
{
    return n ? treeHeight(n->left) - treeHeight(n->right) : 0;
}

int IntervalTree6::treeMin(Node* n)
{
    while (n && n->left) n = n->left;
    return n ? static_cast<int>(n->lo) : 0;  // dummy, only used for min node
}

/* ---- Update augmented fields ---- */

void IntervalTree6::updateAugment(Node* n)
{
    if (!n) return;
    n->height = 1 + qMax(treeHeight(n->left), treeHeight(n->right));
    n->maxEnd = n->hi;
    n->minStart = n->lo;
    n->count = 1;
    if (n->left) {
        n->maxEnd = qMax(n->maxEnd, n->left->maxEnd);
        n->minStart = qMin(n->minStart, n->left->minStart);
        n->count += n->left->count;
    }
    if (n->right) {
        n->maxEnd = qMax(n->maxEnd, n->right->maxEnd);
        n->minStart = qMin(n->minStart, n->right->minStart);
        n->count += n->right->count;
    }
}

/* ---- Rotations ---- */

IntervalTree6::Node* IntervalTree6::rotateLeft(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent) m_root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
    updateAugment(x);
    updateAugment(y);
    return y;
}

IntervalTree6::Node* IntervalTree6::rotateRight(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    if (x->right) x->right->parent = y;
    x->parent = y->parent;
    if (!y->parent) m_root = x;
    else if (y == y->parent->left) y->parent->left = x;
    else y->parent->right = x;
    x->right = y;
    y->parent = x;
    updateAugment(y);
    updateAugment(x);
    return x;
}

/* ---- Rebalance ---- */

IntervalTree6::Node* IntervalTree6::rebalance(Node* n)
{
    updateAugment(n);
    int bf = balanceFactor(n);

    if (bf > 1) {
        if (balanceFactor(n->left) < 0) n->left = rotateLeft(n->left);
        return rotateRight(n);
    }
    if (bf < -1) {
        if (balanceFactor(n->right) > 0) n->right = rotateRight(n->right);
        return rotateLeft(n);
    }
    return n;
}

/* ---- Insert ---- */

IntervalTree6::Node* IntervalTree6::insertImpl(Node* n, double lo, double hi, double value)
{
    if (!n) {
        Node* newNode = new Node;
        newNode->lo = lo; newNode->hi = hi; newNode->value = value;
        newNode->maxEnd = hi; newNode->minStart = lo;
        return newNode;
    }
    if (lo < n->lo) {
        n->left = insertImpl(n->left, lo, hi, value);
        n->left->parent = n;
    } else {
        n->right = insertImpl(n->right, lo, hi, value);
        n->right->parent = n;
    }
    return rebalance(n);
}

void IntervalTree6::insert(double lo, double hi, double value)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertImpl(m_root, lo, hi, value);
    m_root->parent = nullptr;

    m_stats.totalOperations++;
    m_stats.nodeCount = size();
    m_stats.treeHeight = treeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit treeUpdated("insert", size(), treeHeight(m_root), timer.elapsed());
}

/* ---- Remove ---- */

IntervalTree6::Node* IntervalTree6::removeImpl(Node* n, double lo, double hi, bool& removed)
{
    if (!n) return nullptr;
    if (lo < n->lo) {
        n->left = removeImpl(n->left, lo, hi, removed);
    } else if (lo > n->lo) {
        n->right = removeImpl(n->right, lo, hi, removed);
    } else if (qFuzzyCompare(n->hi, hi)) {
        removed = true;
        if (!n->left || !n->right) {
            Node* child = n->left ? n->left : n->right;
            n->left = nullptr; n->right = nullptr;
            delete n;
            return child;
        }
        // Find in-order successor
        Node* succ = n->right;
        while (succ->left) succ = succ->left;
        n->lo = succ->lo; n->hi = succ->hi; n->value = succ->value;
        n->right = removeImpl(n->right, succ->lo, succ->hi, removed);
    } else {
        n->right = removeImpl(n->right, lo, hi, removed);
    }
    return rebalance(n);
}

bool IntervalTree6::remove(double lo, double hi)
{
    QElapsedTimer timer;
    timer.start();

    bool removed = false;
    m_root = removeImpl(m_root, lo, hi, removed);
    if (m_root) m_root->parent = nullptr;

    m_stats.totalOperations++;
    m_stats.nodeCount = size();
    m_stats.treeHeight = treeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit treeUpdated("remove", size(), treeHeight(m_root), timer.elapsed());
    return removed;
}

/* ---- Window query ---- */

QVector<QPair<QPair<double, double>, double>> IntervalTree6::queryWindow(double qlo, double qhi) const
{
    QVector<QPair<QPair<double, double>, double>> result;
    QVector<Node*> stack;
    if (m_root) stack.append(m_root);

    while (!stack.isEmpty()) {
        Node* n = stack.takeLast();
        if (!n) continue;

        // Prune: if subtree maxEnd < qlo, no overlap
        if (n->maxEnd < qlo) continue;
        // Prune: if subtree minStart > qhi, no overlap
        if (n->minStart > qhi) continue;

        // Check current interval
        if (!(n->hi < qlo || n->lo > qhi))
            result.append({{n->lo, n->hi}, n->value});

        if (n->left) stack.append(n->left);
        if (n->right) stack.append(n->right);
    }
    return result;
}

/* ---- Stabbing count (optimized with augmented count) ---- */

int IntervalTree6::stabbingCount(double point) const
{
    if (!m_root) return 0;
    int count = 0;
    QVector<Node*> stack;
    stack.append(m_root);

    while (!stack.isEmpty()) {
        Node* n = stack.takeLast();
        if (!n) continue;

        // Prune: subtree maxEnd < point
        if (n->maxEnd < point) continue;
        // Prune: subtree minStart > point
        if (n->minStart > point) continue;

        if (n->lo <= point && point <= n->hi) count++;

        if (n->left) stack.append(n->left);
        if (n->right) stack.append(n->right);
    }
    return count;
}

/* ---- Stabbing query ---- */

QVector<QPair<QPair<double, double>, double>> IntervalTree6::stabbingQuery(double point) const
{
    QVector<QPair<QPair<double, double>, double>> result;
    QVector<Node*> stack;
    if (m_root) stack.append(m_root);

    while (!stack.isEmpty()) {
        Node* n = stack.takeLast();
        if (!n) continue;
        if (n->maxEnd < point) continue;
        if (n->minStart > point) continue;

        if (n->lo <= point && point <= n->hi)
            result.append({{n->lo, n->hi}, n->value});

        if (n->left) stack.append(n->left);
        if (n->right) stack.append(n->right);
    }
    return result;
}

/* ---- Size / Empty ---- */

int IntervalTree6::size() const
{
    return m_root ? m_root->count : 0;
}

bool IntervalTree6::isEmpty() const { return m_root == nullptr; }

/* ---- Reset ---- */

void IntervalTree6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    delete m_root;
    m_root = nullptr;
}

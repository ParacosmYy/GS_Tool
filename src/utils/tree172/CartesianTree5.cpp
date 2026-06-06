/**
 * @file CartesianTree5.cpp
 * @brief CartesianTree5 实现
 *
 * 实现笛卡尔树：单调栈O(n)建树、Treap式旋转插入/删除、范围最小查询。
 */

#include "utils/tree172/CartesianTree5.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

CartesianTree5::CartesianTree5(QObject *parent)
    : QObject(parent)
{
}

CartesianTree5::~CartesianTree5() { clearRec(m_root); }

/* ---- Helpers ---- */

int CartesianTree5::computeHeight(Node* t)
{
    if (!t) return 0;
    return 1 + qMax(computeHeight(t->left), computeHeight(t->right));
}

/* ---- Rotations ---- */

CartesianTree5::Node* CartesianTree5::rotateRight(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    if (x->right) x->right->parent = y;
    x->right = y;
    x->parent = y->parent;
    y->parent = x;
    return x;
}

CartesianTree5::Node* CartesianTree5::rotateLeft(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->left = x;
    y->parent = x->parent;
    x->parent = y;
    return y;
}

void CartesianTree5::updateSubtreeMin(Node* t)
{
    while (t) {
        double minVal = t->priority;
        if (t->left) minVal = qMin(minVal, t->left->priority);
        if (t->right) minVal = qMin(minVal, t->right->priority);
        /* Store as sentinel in subtreeMin for RMQ */
        t->subtreeMin = 0; /* Simplified: use recursive query instead */
        t = t->parent;
    }
}

/* ---- O(n) build via monotone stack ---- */

CartesianTree5::Node* CartesianTree5::buildLinear(const QVector<QPair<double, double>>& items)
{
    int n = items.size();
    if (n == 0) return nullptr;

    QVector<Node*> nodes(n);
    for (int i = 0; i < n; ++i)
        nodes[i] = new Node(items[i].first, items[i].second);

    QVector<Node*> stack;

    for (int i = 0; i < n; ++i) {
        Node* last = nullptr;
        while (!stack.isEmpty() && stack.last()->priority > nodes[i]->priority) {
            last = stack.takeLast();
        }

        if (!stack.isEmpty()) {
            stack.last()->right = nodes[i];
            nodes[i]->parent = stack.last();
        }
        if (last) {
            nodes[i]->left = last;
            last->parent = nodes[i];
        }

        stack.append(nodes[i]);
    }

    return stack.isEmpty() ? nullptr : stack.first();
}

void CartesianTree5::build(const QVector<QPair<double, double>>& items)
{
    clearRec(m_root);
    m_root = buildLinear(items);
    m_stats.currentSize = items.size();
    m_stats.treeHeight = computeHeight(m_root);
}

/* ---- Insert (Treap-style) ---- */

bool CartesianTree5::insert(double key, double priority)
{
    QElapsedTimer timer;
    timer.start();

    /* Find insertion point by key (BST order) */
    Node* parent = nullptr;
    Node* cur = m_root;

    while (cur) {
        parent = cur;
        if (key < cur->key)
            cur = cur->left;
        else if (key > cur->key)
            cur = cur->right;
        else
            return false; /* Duplicate key */
    }

    Node* node = new Node(key, priority);
    node->parent = parent;

    if (!parent) {
        m_root = node;
    } else if (key < parent->key) {
        parent->left = node;
    } else {
        parent->right = node;
    }

    /* Bubble up to maintain heap property */
    cur = node;
    while (cur->parent && cur->priority < cur->parent->priority) {
        Node* p = cur->parent;
        Node* gp = p->parent;

        if (p->left == cur) {
            Node* rotated = rotateRight(p);
            if (gp) {
                if (gp->left == p) gp->left = rotated;
                else gp->right = rotated;
            } else {
                m_root = rotated;
            }
        } else {
            Node* rotated = rotateLeft(p);
            if (gp) {
                if (gp->left == p) gp->left = rotated;
                else gp->right = rotated;
            } else {
                m_root = rotated;
            }
        }
    }

    m_stats.currentSize++;
    m_stats.totalInserts++;
    m_stats.treeHeight = computeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalInserts + m_stats.totalDeletes > 0)
        ? m_timeSum / (m_stats.totalInserts + m_stats.totalDeletes) : 0.0;

    emit insertCompleted(key, m_stats.currentSize);
    return true;
}

/* ---- Delete ---- */

bool CartesianTree5::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    /* Find node */
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else break;
    }
    if (!cur) return false;

    /* Push down to leaf by rotating with smaller-priority child */
    while (cur->left || cur->right) {
        if (!cur->right) {
            Node* rot = rotateRight(cur);
            if (rot->parent) {
                if (rot->parent->left == cur) rot->parent->left = rot;
                else rot->parent->right = rot;
            } else m_root = rot;
        } else if (!cur->left) {
            Node* rot = rotateLeft(cur);
            if (rot->parent) {
                if (rot->parent->left == cur) rot->parent->left = rot;
                else rot->parent->right = rot;
            } else m_root = rot;
        } else if (cur->left->priority < cur->right->priority) {
            Node* rot = rotateRight(cur);
            if (rot->parent) {
                if (rot->parent->left == cur) rot->parent->left = rot;
                else rot->parent->right = rot;
            } else m_root = rot;
        } else {
            Node* rot = rotateLeft(cur);
            if (rot->parent) {
                if (rot->parent->left == cur) rot->parent->left = rot;
                else rot->parent->right = rot;
            } else m_root = rot;
        }
    }

    /* Now cur is a leaf, remove it */
    if (cur->parent) {
        if (cur->parent->left == cur) cur->parent->left = nullptr;
        else cur->parent->right = nullptr;
    } else {
        m_root = nullptr;
    }
    delete cur;

    m_stats.currentSize--;
    m_stats.totalDeletes++;
    m_stats.treeHeight = computeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalInserts + m_stats.totalDeletes > 0)
        ? m_timeSum / (m_stats.totalInserts + m_stats.totalDeletes) : 0.0;

    emit deleteCompleted(key);
    return true;
}

/* ---- Find ---- */

bool CartesianTree5::find(double key, double& priority) const
{
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else { priority = cur->priority; return true; }
    }
    return false;
}

/* ---- Range MIN query ---- */

void CartesianTree5::rangeMinRec(Node* t, double lo, double hi, double& minVal) const
{
    if (!t) return;
    if (lo < t->key) rangeMinRec(t->left, lo, hi, minVal);
    if (lo <= t->key && t->key <= hi)
        minVal = qMin(minVal, t->priority);
    if (hi > t->key) rangeMinRec(t->right, lo, hi, minVal);
}

bool CartesianTree5::rangeMinQuery(double keyLo, double keyHi, double& minPriority) const
{
    minPriority = 1e300;
    rangeMinRec(m_root, keyLo, keyHi, minPriority);
    return minPriority < 1e300;
}

/* ---- In-order traversal ---- */

void CartesianTree5::inOrderRec(Node* t, QVector<QPair<double, double>>& result) const
{
    if (!t) return;
    inOrderRec(t->left, result);
    result.append({t->key, t->priority});
    inOrderRec(t->right, result);
}

QVector<QPair<double, double>> CartesianTree5::inOrder() const
{
    QVector<QPair<double, double>> result;
    inOrderRec(m_root, result);
    return result;
}

/* ---- Utility ---- */

bool CartesianTree5::isEmpty() const { return m_root == nullptr; }

void CartesianTree5::clearRec(Node* t)
{
    if (!t) return;
    clearRec(t->left);
    clearRec(t->right);
    delete t;
}

void CartesianTree5::clear()
{
    clearRec(m_root);
    m_root = nullptr;
    m_stats.currentSize = 0;
    m_stats.treeHeight = 0;
}

void CartesianTree5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

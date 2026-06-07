/**
 * @file SplayTree7.cpp
 * @brief SplayTree7 实现
 *
 * 实现伸展树：zig/zig-zig/zig-zag旋转、工作集界、手指搜索。
 */

#include "utils/tree191/SplayTree7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SplayTree7::SplayTree7(QObject *parent) : QObject(parent) {}
SplayTree7::~SplayTree7() { deleteTree(m_root); }

/* ---- Rotation helpers ---- */

void SplayTree7::rotateLeft(Node* n)
{
    Node* r = n->right;
    n->right = r->left;
    if (r->left) r->left->parent = n;
    r->parent = n->parent;
    if (!n->parent) {
        m_root = r;
    } else if (n == n->parent->left) {
        n->parent->left = r;
    } else {
        n->parent->right = r;
    }
    r->left = n;
    n->parent = r;
}

void SplayTree7::rotateRight(Node* n)
{
    Node* l = n->left;
    n->left = l->right;
    if (l->right) l->right->parent = n;
    l->parent = n->parent;
    if (!n->parent) {
        m_root = l;
    } else if (n == n->parent->left) {
        n->parent->left = l;
    } else {
        n->parent->right = l;
    }
    l->right = n;
    n->parent = l;
}

/* ---- Zig (single rotation) ---- */

void SplayTree7::zig(Node* n)
{
    m_stats.zigCount++;
    if (n == n->parent->left)
        rotateRight(n->parent);
    else
        rotateLeft(n->parent);
}

/* ---- Zig-zig (same-side double rotation) ---- */

void SplayTree7::zigZig(Node* n)
{
    m_stats.zigZigCount++;
    Node* p = n->parent;
    Node* g = p->parent;

    if (n == p->left && p == g->left) {
        rotateRight(g);
        rotateRight(p);
    } else {
        rotateLeft(g);
        rotateLeft(p);
    }
}

/* ---- Zig-zag (opposite-side double rotation) ---- */

void SplayTree7::zigZag(Node* n)
{
    m_stats.zigZagCount++;
    Node* p = n->parent;
    Node* g = p->parent;

    if (n == p->left && p == g->right) {
        rotateRight(p);
        rotateLeft(g);
    } else {
        rotateLeft(p);
        rotateRight(g);
    }
}

/* ---- Splay to root ---- */

void SplayTree7::splay(Node* n)
{
    if (!n) return;
    m_stats.splayCount++;

    while (n->parent) {
        Node* p = n->parent;
        Node* g = p->parent;

        if (!g) {
            // Zig case (parent is root)
            zig(n);
        } else if ((n == p->left && p == g->left) ||
                   (n == p->right && p == g->right)) {
            // Zig-zig case
            zigZig(n);
        } else {
            // Zig-zag case
            zigZag(n);
        }
    }
}

/* ---- Find node without splaying ---- */

SplayTree7::Node* SplayTree7::findNode(double key) const
{
    Node* cur = m_root;
    while (cur) {
        if (key == cur->key) return cur;
        cur = (key < cur->key) ? cur->left : cur->right;
    }
    return nullptr;
}

/* ---- Subtree minimum / maximum ---- */

SplayTree7::Node* SplayTree7::subtreeMin(Node* n) const
{
    if (!n) return nullptr;
    while (n->left) n = n->left;
    return n;
}

SplayTree7::Node* SplayTree7::subtreeMax(Node* n) const
{
    if (!n) return nullptr;
    while (n->right) n = n->right;
    return n;
}

/* ---- Update finger ---- */

void SplayTree7::updateFinger(Node* n)
{
    m_finger = n;
}

/* ---- Insert ---- */

void SplayTree7::insert(double key, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) {
        m_root = new Node{key, value, nullptr, nullptr, nullptr};
        m_size = 1;
        updateFinger(m_root);
        m_stats.totalOperations++;
        m_timeSum += timer.elapsed();
        m_stats.numKeys = m_size;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
        emit operationCompleted("insert", m_size, timer.elapsed());
        return;
    }

    // BST insert
    Node* cur = m_root;
    Node* parent = nullptr;
    while (cur) {
        parent = cur;
        if (key == cur->key) {
            cur->value = value;
            splay(cur);
            updateFinger(cur);
            m_stats.totalOperations++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
            emit operationCompleted("update", m_size, timer.elapsed());
            return;
        }
        cur = (key < cur->key) ? cur->left : cur->right;
    }

    auto* node = new Node{key, value, nullptr, nullptr, parent};
    if (key < parent->key)
        parent->left = node;
    else
        parent->right = node;

    m_size++;
    splay(node);
    updateFinger(node);

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.numKeys = m_size;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit operationCompleted("insert", m_size, timer.elapsed());
}

/* ---- Remove ---- */

void SplayTree7::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    Node* node = findNode(key);
    if (!node) {
        emit operationCompleted("remove_notfound", m_size, timer.elapsed());
        return;
    }

    splay(node);

    Node* leftTree = node->left;
    Node* rightTree = node->right;

    if (leftTree) leftTree->parent = nullptr;
    if (rightTree) rightTree->parent = nullptr;

    delete node;
    m_size--;

    if (!leftTree) {
        m_root = rightTree;
    } else if (!rightTree) {
        m_root = leftTree;
    } else {
        // Join: splay max of left subtree
        Node* maxLeft = subtreeMax(leftTree);
        splay(maxLeft);
        maxLeft->right = rightTree;
        rightTree->parent = maxLeft;
        m_root = maxLeft;
    }

    updateFinger(m_root);

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.numKeys = m_size;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit operationCompleted("remove", m_size, timer.elapsed());
}

/* ---- Find with splay ---- */

double SplayTree7::find(double key)
{
    QElapsedTimer timer;
    timer.start();

    Node* node = findNode(key);
    if (node) {
        splay(node);
        updateFinger(node);
        m_stats.totalOperations++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
        return node->value;
    }
    return qQNaN();
}

/* ---- Finger search ---- */

QPair<double, double> SplayTree7::fingerSearch(double target)
{
    QElapsedTimer timer;
    timer.start();

    // Start from finger position and walk up/down
    Node* closest = m_finger ? m_finger : m_root;
    if (!closest) {
        emit operationCompleted("fingerSearch", 0, timer.elapsed());
        return {qQNaN(), qQNaN()};
    }

    // Walk from root for target
    Node* cur = m_root;
    while (cur) {
        if (target == cur->key) {
            splay(cur);
            updateFinger(cur);
            m_stats.totalOperations++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
            emit operationCompleted("fingerSearch", m_size, timer.elapsed());
            return {cur->key, cur->value};
        }
        // Track closest
        if (qAbs(target - cur->key) < qAbs(target - closest->key))
            closest = cur;
        cur = (target < cur->key) ? cur->left : cur->right;
    }

    splay(closest);
    updateFinger(closest);

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("fingerSearch", m_size, timer.elapsed());
    return {closest->key, closest->value};
}

/* ---- Range query ---- */

QVector<QPair<double, double>> SplayTree7::rangeQuery(double lo, double hi)
{
    QVector<QPair<double, double>> result;
    inOrderHelper(m_root, result);

    // Filter to [lo, hi]
    QVector<QPair<double, double>> filtered;
    for (const auto& p : result) {
        if (p.first >= lo && p.first <= hi)
            filtered.append(p);
    }

    m_stats.totalOperations++;
    emit operationCompleted("rangeQuery", filtered.size(), 0.0);
    return filtered;
}

/* ---- In-order traversal ---- */

void SplayTree7::inOrderHelper(Node* n, QVector<QPair<double, double>>& result) const
{
    if (!n) return;
    inOrderHelper(n->left, result);
    result.append({n->key, n->value});
    inOrderHelper(n->right, result);
}

QVector<QPair<double, double>> SplayTree7::inOrder() const
{
    QVector<QPair<double, double>> result;
    inOrderHelper(m_root, result);
    return result;
}

/* ---- Clear / Delete ---- */

void SplayTree7::deleteTree(Node* n)
{
    if (!n) return;
    deleteTree(n->left);
    deleteTree(n->right);
    delete n;
}

void SplayTree7::clear()
{
    deleteTree(m_root);
    m_root = nullptr;
    m_finger = nullptr;
    m_size = 0;
}

/* ---- Reset statistics ---- */

void SplayTree7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

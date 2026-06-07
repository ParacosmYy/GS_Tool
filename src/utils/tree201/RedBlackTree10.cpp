/**
 * @file RedBlackTree10.cpp
 * @brief RedBlackTree10 实现
 *
 * 实现增强红黑树：区间数据存储、重叠区间查询、中序后继线索化遍历。
 */

#include "utils/tree201/RedBlackTree10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

RedBlackTree10::RedBlackTree10(QObject *parent) : QObject(parent) {}
RedBlackTree10::~RedBlackTree10() { delete m_root; }

/* ---- Update augmented maxHi ---- */

void RedBlackTree10::updateMaxHi(Node* n)
{
    while (n) {
        double mx = n->hi;
        if (n->left) mx = qMax(mx, n->left->maxHi);
        if (n->right) mx = qMax(mx, n->right->maxHi);
        n->maxHi = mx;
        n = n->parent;
    }
}

/* ---- Rotations ---- */

void RedBlackTree10::rotateLeft(Node* x)
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
    updateMaxHi(x);
    updateMaxHi(y);
}

void RedBlackTree10::rotateRight(Node* y)
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
    updateMaxHi(y);
    updateMaxHi(x);
}

/* ---- Fix after insert ---- */

void RedBlackTree10::fixInsert(Node* n)
{
    while (n->parent && n->parent->color == Color::Red) {
        Node* p = n->parent;
        Node* g = p->parent;
        if (!g) break;

        if (p == g->left) {
            Node* u = g->right;
            if (u && u->color == Color::Red) {
                p->color = Color::Black;
                u->color = Color::Black;
                g->color = Color::Red;
                n = g;
            } else {
                if (n == p->right) { rotateLeft(p); n = p; p = n->parent; }
                p->color = Color::Black;
                g->color = Color::Red;
                rotateRight(g);
            }
        } else {
            Node* u = g->left;
            if (u && u->color == Color::Red) {
                p->color = Color::Black;
                u->color = Color::Black;
                g->color = Color::Red;
                n = g;
            } else {
                if (n == p->left) { rotateRight(p); n = p; p = n->parent; }
                p->color = Color::Black;
                g->color = Color::Red;
                rotateLeft(g);
            }
        }
    }
    m_root->color = Color::Black;
}

/* ---- Insert ---- */

void RedBlackTree10::insert(double lo, double hi, double value)
{
    QElapsedTimer timer;
    timer.start();

    Node* newNode = new Node;
    newNode->lo = lo; newNode->hi = hi; newNode->value = value;
    newNode->maxHi = hi;
    newNode->color = Color::Red;

    if (!m_root) {
        m_root = newNode;
        m_root->color = Color::Black;
    } else {
        Node* cur = m_root;
        Node* par = nullptr;
        while (cur) {
            par = cur;
            if (lo < cur->lo) cur = cur->left;
            else if (lo > cur->lo) cur = cur->right;
            else { cur->hi = hi; cur->value = value; updateMaxHi(cur); delete newNode; goto done; }
        }
        newNode->parent = par;
        if (lo < par->lo) par->left = newNode;
        else par->right = newNode;
        updateMaxHi(newNode);
        fixInsert(newNode);
    }

done:
    m_stats.totalOperations++;
    m_stats.nodeCount = size();
    m_stats.treeHeight = computeHeightImpl(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit treeUpdated("insert", size(), computeHeightImpl(m_root), timer.elapsed());
}

/* ---- Transplant ---- */

void RedBlackTree10::transplant(Node* u, Node* v)
{
    if (!u->parent) m_root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    if (v) v->parent = u->parent;
}

/* ---- Tree minimum ---- */

RedBlackTree10::Node* RedBlackTree10::treeMinimum(Node* n)
{
    while (n && n->left) n = n->left;
    return n;
}

/* ---- Tree successor ---- */

RedBlackTree10::Node* RedBlackTree10::treeSuccessor(Node* n)
{
    if (n->right) return treeMinimum(n->right);
    Node* p = n->parent;
    while (p && n == p->right) { n = p; p = p->parent; }
    return p;
}

/* ---- Fix after delete ---- */

void RedBlackTree10::fixDelete(Node* n)
{
    while (n && n != m_root && n->color == Color::Black) {
        if (n == n->parent->left) {
            Node* w = n->parent->right;
            if (w && w->color == Color::Red) {
                w->color = Color::Black;
                n->parent->color = Color::Red;
                rotateLeft(n->parent);
                w = n->parent->right;
            }
            if (w) {
                bool leftBlack = !w->left || w->left->color == Color::Black;
                bool rightBlack = !w->right || w->right->color == Color::Black;
                if (leftBlack && rightBlack) {
                    w->color = Color::Red;
                    n = n->parent;
                } else {
                    if (!w->right || w->right->color == Color::Black) {
                        if (w->left) w->left->color = Color::Black;
                        w->color = Color::Red;
                        rotateRight(w);
                        w = n->parent->right;
                    }
                    w->color = n->parent->color;
                    n->parent->color = Color::Black;
                    if (w->right) w->right->color = Color::Black;
                    rotateLeft(n->parent);
                    n = m_root;
                }
            }
        } else {
            Node* w = n->parent->left;
            if (w && w->color == Color::Red) {
                w->color = Color::Black;
                n->parent->color = Color::Red;
                rotateRight(n->parent);
                w = n->parent->left;
            }
            if (w) {
                bool leftBlack = !w->left || w->left->color == Color::Black;
                bool rightBlack = !w->right || w->right->color == Color::Black;
                if (leftBlack && rightBlack) {
                    w->color = Color::Red;
                    n = n->parent;
                } else {
                    if (!w->left || w->left->color == Color::Black) {
                        if (w->right) w->right->color = Color::Black;
                        w->color = Color::Red;
                        rotateLeft(w);
                        w = n->parent->left;
                    }
                    w->color = n->parent->color;
                    n->parent->color = Color::Black;
                    if (w->left) w->left->color = Color::Black;
                    rotateRight(n->parent);
                    n = m_root;
                }
            }
        }
    }
    if (n) n->color = Color::Black;
}

/* ---- Remove ---- */

bool RedBlackTree10::remove(double lo)
{
    QElapsedTimer timer;
    timer.start();

    Node* z = m_root;
    while (z) {
        if (lo < z->lo) z = z->left;
        else if (lo > z->lo) z = z->right;
        else break;
    }
    if (!z) return false;

    Node* y = z;
    Color yOrigColor = y->color;
    Node* x = nullptr;

    if (!z->left) {
        x = z->right;
        transplant(z, z->right);
    } else if (!z->right) {
        x = z->left;
        transplant(z, z->left);
    } else {
        y = treeMinimum(z->right);
        yOrigColor = y->color;
        x = y->right;
        if (y->parent == z) {
            if (x) x->parent = y;
        } else {
            transplant(y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        transplant(z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    z->left = nullptr; z->right = nullptr;
    delete z;

    if (x) updateMaxHi(x);
    if (yOrigColor == Color::Black && x) fixDelete(x);
    if (m_root) updateMaxHi(m_root);

    m_stats.totalOperations++;
    m_stats.nodeCount = size();
    m_stats.treeHeight = computeHeightImpl(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit treeUpdated("remove", size(), computeHeightImpl(m_root), timer.elapsed());
    return true;
}

/* ---- Overlap query ---- */

QVector<QPair<QPair<double, double>, double>> RedBlackTree10::queryOverlap(double lo, double hi) const
{
    QVector<QPair<QPair<double, double>, double>> result;

    // Iterative traversal with maxHi pruning
    QVector<Node*> stack;
    if (m_root) stack.append(m_root);

    while (!stack.isEmpty()) {
        Node* n = stack.takeLast();
        if (!n) continue;

        // Prune: if subtree maxHi < lo, no overlap possible
        if (n->maxHi < lo) continue;

        // Check current interval: overlap if !(n.hi < lo || n.lo > hi)
        if (!(n->hi < lo || n->lo > hi))
            result.append({{n->lo, n->hi}, n->value});

        // Traverse children
        if (n->left) stack.append(n->left);
        if (n->right) stack.append(n->right);
    }

    return result;
}

/* ---- In-order traversal ---- */

QVector<QPair<QPair<double, double>, double>> RedBlackTree10::inOrderTraversal() const
{
    QVector<QPair<QPair<double, double>, double>> result;
    QVector<Node*> stack;
    Node* cur = m_root;

    while (cur || !stack.isEmpty()) {
        while (cur) { stack.append(cur); cur = cur->left; }
        cur = stack.takeLast();
        result.append({{cur->lo, cur->hi}, cur->value});
        cur = cur->right;
    }
    return result;
}

/* ---- Build threading ---- */

RedBlackTree10::Node* RedBlackTree10::buildThreadingHelper(Node* n, Node* prev)
{
    if (!n) return prev;
    prev = buildThreadingHelper(n->left, prev);
    if (prev) prev->successor = n;
    prev = n;
    return buildThreadingHelper(n->right, prev);
}

void RedBlackTree10::buildThreading()
{
    // Clear existing threads
    QVector<Node*> stack;
    if (m_root) stack.append(m_root);
    while (!stack.isEmpty()) {
        Node* n = stack.takeLast();
        n->successor = nullptr;
        if (n->left) stack.append(n->left);
        if (n->right) stack.append(n->right);
    }
    buildThreadingHelper(m_root, nullptr);
}

/* ---- Threaded traversal ---- */

QVector<QPair<QPair<double, double>, double>> RedBlackTree10::threadedTraversal() const
{
    QVector<QPair<QPair<double, double>, double>> result;
    // Find leftmost
    Node* cur = m_root;
    while (cur && cur->left) cur = cur->left;

    while (cur) {
        result.append({{cur->lo, cur->hi}, cur->value});
        cur = cur->successor;
    }
    return result;
}

/* ---- Size / Empty ---- */

int RedBlackTree10::size() const
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

bool RedBlackTree10::isEmpty() const { return m_root == nullptr; }

/* ---- Compute height (used in stats) ---- */

namespace {
int computeHeightImpl(const RedBlackTree10::Node* n) {
    if (!n) return 0;
    return 1 + qMax(computeHeightImpl(n->left), computeHeightImpl(n->right));
}
}

/* ---- Reset ---- */

void RedBlackTree10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    delete m_root;
    m_root = nullptr;
}

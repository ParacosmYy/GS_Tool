/**
 * @file RedBlackTree16.cpp
 * @brief RedBlackTree16 实现
 *
 * 实现红黑树：左倾变体与迭代插入颜色翻转的简化平衡树维护。
 */

#include "utils/tree280/RedBlackTree16.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

RedBlackTree16::RedBlackTree16(QObject *parent)
    : QObject(parent)
{
    m_nil = new Node{};
    m_nil->color = BLACK;
    m_nil->left = m_nil;
    m_nil->right = m_nil;
    m_nil->parent = m_nil;
    m_root = m_nil;
}

RedBlackTree16::~RedBlackTree16()
{
    destroyTree(m_root);
    delete m_nil;
}

/* ---- Destroy tree recursively ---- */

void RedBlackTree16::destroyTree(Node* x)
{
    if (x == m_nil) return;
    destroyTree(x->left);
    destroyTree(x->right);
    delete x;
}

/* ---- Rotate left ---- */

void RedBlackTree16::rotateLeft(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    if (y->left != m_nil) y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == m_nil) m_root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
    m_stats.numRotations++;
}

/* ---- Rotate right ---- */

void RedBlackTree16::rotateRight(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    if (x->right != m_nil) x->right->parent = y;
    x->parent = y->parent;
    if (y->parent == m_nil) m_root = x;
    else if (y == y->parent->right) y->parent->right = x;
    else y->parent->left = x;
    x->right = y;
    y->parent = x;
    m_stats.numRotations++;
}

/* ---- Color flip (parent + two children) ---- */

void RedBlackTree16::colorFlip(Node* h)
{
    h->color = (h->color == RED) ? BLACK : RED;
    if (h->left != m_nil) h->left->color = (h->left->color == RED) ? BLACK : RED;
    if (h->right != m_nil) h->right->color = (h->right->color == RED) ? BLACK : RED;
    m_stats.numColorFlips++;
}

/* ---- Left-leaning fix: ensure red links lean left ---- */

void RedBlackTree16::fixLean(Node* node)
{
    // If right child is red and left child is black: rotate left
    if (isRed(node->right) && !isRed(node->left))
        rotateLeft(node);
    // If both children are red: color flip
    if (isRed(node->left) && isRed(node->right))
        colorFlip(node);
}

/* ---- Insert fixup (iterative) ---- */

void RedBlackTree16::insertFixup(Node* z)
{
    // Iterative fixup with color flips for left-leaning property
    while (z != m_root && isRed(z->parent)) {
        if (z->parent == z->parent->parent->left) {
            Node* uncle = z->parent->parent->right;
            if (isRed(uncle)) {
                // Case 1: uncle is red -> color flip (push black down)
                colorFlip(z->parent->parent);
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    // Case 2: triangle -> rotate to line
                    z = z->parent;
                    rotateLeft(z);
                }
                // Case 3: line -> rotate and recolor
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rotateRight(z->parent->parent);
            }
        } else {
            // Mirror: parent is right child
            Node* uncle = z->parent->parent->left;
            if (isRed(uncle)) {
                colorFlip(z->parent->parent);
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rotateRight(z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rotateLeft(z->parent->parent);
            }
        }
    }

    // Enforce left-leaning: fix any right-leaning red links
    Node* curr = m_root;
    while (curr != m_nil) {
        fixLean(curr);
        curr = curr->left;
    }

    m_root->color = BLACK;
}

/* ---- Insert (iterative) ---- */

void RedBlackTree16::insert(double key)
{
    QElapsedTimer timer;
    timer.start();

    // Iterative BST insert
    Node* z = new Node{key, RED, m_nil, m_nil, m_nil};
    Node* y = m_nil;
    Node* x = m_root;

    while (x != m_nil) {
        y = x;
        if (key < x->key) x = x->left;
        else if (key > x->key) x = x->right;
        else { delete z; return; }    // Duplicate
    }

    z->parent = y;
    if (y == m_nil) m_root = z;
    else if (key < y->key) y->left = z;
    else y->right = z;

    m_numNodes++;
    insertFixup(z);

    double elapsed = timer.elapsed();
    m_stats.numNodes = m_numNodes;
    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationDone(QStringLiteral("insert"), m_numNodes, m_stats.treeHeight, elapsed);
    emit rebalanceDone(m_stats.numRotations, m_stats.numColorFlips);
}

/* ---- Transplant ---- */

void RedBlackTree16::transplant(Node* u, Node* v)
{
    if (u->parent == m_nil) m_root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent;
}

/* ---- Tree minimum ---- */

RedBlackTree16::Node* RedBlackTree16::treeMinimum(Node* x) const
{
    while (x->left != m_nil) x = x->left;
    return x;
}

/* ---- Tree maximum ---- */

RedBlackTree16::Node* RedBlackTree16::treeMaximum(Node* x) const
{
    while (x->right != m_nil) x = x->right;
    return x;
}

/* ---- Delete fixup ---- */

void RedBlackTree16::deleteFixup(Node* x)
{
    while (x != m_root && !isRed(x)) {
        if (x == x->parent->left) {
            Node* w = x->parent->right;
            if (isRed(w)) {
                w->color = BLACK;
                x->parent->color = RED;
                rotateLeft(x->parent);
                w = x->parent->right;
            }
            if (!isRed(w->left) && !isRed(w->right)) {
                w->color = RED;
                x = x->parent;
            } else {
                if (!isRed(w->right)) {
                    w->left->color = BLACK;
                    w->color = RED;
                    rotateRight(w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                rotateLeft(x->parent);
                x = m_root;
            }
        } else {
            Node* w = x->parent->left;
            if (isRed(w)) {
                w->color = BLACK;
                x->parent->color = RED;
                rotateRight(x->parent);
                w = x->parent->left;
            }
            if (!isRed(w->right) && !isRed(w->left)) {
                w->color = RED;
                x = x->parent;
            } else {
                if (!isRed(w->left)) {
                    w->right->color = BLACK;
                    w->color = RED;
                    rotateLeft(w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                rotateRight(x->parent);
                x = m_root;
            }
        }
    }
    x->color = BLACK;
}

/* ---- Remove ---- */

void RedBlackTree16::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    Node* z = findNode(key);
    if (z == m_nil) return;

    Node* y = z;
    Node* x = m_nil;
    Color yOrigColor = y->color;

    if (z->left == m_nil) {
        x = z->right;
        transplant(z, z->right);
    } else if (z->right == m_nil) {
        x = z->left;
        transplant(z, z->left);
    } else {
        y = treeMinimum(z->right);
        yOrigColor = y->color;
        x = y->right;
        if (y->parent == z) {
            x->parent = y;
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

    delete z;
    m_numNodes--;

    if (yOrigColor == BLACK)
        deleteFixup(x);

    // Enforce left-leaning after delete
    Node* curr = m_root;
    while (curr != m_nil) {
        fixLean(curr);
        curr = curr->left;
    }

    double elapsed = timer.elapsed();
    m_stats.numNodes = m_numNodes;
    m_stats.treeHeight = height();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationDone(QStringLiteral("remove"), m_numNodes, m_stats.treeHeight, elapsed);
}

/* ---- Find node ---- */

RedBlackTree16::Node* RedBlackTree16::findNode(double key) const
{
    Node* x = m_root;
    while (x != m_nil) {
        if (key < x->key) x = x->left;
        else if (key > x->key) x = x->right;
        else return x;
    }
    return m_nil;
}

/* ---- Contains ---- */

bool RedBlackTree16::contains(double key) const { return findNode(key) != m_nil; }

/* ---- Min/Max ---- */

double RedBlackTree16::minimum() const
{
    if (m_root == m_nil) return 0.0;
    return treeMinimum(m_root)->key;
}

double RedBlackTree16::maximum() const
{
    if (m_root == m_nil) return 0.0;
    return treeMaximum(m_root)->key;
}

/* ---- Successor ---- */

double RedBlackTree16::successor(double key) const
{
    Node* x = m_root;
    Node* succ = m_nil;
    while (x != m_nil) {
        if (key < x->key) { succ = x; x = x->left; }
        else x = x->right;
    }
    return (succ != m_nil) ? succ->key : key;
}

/* ---- Predecessor ---- */

double RedBlackTree16::predecessor(double key) const
{
    Node* x = m_root;
    Node* pred = m_nil;
    while (x != m_nil) {
        if (key > x->key) { pred = x; x = x->right; }
        else x = x->left;
    }
    return (pred != m_nil) ? pred->key : key;
}

/* ---- In-order traversal ---- */

void RedBlackTree16::inOrderRec(Node* x, QVector<double>& result) const
{
    if (x == m_nil) return;
    inOrderRec(x->left, result);
    result.append(x->key);
    inOrderRec(x->right, result);
}

QVector<double> RedBlackTree16::inOrder() const
{
    QVector<double> result;
    inOrderRec(m_root, result);
    return result;
}

/* ---- Height ---- */

int RedBlackTree16::heightRec(Node* x) const
{
    if (x == m_nil) return 0;
    return 1 + qMax(heightRec(x->left), heightRec(x->right));
}

int RedBlackTree16::height() const { return heightRec(m_root); }

/* ---- Size ---- */

int RedBlackTree16::size() const { return m_numNodes; }

/* ---- Reset ---- */

void RedBlackTree16::resetStatistics()
{
    destroyTree(m_root);
    m_root = m_nil;
    m_numNodes = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

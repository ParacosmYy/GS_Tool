/**
 * @file RedBlackTree15.cpp
 * @brief RedBlackTree15 实现
 *
 * 实现红黑树：自底向上插入修复与自顶向下中序后继提升删除平衡BST。
 */

#include "utils/tree266/RedBlackTree15.h"

#include <QElapsedTimer>
#include <QtGlobal>
#include <algorithm>
#include <climits>

/* ---- Construction / Destruction ---- */

RedBlackTree15::RedBlackTree15(QObject *parent)
    : QObject(parent)
{
    m_nil = new Node();
    m_nil->color = Black;
    m_nil->left = m_nil->right = m_nil->parent = m_nil;
    m_root = m_nil;
}

RedBlackTree15::~RedBlackTree15()
{
    destroyTree(m_root);
    delete m_nil;
}

void RedBlackTree15::destroyTree(Node* node)
{
    if (node == m_nil) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}

/* ---- Rotations ---- */

void RedBlackTree15::rotateLeft(Node* x)
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

void RedBlackTree15::rotateRight(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    if (x->right != m_nil) x->right->parent = y;
    x->parent = y->parent;
    if (y->parent == m_nil) m_root = x;
    else if (y == y->parent->left) y->parent->left = x;
    else y->parent->right = x;
    x->right = y;
    y->parent = x;
    m_stats.numRotations++;
}

/* ---- Insert ---- */

void RedBlackTree15::insert(int key)
{
    QElapsedTimer timer;
    timer.start();

    // Bottom-up insert
    Node* z = new Node();
    z->key = key;
    z->left = m_nil;
    z->right = m_nil;
    z->color = Red;

    Node* y = m_nil;
    Node* x = m_root;
    while (x != m_nil) {
        y = x;
        if (z->key < x->key) x = x->left;
        else x = x->right;
    }
    z->parent = y;
    if (y == m_nil) m_root = z;
    else if (z->key < y->key) y->left = z;
    else y->right = z;

    insertFixup(z);
    m_size++;

    double elapsed = timer.elapsed();
    m_stats.numElements = m_size;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_size, m_stats.treeHeight, m_stats.numRotations, elapsed);
}

/* ---- Bottom-up insertion fixup ---- */

void RedBlackTree15::insertFixup(Node* z)
{
    while (z->parent->color == Red) {
        if (z->parent == z->parent->parent->left) {
            Node* y = z->parent->parent->right;  // Uncle
            if (y->color == Red) {
                // Case 1: Red uncle -> recolor
                z->parent->color = Black;
                y->color = Black;
                z->parent->parent->color = Red;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    // Case 2: Left-rotate to set up case 3
                    z = z->parent;
                    rotateLeft(z);
                }
                // Case 3: Recolor and right-rotate
                z->parent->color = Black;
                z->parent->parent->color = Red;
                rotateRight(z->parent->parent);
            }
        } else {
            // Symmetric cases
            Node* y = z->parent->parent->left;
            if (y->color == Red) {
                z->parent->color = Black;
                y->color = Black;
                z->parent->parent->color = Red;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rotateRight(z);
                }
                z->parent->color = Black;
                z->parent->parent->color = Red;
                rotateLeft(z->parent->parent);
            }
        }
    }
    m_root->color = Black;
}

/* ---- Transplant ---- */

void RedBlackTree15::transplant(Node* u, Node* v)
{
    if (u->parent == m_nil) m_root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent;
}

/* ---- Tree minimum/maximum ---- */

RedBlackTree15::Node* RedBlackTree15::treeMinimum(Node* node) const
{
    while (node->left != m_nil) node = node->left;
    return node;
}

RedBlackTree15::Node* RedBlackTree15::treeMaximum(Node* node) const
{
    while (node->right != m_nil) node = node->right;
    return node;
}

/* ---- Top-down deletion with in-order successor promotion ---- */

void RedBlackTree15::deleteNode(Node* z)
{
    Node* y = z;
    Node* x = nullptr;
    Color yOrigColor = y->color;

    if (z->left == m_nil) {
        // No left child: promote right
        x = z->right;
        transplant(z, z->right);
    } else if (z->right == m_nil) {
        // No right child: promote left
        x = z->left;
        transplant(z, z->left);
    } else {
        // Two children: find in-order successor
        y = treeMinimum(z->right);
        yOrigColor = y->color;
        x = y->right;

        if (y->parent == z) {
            // Successor is direct right child
            x->parent = y;
        } else {
            // Transplant successor with its right child
            transplant(y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        transplant(z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    if (yOrigColor == Black)
        deleteFixup(x);

    delete z;
    m_size--;
}

/* ---- Delete fixup ---- */

void RedBlackTree15::deleteFixup(Node* x)
{
    while (x != m_root && x->color == Black) {
        if (x == x->parent->left) {
            Node* w = x->parent->right;
            if (w->color == Red) {
                w->color = Black;
                x->parent->color = Red;
                rotateLeft(x->parent);
                w = x->parent->right;
            }
            if (w->left->color == Black && w->right->color == Black) {
                w->color = Red;
                x = x->parent;
            } else {
                if (w->right->color == Black) {
                    w->left->color = Black;
                    w->color = Red;
                    rotateRight(w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = Black;
                w->right->color = Black;
                rotateLeft(x->parent);
                x = m_root;
            }
        } else {
            Node* w = x->parent->left;
            if (w->color == Red) {
                w->color = Black;
                x->parent->color = Red;
                rotateRight(x->parent);
                w = x->parent->left;
            }
            if (w->right->color == Black && w->left->color == Black) {
                w->color = Red;
                x = x->parent;
            } else {
                if (w->left->color == Black) {
                    w->right->color = Black;
                    w->color = Red;
                    rotateLeft(w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = Black;
                w->left->color = Black;
                rotateRight(x->parent);
                x = m_root;
            }
        }
    }
    x->color = Black;
}

/* ---- Remove ---- */

void RedBlackTree15::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    Node* z = search(key);
    if (z != m_nil) {
        deleteNode(z);
    }

    double elapsed = timer.elapsed();
    m_stats.numElements = m_size;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit treeUpdated(m_size, m_stats.treeHeight, m_stats.numRotations, elapsed);
}

/* ---- Search ---- */

RedBlackTree15::Node* RedBlackTree15::search(int key) const
{
    Node* x = m_root;
    while (x != m_nil) {
        if (key == x->key) return x;
        x = (key < x->key) ? x->left : x->right;
    }
    return m_nil;
}

bool RedBlackTree15::contains(int key) const
{
    return search(key) != m_nil;
}

/* ---- Min/Max ---- */

int RedBlackTree15::minimum() const
{
    if (m_root == m_nil) return INT_MIN;
    return treeMinimum(m_root)->key;
}

int RedBlackTree15::maximum() const
{
    if (m_root == m_nil) return INT_MAX;
    return treeMaximum(m_root)->key;
}

/* ---- Successor/Predecessor ---- */

int RedBlackTree15::successor(int key) const
{
    Node* x = m_root;
    Node* succ = m_nil;
    while (x != m_nil) {
        if (key < x->key) {
            succ = x;
            x = x->left;
        } else {
            x = x->right;
        }
    }
    return (succ != m_nil) ? succ->key : INT_MAX;
}

int RedBlackTree15::predecessor(int key) const
{
    Node* x = m_root;
    Node* pred = m_nil;
    while (x != m_nil) {
        if (key > x->key) {
            pred = x;
            x = x->right;
        } else {
            x = x->left;
        }
    }
    return (pred != m_nil) ? pred->key : INT_MIN;
}

/* ---- In-order traversal ---- */

void RedBlackTree15::inOrderCollect(Node* node, QVector<int>& keys) const
{
    if (node == m_nil) return;
    inOrderCollect(node->left, keys);
    keys.append(node->key);
    inOrderCollect(node->right, keys);
}

QVector<int> RedBlackTree15::inOrderKeys() const
{
    QVector<int> keys;
    inOrderCollect(m_root, keys);
    return keys;
}

/* ---- Height ---- */

int RedBlackTree15::computeHeight(Node* node) const
{
    if (node == m_nil) return 0;
    return 1 + qMax(computeHeight(node->left), computeHeight(node->right));
}

int RedBlackTree15::size() const { return m_size; }

/* ---- Reset ---- */

void RedBlackTree15::resetStatistics()
{
    destroyTree(m_root);
    m_root = m_nil;
    m_size = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

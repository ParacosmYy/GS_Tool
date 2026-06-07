/**
 * @file RedBlackTree11.cpp
 * @brief RedBlackTree11 实现
 *
 * 实现红黑树：顺序统计量增强、秩查询(rank/select)、有序敏感范围删除。
 */

#include "utils/tree210/RedBlackTree11.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

RedBlackTree11::RedBlackTree11(QObject *parent) : QObject(parent)
{
    m_nil = new Node(0);
    m_nil->color = Black;
    m_nil->size = 0;
    m_nil->left = m_nil->right = m_nil->parent = m_nil;
    m_root = m_nil;
}

RedBlackTree11::~RedBlackTree11()
{
    deleteTree(m_root);
    delete m_nil;
}

/* ---- Delete tree ---- */

void RedBlackTree11::deleteTree(Node *x)
{
    if (x == m_nil) return;
    deleteTree(x->left);
    deleteTree(x->right);
    delete x;
}

/* ---- Update sizes ---- */

void RedBlackTree11::updateSizes(Node *x)
{
    while (x != m_nil) {
        x->size = x->left->size + x->right->size + 1;
        x = x->parent;
    }
}

/* ---- Left rotation ---- */

void RedBlackTree11::rotateLeft(Node *x)
{
    Node *y = x->right;
    x->right = y->left;
    if (y->left != m_nil) y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == m_nil) m_root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;

    // Update sizes
    x->size = x->left->size + x->right->size + 1;
    y->size = y->left->size + y->right->size + 1;

    m_stats.totalRotations++;
}

/* ---- Right rotation ---- */

void RedBlackTree11::rotateRight(Node *x)
{
    Node *y = x->left;
    x->left = y->right;
    if (y->right != m_nil) y->right->parent = x;
    y->parent = x->parent;
    if (x->parent == m_nil) m_root = y;
    else if (x == x->parent->right) x->parent->right = y;
    else x->parent->left = y;
    y->right = x;
    x->parent = y;

    x->size = x->left->size + x->right->size + 1;
    y->size = y->left->size + y->right->size + 1;

    m_stats.totalRotations++;
}

/* ---- Insert fixup ---- */

void RedBlackTree11::insertFixup(Node *z)
{
    while (z->parent->color == Red) {
        if (z->parent == z->parent->parent->left) {
            Node *y = z->parent->parent->right;
            if (y->color == Red) {
                z->parent->color = Black;
                y->color = Black;
                z->parent->parent->color = Red;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    rotateLeft(z);
                }
                z->parent->color = Black;
                z->parent->parent->color = Red;
                rotateRight(z->parent->parent);
            }
        } else {
            Node *y = z->parent->parent->left;
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

/* ---- Insert ---- */

void RedBlackTree11::insert(int key)
{
    QElapsedTimer timer;
    timer.start();

    Node *z = new Node(key);
    z->left = z->right = z->parent = m_nil;

    Node *y = m_nil;
    Node *x = m_root;

    while (x != m_nil) {
        y = x;
        if (z->key < x->key) x = x->left;
        else x = x->right;
    }

    z->parent = y;
    if (y == m_nil) m_root = z;
    else if (z->key < y->key) y->left = z;
    else y->right = z;

    updateSizes(z);
    insertFixup(z);

    m_stats.numNodes++;
    m_stats.totalOps++;
    m_stats.treeHeight = height();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("insert", key, timer.elapsed());
}

/* ---- Search ---- */

RedBlackTree11::Node* RedBlackTree11::search(int key) const
{
    Node *x = m_root;
    while (x != m_nil) {
        if (key == x->key) return x;
        x = (key < x->key) ? x->left : x->right;
    }
    return m_nil;
}

bool RedBlackTree11::contains(int key) const
{
    return search(key) != m_nil;
}

/* ---- Minimum / Maximum ---- */

RedBlackTree11::Node* RedBlackTree11::minimum(Node *x) const
{
    while (x->left != m_nil) x = x->left;
    return x;
}

RedBlackTree11::Node* RedBlackTree11::maximum(Node *x) const
{
    while (x->right != m_nil) x = x->right;
    return x;
}

/* ---- Transplant ---- */

void RedBlackTree11::transplant(Node *u, Node *v)
{
    if (u->parent == m_nil) m_root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent;
}

/* ---- Delete fixup ---- */

void RedBlackTree11::deleteFixup(Node *x)
{
    while (x != m_root && x->color == Black) {
        if (x == x->parent->left) {
            Node *w = x->parent->right;
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
            Node *w = x->parent->left;
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

void RedBlackTree11::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    Node *z = search(key);
    if (z == m_nil) return;

    Node *y = z;
    Color yOrigColor = y->color;
    Node *x = m_nil;

    if (z->left == m_nil) {
        x = z->right;
        transplant(z, z->right);
    } else if (z->right == m_nil) {
        x = z->left;
        transplant(z, z->left);
    } else {
        y = minimum(z->right);
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

    updateSizes(x->parent);
    delete z;

    if (yOrigColor == Black) deleteFixup(x);

    m_stats.numNodes--;
    m_stats.totalOps++;
    m_stats.treeHeight = height();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("remove", key, timer.elapsed());
}

/* ---- Rank (1-based) ---- */

int RedBlackTree11::rank(int key) const
{
    int r = 0;
    Node *x = m_root;
    while (x != m_nil) {
        if (key < x->key) {
            x = x->left;
        } else if (key > x->key) {
            r += x->left->size + 1;
            x = x->right;
        } else {
            return r + x->left->size + 1;
        }
    }
    return 0; // Key not found
}

/* ---- Select (k-th smallest, 1-based) ---- */

int RedBlackTree11::select(int k) const
{
    if (k < 1 || k > m_root->size) return -1;

    Node *x = m_root;
    while (x != m_nil) {
        int leftSize = x->left->size;
        if (k <= leftSize) {
            x = x->left;
        } else if (k == leftSize + 1) {
            return x->key;
        } else {
            k -= leftSize + 1;
            x = x->right;
        }
    }
    return -1;
}

/* ---- Inorder ---- */

void RedBlackTree11::inorderHelper(Node *x, QVector<int>& result) const
{
    if (x == m_nil) return;
    inorderHelper(x->left, result);
    result.append(x->key);
    inorderHelper(x->right, result);
}

QVector<int> RedBlackTree11::inorder() const
{
    QVector<int> result;
    inorderHelper(m_root, result);
    return result;
}

/* ---- Range query ---- */

void RedBlackTree11::rangeQueryHelper(Node *x, int lo, int hi,
                                        QVector<int>& result) const
{
    if (x == m_nil) return;
    if (lo < x->key) rangeQueryHelper(x->left, lo, hi, result);
    if (lo <= x->key && x->key <= hi) result.append(x->key);
    if (hi > x->key) rangeQueryHelper(x->right, lo, hi, result);
}

QVector<int> RedBlackTree11::rangeQuery(int lo, int hi) const
{
    QVector<int> result;
    rangeQueryHelper(m_root, lo, hi, result);
    return result;
}

/* ---- Range deletion ---- */

void RedBlackTree11::rangeDelete(int lo, int hi)
{
    QElapsedTimer timer;
    timer.start();

    // Collect keys in range, then delete one by one
    QVector<int> keys = rangeQuery(lo, hi);
    for (int key : keys) remove(key);

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Size / Height ---- */

int RedBlackTree11::size() const { return m_root->size; }

int RedBlackTree11::heightHelper(Node *x) const
{
    if (x == m_nil) return 0;
    return 1 + qMax(heightHelper(x->left), heightHelper(x->right));
}

int RedBlackTree11::height() const { return heightHelper(m_root); }

/* ---- Clear ---- */

void RedBlackTree11::clear()
{
    deleteTree(m_root);
    m_root = m_nil;
    m_stats.numNodes = 0;
    m_stats.treeHeight = 0;
}

/* ---- Reset ---- */

void RedBlackTree11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    clear();
}

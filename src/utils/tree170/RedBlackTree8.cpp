/**
 * @file RedBlackTree8.cpp
 * @brief RedBlackTree8 实现
 *
 * 实现红黑树：插入修复、删除修复、旋转、顺序统计、范围查询。
 */

#include "utils/tree170/RedBlackTree8.h"

#include <QElapsedTimer>
#include <algorithm>

RedBlackTree8::RedBlackTree8(QObject *parent)
    : QObject(parent)
{
    /* Allocate sentinel nil node */
    m_nil = new Node();
    m_nil->color = Black;
    m_nil->subtreeSize = 0;
    m_nil->left = m_nil->right = m_nil->parent = m_nil;
    m_root = m_nil;
}

RedBlackTree8::~RedBlackTree8()
{
    clearRec(m_root);
    delete m_nil;
}

int RedBlackTree8::getSize(Node* x) { return (x == nullptr) ? 0 : x->subtreeSize; }

void RedBlackTree8::updateSize(Node* x)
{
    while (x != m_nil) {
        x->subtreeSize = 1 + getSize(x->left) + getSize(x->right);
        x = x->parent;
    }
}

void RedBlackTree8::rotateLeft(Node* x)
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
    y->subtreeSize = x->subtreeSize;
    x->subtreeSize = 1 + getSize(x->left) + getSize(x->right);
    m_stats.totalRotations++;
}

void RedBlackTree8::rotateRight(Node* y)
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
    x->subtreeSize = y->subtreeSize;
    y->subtreeSize = 1 + getSize(y->left) + getSize(y->right);
    m_stats.totalRotations++;
}

void RedBlackTree8::insertFixup(Node* z)
{
    while (z->parent->color == Red) {
        if (z->parent == z->parent->parent->left) {
            Node* y = z->parent->parent->right;
            if (y->color == Red) {
                /* Case 1: uncle is red */
                z->parent->color = Black;
                y->color = Black;
                z->parent->parent->color = Red;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    /* Case 2: uncle is black, z is right child */
                    z = z->parent;
                    rotateLeft(z);
                }
                /* Case 3: uncle is black, z is left child */
                z->parent->color = Black;
                z->parent->parent->color = Red;
                rotateRight(z->parent->parent);
            }
        } else {
            /* Mirror: parent is right child */
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

bool RedBlackTree8::insert(double key, double value)
{
    QElapsedTimer timer;
    timer.start();

    /* Find insertion point */
    Node* y = m_nil;
    Node* x = m_root;
    while (x != m_nil) {
        y = x;
        if (key < x->key) x = x->left;
        else if (key > x->key) x = x->right;
        else return false; /* Duplicate */
    }

    /* Create new node */
    Node* z = new Node(key, value);
    z->parent = y;
    z->left = m_nil;
    z->right = m_nil;
    z->color = Red;
    z->subtreeSize = 1;

    if (y == m_nil) m_root = z;
    else if (key < y->key) y->left = z;
    else y->right = z;

    updateSize(z->parent);
    insertFixup(z);
    /* Fix sizes after rotations */
    updateSize(z);

    m_stats.currentSize++;
    m_stats.totalInserts++;
    m_stats.treeHeight = computeHeight(m_root);
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalInserts + m_stats.totalDeletes;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit insertCompleted(key, m_stats.currentSize);
    emit rotationPerformed(0);
    return true;
}

void RedBlackTree8::transplant(Node* u, Node* v)
{
    if (u->parent == m_nil) m_root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent;
}

RedBlackTree8::Node* RedBlackTree8::minimum(Node* x) const
{
    while (x->left != m_nil) x = x->left;
    return x;
}

void RedBlackTree8::deleteFixup(Node* x, Node* xParent)
{
    while (x != m_root && (x == m_nil || x->color == Black)) {
        if (x == xParent->left) {
            Node* w = xParent->right;
            if (w->color == Red) {
                w->color = Black;
                xParent->color = Red;
                rotateLeft(xParent);
                w = xParent->right;
            }
            if ((w->left == m_nil || w->left->color == Black) &&
                (w->right == m_nil || w->right->color == Black)) {
                w->color = Red;
                x = xParent;
                xParent = x->parent;
            } else {
                if (w->right == m_nil || w->right->color == Black) {
                    if (w->left != m_nil) w->left->color = Black;
                    w->color = Red;
                    rotateRight(w);
                    w = xParent->right;
                }
                w->color = xParent->color;
                xParent->color = Black;
                if (w->right != m_nil) w->right->color = Black;
                rotateLeft(xParent);
                x = m_root;
                xParent = m_nil;
            }
        } else {
            Node* w = xParent->left;
            if (w->color == Red) {
                w->color = Black;
                xParent->color = Red;
                rotateRight(xParent);
                w = xParent->left;
            }
            if ((w->right == m_nil || w->right->color == Black) &&
                (w->left == m_nil || w->left->color == Black)) {
                w->color = Red;
                x = xParent;
                xParent = x->parent;
            } else {
                if (w->left == m_nil || w->left->color == Black) {
                    if (w->right != m_nil) w->right->color = Black;
                    w->color = Red;
                    rotateLeft(w);
                    w = xParent->left;
                }
                w->color = xParent->color;
                xParent->color = Black;
                if (w->left != m_nil) w->left->color = Black;
                rotateRight(xParent);
                x = m_root;
                xParent = m_nil;
            }
        }
    }
    if (x != m_nil) x->color = Black;
}

bool RedBlackTree8::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    /* Find node */
    Node* z = m_root;
    while (z != m_nil) {
        if (key < z->key) z = z->left;
        else if (key > z->key) z = z->right;
        else break;
    }
    if (z == m_nil) return false;

    Node* y = z;
    Color yOrigColor = y->color;
    Node* x;
    Node* xParent;

    if (z->left == m_nil) {
        x = z->right;
        xParent = z->parent;
        transplant(z, z->right);
    } else if (z->right == m_nil) {
        x = z->left;
        xParent = z->parent;
        transplant(z, z->left);
    } else {
        y = minimum(z->right);
        yOrigColor = y->color;
        x = y->right;
        xParent = y;
        if (y->parent == z) {
            xParent = y;
        } else {
            transplant(y, y->right);
            xParent = y->parent;
            y->right = z->right;
            y->right->parent = y;
        }
        transplant(z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    updateSize(xParent);
    delete z;

    if (yOrigColor == Black) deleteFixup(x, xParent);

    m_stats.currentSize--;
    m_stats.totalDeletes++;
    m_stats.treeHeight = computeHeight(m_root);
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalInserts + m_stats.totalDeletes;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit deleteCompleted(key);
    return true;
}

bool RedBlackTree8::find(double key, double& value) const
{
    Node* x = m_root;
    while (x != m_nil) {
        if (key < x->key) x = x->left;
        else if (key > x->key) x = x->right;
        else { value = x->value; return true; }
    }
    return false;
}

int RedBlackTree8::rank(double key) const
{
    int r = 1;
    Node* x = m_root;
    while (x != m_nil) {
        if (key < x->key) {
            x = x->left;
        } else if (key > x->key) {
            r += getSize(x->left) + 1;
            x = x->right;
        } else {
            return r + getSize(x->left);
        }
    }
    return -1; /* Not found */
}

bool RedBlackTree8::select(int k, double& key, double& value) const
{
    if (k < 1 || k > m_stats.currentSize) return false;
    Node* x = m_root;
    while (x != m_nil) {
        int leftSize = getSize(x->left);
        if (k <= leftSize) {
            x = x->left;
        } else if (k == leftSize + 1) {
            key = x->key;
            value = x->value;
            return true;
        } else {
            k -= leftSize + 1;
            x = x->right;
        }
    }
    return false;
}

void RedBlackTree8::rangeQueryRec(Node* x, double lo, double hi,
                                   QVector<QPair<double, double>>& result) const
{
    if (x == m_nil) return;
    if (lo < x->key) rangeQueryRec(x->left, lo, hi, result);
    if (lo <= x->key && x->key <= hi)
        result.append({x->key, x->value});
    if (hi > x->key) rangeQueryRec(x->right, lo, hi, result);
}

QVector<QPair<double, double>> RedBlackTree8::rangeQuery(double lo, double hi) const
{
    QVector<QPair<double, double>> result;
    rangeQueryRec(m_root, lo, hi, result);
    return result;
}

void RedBlackTree8::inOrderRec(Node* x, QVector<QPair<double, double>>& result) const
{
    if (x == m_nil) return;
    inOrderRec(x->left, result);
    result.append({x->key, x->value});
    inOrderRec(x->right, result);
}

QVector<QPair<double, double>> RedBlackTree8::inOrder() const
{
    QVector<QPair<double, double>> result;
    inOrderRec(m_root, result);
    return result;
}

bool RedBlackTree8::isEmpty() const { return m_root == m_nil; }

void RedBlackTree8::clearRec(Node* x)
{
    if (x == m_nil || x == nullptr) return;
    clearRec(x->left);
    clearRec(x->right);
    delete x;
}

void RedBlackTree8::clear()
{
    clearRec(m_root);
    m_root = m_nil;
    m_stats.currentSize = 0;
    m_stats.treeHeight = 0;
}

int RedBlackTree8::computeHeight(Node* x)
{
    if (x == nullptr || x == m_nil->parent) return 0; // Can't check m_nil directly in static
    int h = 0;
    while (x != nullptr && x->left != nullptr) {
        h++;
        x = (x->left) ? x->left : x->right;
    }
    return h;
}

void RedBlackTree8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

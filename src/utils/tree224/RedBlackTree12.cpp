/**
 * @file RedBlackTree12.cpp
 * @brief RedBlackTree12 实现
 *
 * 实现红黑树：增强子树求和、逆秩查询、百分位访问。
 */

#include "utils/tree224/RedBlackTree12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

RedBlackTree12::RedBlackTree12(QObject *parent) : QObject(parent)
{
    m_nil = new RBNode();
    m_nil->color = BLACK;
    m_nil->subtreeSize = 0;
    m_nil->subtreeSum = 0.0;
    m_nil->left = m_nil->right = m_nil->parent = m_nil;
    m_root = m_nil;
}

RedBlackTree12::~RedBlackTree12()
{
    deleteTree(m_root);
    delete m_nil;
}

void RedBlackTree12::deleteTree(RBNode* node)
{
    if (node == m_nil) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

/* ---- Allocate node ---- */

RedBlackTree12::RBNode* RedBlackTree12::allocNode(double key, double value)
{
    RBNode* n = new RBNode();
    n->key = key;
    n->value = value;
    n->subtreeSize = 1;
    n->subtreeSum = value;
    n->color = RED;
    n->left = m_nil;
    n->right = m_nil;
    n->parent = m_nil;
    return n;
}

/* ---- Update augmented data ---- */

void RedBlackTree12::updateAugmented(RBNode* node)
{
    while (node != m_nil) {
        node->subtreeSize = 1 + node->left->subtreeSize + node->right->subtreeSize;
        node->subtreeSum = node->value + node->left->subtreeSum + node->right->subtreeSum;
        node = node->parent;
    }
}

/* ---- Rotations ---- */

void RedBlackTree12::rotateLeft(RBNode* x)
{
    RBNode* y = x->right;
    x->right = y->left;
    if (y->left != m_nil) y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == m_nil) m_root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
    // Update augmented data
    x->subtreeSize = 1 + x->left->subtreeSize + x->right->subtreeSize;
    x->subtreeSum = x->value + x->left->subtreeSum + x->right->subtreeSum;
    y->subtreeSize = 1 + y->left->subtreeSize + y->right->subtreeSize;
    y->subtreeSum = y->value + y->left->subtreeSum + y->right->subtreeSum;
    m_stats.numRotations++;
}

void RedBlackTree12::rotateRight(RBNode* y)
{
    RBNode* x = y->left;
    y->left = x->right;
    if (x->right != m_nil) x->right->parent = y;
    x->parent = y->parent;
    if (y->parent == m_nil) m_root = x;
    else if (y == y->parent->right) y->parent->right = x;
    else y->parent->left = x;
    x->right = y;
    y->parent = x;
    y->subtreeSize = 1 + y->left->subtreeSize + y->right->subtreeSize;
    y->subtreeSum = y->value + y->left->subtreeSum + y->right->subtreeSum;
    x->subtreeSize = 1 + x->left->subtreeSize + x->right->subtreeSize;
    x->subtreeSum = x->value + x->left->subtreeSum + x->right->subtreeSum;
    m_stats.numRotations++;
}

/* ---- Insert fixup ---- */

void RedBlackTree12::insertFixup(RBNode* z)
{
    while (z->parent->color == RED) {
        if (z->parent == z->parent->parent->left) {
            RBNode* y = z->parent->parent->right;
            if (y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    rotateLeft(z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rotateRight(z->parent->parent);
            }
        } else {
            RBNode* y = z->parent->parent->left;
            if (y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
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
    m_root->color = BLACK;
}

/* ---- Insert ---- */

void RedBlackTree12::insert(double key, double value)
{
    QElapsedTimer timer;
    timer.start();

    RBNode* z = allocNode(key, value);
    RBNode* y = m_nil;
    RBNode* x = m_root;

    while (x != m_nil) {
        y = x;
        if (key < x->key) x = x->left;
        else if (key > x->key) x = x->right;
        else { x->value = value; updateAugmented(x); delete z; return; } // Update
    }

    z->parent = y;
    if (y == m_nil) m_root = z;
    else if (key < y->key) y->left = z;
    else y->right = z;

    updateAugmented(z);
    insertFixup(z);

    m_stats.numNodes++;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("insert", m_stats.numNodes, timer.elapsed());
}

/* ---- Transplant ---- */

void RedBlackTree12::transplant(RBNode* u, RBNode* v)
{
    if (u->parent == m_nil) m_root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent;
}

/* ---- Tree minimum ---- */

RedBlackTree12::RBNode* RedBlackTree12::treeMinimum(RBNode* node) const
{
    while (node->left != m_nil) node = node->left;
    return node;
}

/* ---- Remove fixup ---- */

void RedBlackTree12::removeFixup(RBNode* x)
{
    while (x != m_root && x->color == BLACK) {
        if (x == x->parent->left) {
            RBNode* w = x->parent->right;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rotateLeft(x->parent);
                w = x->parent->right;
            }
            if (w->left->color == BLACK && w->right->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->right->color == BLACK) {
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
            RBNode* w = x->parent->left;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rotateRight(x->parent);
                w = x->parent->left;
            }
            if (w->right->color == BLACK && w->left->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->left->color == BLACK) {
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

bool RedBlackTree12::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    RBNode* z = findNode(key);
    if (z == m_nil) return false;

    RBNode* y = z;
    Color yOrigColor = y->color;
    RBNode* x;

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

    updateAugmented(x->parent);
    delete z;

    if (yOrigColor == BLACK) removeFixup(x);

    m_stats.numNodes--;
    m_stats.treeHeight = computeHeight(m_root);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("remove", m_stats.numNodes, timer.elapsed());
    return true;
}

/* ---- Find node ---- */

RedBlackTree12::RBNode* RedBlackTree12::findNode(double key) const
{
    RBNode* x = m_root;
    while (x != m_nil) {
        if (key < x->key) x = x->left;
        else if (key > x->key) x = x->right;
        else return x;
    }
    return m_nil;
}

/* ---- Contains ---- */

bool RedBlackTree12::contains(double key) const { return findNode(key) != m_nil; }

/* ---- Select k-th smallest ---- */

double RedBlackTree12::select(int k) const
{
    if (k < 0 || k >= m_root->subtreeSize) return 0.0;
    RBNode* x = m_root;
    while (x != m_nil) {
        int leftSize = x->left->subtreeSize;
        if (k < leftSize) { x = x->left; }
        else if (k == leftSize) { return x->key; }
        else { k -= leftSize + 1; x = x->right; }
    }
    return 0.0;
}

/* ---- Percentile ---- */

double RedBlackTree12::percentile(double p) const
{
    if (m_root == m_nil) return 0.0;
    p = qBound(0.0, p, 1.0);
    int total = m_root->subtreeSize;
    int k = static_cast<int>(p * (total - 1));
    return select(k);
}

/* ---- Rank ---- */

int RedBlackTree12::rank(double key) const
{
    int r = 0;
    RBNode* x = m_root;
    while (x != m_nil) {
        if (key < x->key) { x = x->left; }
        else if (key > x->key) {
            r += x->left->subtreeSize + 1;
            x = x->right;
        } else { return r + x->left->subtreeSize; }
    }
    return r;
}

/* ---- Total sum ---- */

double RedBlackTree12::totalSum() const { return m_root->subtreeSum; }

/* ---- Prefix sum ---- */

double RedBlackTree12::prefixSum(double key) const
{
    double sum = 0.0;
    RBNode* x = m_root;
    while (x != m_nil) {
        if (key < x->key) { x = x->left; }
        else if (key > x->key) {
            sum += x->left->subtreeSum + x->value;
            x = x->right;
        } else { return sum + x->left->subtreeSum + x->value; }
    }
    return sum;
}

/* ---- Collect keys ---- */

void RedBlackTree12::collectKeys(RBNode* node, QVector<double>& result) const
{
    if (node == m_nil) return;
    collectKeys(node->left, result);
    result.append(node->key);
    collectKeys(node->right, result);
}

QVector<double> RedBlackTree12::keys() const
{
    QVector<double> result;
    collectKeys(m_root, result);
    return result;
}

/* ---- Size ---- */

int RedBlackTree12::size() const { return m_root->subtreeSize; }

/* ---- Compute height ---- */

int RedBlackTree12::computeHeight(RBNode* node) const
{
    if (node == m_nil) return 0;
    return 1 + qMax(computeHeight(node->left), computeHeight(node->right));
}

/* ---- Reset ---- */

void RedBlackTree12::resetStatistics()
{
    deleteTree(m_root);
    m_root = m_nil;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

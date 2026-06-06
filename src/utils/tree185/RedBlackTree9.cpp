/**
 * @file RedBlackTree9.cpp
 * @brief RedBlackTree9 实现
 *
 * 实现红黑树：迭代插入/删除修复、范围查询、增强子树大小、顺序统计。
 */

#include "utils/tree185/RedBlackTree9.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

RedBlackTree9::RedBlackTree9(QObject *parent) : QObject(parent)
{
    m_nil = new Node{};
    m_nil->color = Color::Black;
    m_nil->subtreeSize = 0;
    m_nil->left = m_nil->right = m_nil->parent = m_nil;
    m_root = m_nil;
}

RedBlackTree9::~RedBlackTree9()
{
    deleteTree(m_root);
    delete m_nil;
}

void RedBlackTree9::deleteTree(Node* n)
{
    if (n == m_nil) return;
    deleteTree(n->left);
    deleteTree(n->right);
    delete n;
}

/* ---- Size update ---- */

void RedBlackTree9::updateSize(Node* n)
{
    while (n != m_nil) {
        n->subtreeSize = 1 + n->left->subtreeSize + n->right->subtreeSize;
        n = n->parent;
    }
}

/* ---- Rotations ---- */

void RedBlackTree9::rotateLeft(Node* x)
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

    // Update sizes
    x->subtreeSize = 1 + x->left->subtreeSize + x->right->subtreeSize;
    y->subtreeSize = 1 + y->left->subtreeSize + y->right->subtreeSize;

    m_stats.numRotations++;
}

void RedBlackTree9::rotateRight(Node* x)
{
    Node* y = x->left;
    x->left = y->right;
    if (y->right != m_nil) y->right->parent = x;
    y->parent = x->parent;
    if (x->parent == m_nil) m_root = y;
    else if (x == x->parent->right) x->parent->right = y;
    else x->parent->left = y;
    y->right = x;
    x->parent = y;

    x->subtreeSize = 1 + x->left->subtreeSize + x->right->subtreeSize;
    y->subtreeSize = 1 + y->left->subtreeSize + y->right->subtreeSize;

    m_stats.numRotations++;
}

/* ---- Insert fixup (iterative) ---- */

void RedBlackTree9::insertFixup(Node* z)
{
    while (z->parent->color == Color::Red) {
        if (z->parent == z->parent->parent->left) {
            Node* y = z->parent->parent->right; // Uncle
            if (y->color == Color::Red) {
                // Case 1: Red uncle
                z->parent->color = Color::Black;
                y->color = Color::Black;
                z->parent->parent->color = Color::Red;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    // Case 2: Triangle
                    z = z->parent;
                    rotateLeft(z);
                }
                // Case 3: Line
                z->parent->color = Color::Black;
                z->parent->parent->color = Color::Red;
                rotateRight(z->parent->parent);
            }
        } else {
            // Mirror: parent is right child
            Node* y = z->parent->parent->left;
            if (y->color == Color::Red) {
                z->parent->color = Color::Black;
                y->color = Color::Black;
                z->parent->parent->color = Color::Red;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rotateRight(z);
                }
                z->parent->color = Color::Black;
                z->parent->parent->color = Color::Red;
                rotateLeft(z->parent->parent);
            }
        }
    }
    m_root->color = Color::Black;
}

/* ---- Insert ---- */

void RedBlackTree9::insert(int key)
{
    QElapsedTimer timer;
    timer.start();

    // Iterative descent
    Node* y = m_nil;
    Node* x = m_root;
    while (x != m_nil) {
        y = x;
        if (key < x->key) x = x->left;
        else if (key > x->key) x = x->right;
        else return; // Duplicate
    }

    Node* z = new Node{key, 1, Color::Red, m_nil, m_nil, y};
    if (y == m_nil) m_root = z;
    else if (key < y->key) y->left = z;
    else y->right = z;

    updateSize(z);
    insertFixup(z);

    m_stats.totalOperations++;
    m_stats.numNodes = m_root->subtreeSize;
    m_stats.treeHeight = treeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("insert", key);
}

/* ---- Transplant ---- */

void RedBlackTree9::transplant(Node* u, Node* v)
{
    if (u->parent == m_nil) m_root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent;
}

/* ---- Find minimum ---- */

RedBlackTree9::Node* RedBlackTree9::findMin(Node* n) const
{
    while (n->left != m_nil) n = n->left;
    return n;
}

/* ---- Delete fixup (iterative) ---- */

void RedBlackTree9::deleteFixup(Node* x)
{
    while (x != m_root && x->color == Color::Black) {
        if (x == x->parent->left) {
            Node* w = x->parent->right;
            if (w->color == Color::Red) {
                w->color = Color::Black;
                x->parent->color = Color::Red;
                rotateLeft(x->parent);
                w = x->parent->right;
            }
            if (w->left->color == Color::Black &&
                w->right->color == Color::Black) {
                w->color = Color::Red;
                x = x->parent;
            } else {
                if (w->right->color == Color::Black) {
                    w->left->color = Color::Black;
                    w->color = Color::Red;
                    rotateRight(w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = Color::Black;
                w->right->color = Color::Black;
                rotateLeft(x->parent);
                x = m_root;
            }
        } else {
            Node* w = x->parent->left;
            if (w->color == Color::Red) {
                w->color = Color::Black;
                x->parent->color = Color::Red;
                rotateRight(x->parent);
                w = x->parent->left;
            }
            if (w->right->color == Color::Black &&
                w->left->color == Color::Black) {
                w->color = Color::Red;
                x = x->parent;
            } else {
                if (w->left->color == Color::Black) {
                    w->right->color = Color::Black;
                    w->color = Color::Red;
                    rotateLeft(w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = Color::Black;
                w->left->color = Color::Black;
                rotateRight(x->parent);
                x = m_root;
            }
        }
    }
    x->color = Color::Black;
}

/* ---- Remove ---- */

void RedBlackTree9::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    // Find node
    Node* z = m_root;
    while (z != m_nil) {
        if (key < z->key) z = z->left;
        else if (key > z->key) z = z->right;
        else break;
    }
    if (z == m_nil) return;

    Node* y = z;
    Color yOrigColor = y->color;
    Node* x = m_nil;

    if (z->left == m_nil) {
        x = z->right;
        transplant(z, z->right);
    } else if (z->right == m_nil) {
        x = z->left;
        transplant(z, z->left);
    } else {
        y = findMin(z->right);
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
    updateSize(x->parent != m_nil ? x->parent : m_root);

    if (yOrigColor == Color::Black)
        deleteFixup(x);

    m_stats.totalOperations++;
    m_stats.numNodes = m_root->subtreeSize;
    m_stats.treeHeight = treeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("remove", key);
}

/* ---- Contains ---- */

bool RedBlackTree9::contains(int key) const
{
    Node* n = m_root;
    while (n != m_nil) {
        if (key < n->key) n = n->left;
        else if (key > n->key) n = n->right;
        else return true;
    }
    return false;
}

/* ---- Range query ---- */

void RedBlackTree9::rangeHelper(Node* n, int lo, int hi,
                                   QVector<int>& result) const
{
    if (n == m_nil) return;
    if (lo < n->key) rangeHelper(n->left, lo, hi, result);
    if (lo <= n->key && n->key <= hi) result.append(n->key);
    if (hi > n->key) rangeHelper(n->right, lo, hi, result);
}

QVector<int> RedBlackTree9::rangeQuery(int lo, int hi) const
{
    QVector<int> result;
    rangeHelper(m_root, lo, hi, result);
    return result;
}

/* ---- Select k-th ---- */

int RedBlackTree9::selectKth(int k) const
{
    if (k < 0 || k >= m_root->subtreeSize) return -1;
    Node* n = m_root;
    while (n != m_nil) {
        int leftSize = n->left->subtreeSize;
        if (k < leftSize) n = n->left;
        else if (k == leftSize) return n->key;
        else { k -= leftSize + 1; n = n->right; }
    }
    return -1;
}

/* ---- Rank ---- */

int RedBlackTree9::rank(int key) const
{
    int r = 0;
    Node* n = m_root;
    while (n != m_nil) {
        if (key < n->key) n = n->left;
        else if (key > n->key) { r += n->left->subtreeSize + 1; n = n->right; }
        else return r + n->left->subtreeSize;
    }
    return r;
}

/* ---- Utilities ---- */

void RedBlackTree9::inorderHelper(Node* n, QVector<int>& result) const
{
    if (n == m_nil) return;
    inorderHelper(n->left, result);
    result.append(n->key);
    inorderHelper(n->right, result);
}

QVector<int> RedBlackTree9::inorder() const
{
    QVector<int> result;
    inorderHelper(m_root, result);
    return result;
}

int RedBlackTree9::size() const { return m_root->subtreeSize; }

int RedBlackTree9::treeHeight(Node* n) const
{
    if (n == m_nil) return 0;
    return 1 + qMax(treeHeight(n->left), treeHeight(n->right));
}

void RedBlackTree9::clear()
{
    deleteTree(m_root);
    m_root = m_nil;
}

/* ---- Reset ---- */

void RedBlackTree9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file RedBlackTree14.cpp
 * @brief RedBlackTree14 实现
 *
 * 实现红黑树：范围提取批量删除与兄弟旋转优化再平衡。
 */

#include "utils/tree252/RedBlackTree14.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

RedBlackTree14::RedBlackTree14(QObject *parent) : QObject(parent)
{
    m_nil = new Node();
    m_nil->color = Black;
    m_nil->left = m_nil;
    m_nil->right = m_nil;
    m_nil->parent = m_nil;
    m_root = m_nil;
}

RedBlackTree14::~RedBlackTree14() { deleteTree(m_root); delete m_nil; }

/* ---- Delete tree ---- */

void RedBlackTree14::deleteTree(Node* node)
{
    if (node == m_nil) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

/* ---- Rotations ---- */

void RedBlackTree14::rotateLeft(Node* x)
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

void RedBlackTree14::rotateRight(Node* y)
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

/* ---- Insert fixup ---- */

void RedBlackTree14::insertFixup(Node* z)
{
    while (z->parent->color == Red) {
        if (z->parent == z->parent->parent->left) {
            Node* y = z->parent->parent->right;
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

void RedBlackTree14::transplant(Node* u, Node* v)
{
    if (u->parent == m_nil) m_root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent;
}

/* ---- Minimum ---- */

RedBlackTree14::Node* RedBlackTree14::minimum(Node* x) const
{
    while (x->left != m_nil) x = x->left;
    return x;
}

/* ---- Sibling rotation optimization ---- */

void RedBlackTree14::siblingRotationOpt(Node* parent, bool isLeft)
{
    // Try single rotation first (optimization over standard double rotation)
    Node* sibling = isLeft ? parent->right : parent->left;
    if (sibling == m_nil) return;

    if (isLeft) {
        if (sibling->right->color == Red) {
            // Single rotation sufficient
            sibling->color = parent->color;
            parent->color = Black;
            sibling->right->color = Black;
            rotateLeft(parent);
        } else if (sibling->left->color == Red) {
            // Need double rotation but optimize via color flip
            sibling->left->color = Black;
            sibling->color = Red;
            rotateRight(sibling);
            sibling = parent->right;
            sibling->color = parent->color;
            parent->color = Black;
            sibling->right->color = Black;
            rotateLeft(parent);
        }
    } else {
        if (sibling->left->color == Red) {
            sibling->color = parent->color;
            parent->color = Black;
            sibling->left->color = Black;
            rotateRight(parent);
        } else if (sibling->right->color == Red) {
            sibling->right->color = Black;
            sibling->color = Red;
            rotateLeft(sibling);
            sibling = parent->left;
            sibling->color = parent->color;
            parent->color = Black;
            sibling->left->color = Black;
            rotateRight(parent);
        }
    }
}

/* ---- Delete fixup ---- */

void RedBlackTree14::deleteFixup(Node* x)
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
                siblingRotationOpt(x->parent, true);
                x = m_root; // Terminate after sibling rotation
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
                siblingRotationOpt(x->parent, false);
                x = m_root;
            }
        }
    }
    x->color = Black;
}

/* ---- Extract nodes in range ---- */

void RedBlackTree14::extractRange(Node* node, int lo, int hi, QVector<Node*>& extracted)
{
    if (node == m_nil) return;
    if (node->key > lo) extractRange(node->left, lo, hi, extracted);
    if (node->key >= lo && node->key <= hi) extracted.append(node);
    if (node->key < hi) extractRange(node->right, lo, hi, extracted);
}

/* ---- In-order traversal ---- */

void RedBlackTree14::inorder(Node* node, QVector<int>& result) const
{
    if (node == m_nil) return;
    inorder(node->left, result);
    result.append(node->key);
    inorder(node->right, result);
}

/* ---- Height ---- */

int RedBlackTree14::heightRec(Node* node) const
{
    if (node == m_nil) return 0;
    return 1 + qMax(heightRec(node->left), heightRec(node->right));
}

/* ---- Insert ---- */

void RedBlackTree14::insert(int key)
{
    QElapsedTimer timer;
    timer.start();

    Node* z = new Node();
    z->key = key;
    z->left = m_nil;
    z->right = m_nil;
    z->parent = m_nil;
    z->color = Red;

    Node* y = m_nil;
    Node* x = m_root;
    while (x != m_nil) {
        y = x;
        x = (z->key < x->key) ? x->left : x->right;
    }

    z->parent = y;
    if (y == m_nil) m_root = z;
    else if (z->key < y->key) y->left = z;
    else y->right = z;

    insertFixup(z);
    m_stats.numNodes++;
    m_stats.numInsertions++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Remove single key ---- */

void RedBlackTree14::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    // Find node
    Node* z = m_root;
    while (z != m_nil && z->key != key)
        z = (key < z->key) ? z->left : z->right;
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

    if (yOrigColor == Black) deleteFixup(x);
    delete z;

    m_stats.numNodes--;
    m_stats.numDeletions++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
}

/* ---- Bulk delete range ---- */

void RedBlackTree14::bulkDelete(int lo, int hi)
{
    QElapsedTimer timer;
    timer.start();

    // Collect keys in range, then delete one by one
    QVector<int> toRemove;
    Node* current = m_root;
    // In-order collect
    QVector<Node*> nodes;
    extractRange(m_root, lo, hi, nodes);
    int removed = nodes.size();

    for (auto* nd : nodes) {
        int k = nd->key;
        // Re-remove (key may have shifted tree)
        remove(k);
    }

    m_stats.numBulkDeletes++;
    emit bulkDeleteCompleted(lo, hi, removed, timer.elapsed());
}

/* ---- Contains ---- */

bool RedBlackTree14::contains(int key) const
{
    Node* x = m_root;
    while (x != m_nil) {
        if (key == x->key) return true;
        x = (key < x->key) ? x->left : x->right;
    }
    return false;
}

/* ---- Keys in sorted order ---- */

QVector<int> RedBlackTree14::keys() const
{
    QVector<int> result;
    inorder(m_root, result);
    return result;
}

/* ---- Height ---- */

int RedBlackTree14::height() const { return heightRec(m_root); }

/* ---- Clear ---- */

void RedBlackTree14::clear()
{
    deleteTree(m_root);
    m_root = m_nil;
    m_stats.numNodes = 0;
}

/* ---- Reset ---- */

void RedBlackTree14::resetStatistics()
{
    clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

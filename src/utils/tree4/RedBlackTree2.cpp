/**
 * @file RedBlackTree2.cpp
 * @brief 红黑树增强版实现
 */

#include "RedBlackTree2.h"
#include <QElapsedTimer>

RedBlackTree2::RedBlackTree2(QObject* parent)
    : QObject(parent)
    , m_count(0)
    , m_timeSum(0.0)
{
    m_nil = new Node{0.0, QVariant(), Black, nullptr, nullptr, nullptr, 0};
    m_nil->left = m_nil;
    m_nil->right = m_nil;
    m_nil->parent = m_nil;
    m_root = m_nil;
}

RedBlackTree2::~RedBlackTree2()
{
    deleteTree(m_root);
    delete m_nil;
}

void RedBlackTree2::deleteTree(Node* node)
{
    if (node == m_nil) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

void RedBlackTree2::leftRotate(Node* x)
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
    updateSize(x);
    updateSize(y);
    m_stats.totalRotations++;
}

void RedBlackTree2::rightRotate(Node* y)
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
    updateSize(y);
    updateSize(x);
    m_stats.totalRotations++;
}

void RedBlackTree2::updateSize(Node* node)
{
    if (node != m_nil)
        node->subtreeSize = 1 + node->left->subtreeSize + node->right->subtreeSize;
}

void RedBlackTree2::insert(double key, const QVariant& value)
{
    QElapsedTimer timer;
    timer.start();

    Node* z = new Node{key, value, Red, m_nil, m_nil, m_nil, 1};
    Node* y = m_nil;
    Node* x = m_root;

    while (x != m_nil) {
        y = x;
        x->subtreeSize++;
        if (key < x->key) x = x->left;
        else if (key > x->key) x = x->right;
        else { x->value = value; delete z; return; }
    }

    z->parent = y;
    if (y == m_nil) m_root = z;
    else if (key < y->key) y->left = z;
    else y->right = z;

    insertFixup(z);
    m_count++;

    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalInserts + m_stats.totalRemoves + m_stats.totalSearches;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit insertCompleted(key);
}

void RedBlackTree2::insertFixup(Node* z)
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
                if (z == z->parent->right) { z = z->parent; leftRotate(z); }
                z->parent->color = Black;
                z->parent->parent->color = Red;
                rightRotate(z->parent->parent);
            }
        } else {
            Node* y = z->parent->parent->left;
            if (y->color == Red) {
                z->parent->color = Black;
                y->color = Black;
                z->parent->parent->color = Red;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) { z = z->parent; rightRotate(z); }
                z->parent->color = Black;
                z->parent->parent->color = Red;
                leftRotate(z->parent->parent);
            }
        }
    }
    m_root->color = Black;
}

RedBlackTree2::Node* RedBlackTree2::findNode(double key) const
{
    Node* cur = m_root;
    while (cur != m_nil) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return cur;
    }
    return m_nil;
}

QVariant RedBlackTree2::search(double key) const
{
    m_stats.totalSearches++;
    Node* n = findNode(key);
    return (n != m_nil) ? n->value : QVariant();
}

bool RedBlackTree2::contains(double key) const { return findNode(key) != m_nil; }

double RedBlackTree2::kth(int k) const
{
    Node* cur = m_root;
    while (cur != m_nil) {
        int leftSize = cur->left->subtreeSize;
        if (k < leftSize) cur = cur->left;
        else if (k == leftSize) return cur->key;
        else { k -= leftSize + 1; cur = cur->right; }
    }
    return 0.0;
}

int RedBlackTree2::rank(double key) const
{
    int r = 0;
    Node* cur = m_root;
    while (cur != m_nil) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) { r += cur->left->subtreeSize + 1; cur = cur->right; }
        else { r += cur->left->subtreeSize; break; }
    }
    return r;
}

int RedBlackTree2::rangeCount(double lo, double hi) const
{
    return rank(hi) - rank(lo);
}

void RedBlackTree2::transplant(Node* u, Node* v)
{
    if (u->parent == m_nil) m_root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent;
}

RedBlackTree2::Node* RedBlackTree2::treeMinimum(Node* x) const
{
    while (x->left != m_nil) x = x->left;
    return x;
}

bool RedBlackTree2::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    Node* z = findNode(key);
    if (z == m_nil) {
        emit removeCompleted(key, false);
        return false;
    }

    Node* y = z;
    Color yOrigColor = y->color;
    Node* x;

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
        if (y->parent == z) x->parent = y;
        else { transplant(y, y->right); y->right = z->right; y->right->parent = y; }
        transplant(z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    /* 更新路径上的subtreeSize */
    Node* p = x->parent;
    while (p != m_nil) { updateSize(p); p = p->parent; }

    if (yOrigColor == Black) deleteFixup(x);
    delete z;
    m_count--;

    m_stats.totalRemoves++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalInserts + m_stats.totalRemoves + m_stats.totalSearches;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit removeCompleted(key, true);
    return true;
}

void RedBlackTree2::deleteFixup(Node* x)
{
    while (x != m_root && x->color == Black) {
        if (x == x->parent->left) {
            Node* w = x->parent->right;
            if (w->color == Red) { w->color = Black; x->parent->color = Red; leftRotate(x->parent); w = x->parent->right; }
            if (w->left->color == Black && w->right->color == Black) { w->color = Red; x = x->parent; }
            else { if (w->right->color == Black) { w->left->color = Black; w->color = Red; rightRotate(w); w = x->parent->right; }
                w->color = x->parent->color; x->parent->color = Black; w->right->color = Black; leftRotate(x->parent); x = m_root; }
        } else {
            Node* w = x->parent->left;
            if (w->color == Red) { w->color = Black; x->parent->color = Red; rightRotate(x->parent); w = x->parent->left; }
            if (w->right->color == Black && w->left->color == Black) { w->color = Red; x = x->parent; }
            else { if (w->left->color == Black) { w->right->color = Black; w->color = Red; leftRotate(w); w = x->parent->left; }
                w->color = x->parent->color; x->parent->color = Black; w->left->color = Black; rightRotate(x->parent); x = m_root; }
        }
    }
    x->color = Black;
}

void RedBlackTree2::inOrderHelper(Node* node, QVector<double>& result) const
{
    if (node == m_nil) return;
    inOrderHelper(node->left, result);
    result.append(node->key);
    inOrderHelper(node->right, result);
}

QVector<double> RedBlackTree2::inOrderKeys() const
{
    QVector<double> result;
    inOrderHelper(m_root, result);
    return result;
}

int RedBlackTree2::size() const { return m_count; }

int RedBlackTree2::heightHelper(Node* node) const
{
    if (node == m_nil) return 0;
    return 1 + qMax(heightHelper(node->left), heightHelper(node->right));
}

int RedBlackTree2::height() const { return heightHelper(m_root); }

bool RedBlackTree2::verifyHelper(Node* node, int blackCount, int& pathBlackCount) const
{
    if (node == m_nil) {
        if (pathBlackCount < 0) pathBlackCount = blackCount;
        return blackCount == pathBlackCount;
    }
    if (node->color == Red) {
        if (node->left->color == Red || node->right->color == Red) return false;
    } else blackCount++;
    return verifyHelper(node->left, blackCount, pathBlackCount)
        && verifyHelper(node->right, blackCount, pathBlackCount);
}

bool RedBlackTree2::verify() const
{
    if (m_root->color != Black) return false;
    int pathBlack = -1;
    return verifyHelper(m_root, 0, pathBlack);
}

RedBlackTree2::Stats RedBlackTree2::stats() const { return m_stats; }

void RedBlackTree2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

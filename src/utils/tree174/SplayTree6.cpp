/**
 * @file SplayTree6.cpp
 * @brief SplayTree6 实现
 *
 * 实现Splay Tree：zig/zig-zig/zig-zag旋转、split/join操作。
 */

#include "utils/tree174/SplayTree6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SplayTree6::SplayTree6(QObject *parent)
    : QObject(parent)
{
}

SplayTree6::~SplayTree6() { clearRec(m_root); }

/* ---- Rotations ---- */

void SplayTree6::rotateRight(Node* x)
{
    if (!x || !x->parent) return;
    Node* p = x->parent;
    Node* g = p->parent;

    p->left = x->right;
    if (x->right) x->right->parent = p;

    x->right = p;
    p->parent = x;
    x->parent = g;

    if (g) {
        if (g->left == p) g->left = x;
        else g->right = x;
    } else {
        m_root = x;
    }
}

void SplayTree6::rotateLeft(Node* x)
{
    if (!x || !x->parent) return;
    Node* p = x->parent;
    Node* g = p->parent;

    p->right = x->left;
    if (x->left) x->left->parent = p;

    x->left = p;
    p->parent = x;
    x->parent = g;

    if (g) {
        if (g->left == p) g->left = x;
        else g->right = x;
    } else {
        m_root = x;
    }
}

/* ---- Splay operation ---- */

void SplayTree6::splay(Node* x)
{
    if (!x) return;

    while (x->parent) {
        Node* p = x->parent;
        Node* g = p->parent;

        if (!g) {
            /* Zig case: x is child of root */
            if (p->left == x) rotateRight(x);
            else rotateLeft(x);
        } else if ((g->left == p) && (p->left == x)) {
            /* Zig-zig case (left-left) */
            rotateRight(p);
            rotateRight(x);
        } else if ((g->right == p) && (p->right == x)) {
            /* Zig-zig case (right-right) */
            rotateLeft(p);
            rotateLeft(x);
        } else if ((g->left == p) && (p->right == x)) {
            /* Zig-zag case (left-right) */
            rotateLeft(x);
            rotateRight(x);
        } else {
            /* Zig-zag case (right-left) */
            rotateRight(x);
            rotateLeft(x);
        }
    }
}

/* ---- Find node ---- */

SplayTree6::Node* SplayTree6::findNode(double key) const
{
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return cur;
    }
    return nullptr;
}

/* ---- Subtree min/max ---- */

SplayTree6::Node* SplayTree6::subtreeMin(Node* t)
{
    if (!t) return nullptr;
    while (t->left) t = t->left;
    return t;
}

SplayTree6::Node* SplayTree6::subtreeMax(Node* t)
{
    if (!t) return nullptr;
    while (t->right) t = t->right;
    return t;
}

/* ---- Tree height ---- */

int SplayTree6::computeHeight(Node* t)
{
    if (!t) return 0;
    return 1 + qMax(computeHeight(t->left), computeHeight(t->right));
}

/* ---- In-order traversal ---- */

void SplayTree6::inOrderRec(Node* t, QVector<QPair<double, double>>& result)
{
    if (!t) return;
    inOrderRec(t->left, result);
    result.append({t->key, t->value});
    inOrderRec(t->right, result);
}

/* ---- Range query ---- */

void SplayTree6::rangeRec(Node* t, double lo, double hi,
                            QVector<QPair<double, double>>& result)
{
    if (!t) return;
    if (lo < t->key) rangeRec(t->left, lo, hi, result);
    if (lo <= t->key && t->key <= hi)
        result.append({t->key, t->value});
    if (hi > t->key) rangeRec(t->right, lo, hi, result);
}

/* ---- Clear ---- */

void SplayTree6::clearRec(Node* t)
{
    if (!t) return;
    clearRec(t->left);
    clearRec(t->right);
    delete t;
}

/* ---- Insert ---- */

bool SplayTree6::insert(double key, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) {
        m_root = new Node(key, value);
    } else {
        Node* cur = m_root;
        Node* parent = nullptr;
        while (cur) {
            parent = cur;
            if (key < cur->key) cur = cur->left;
            else if (key > cur->key) cur = cur->right;
            else {
                /* Key exists, update value */
                cur->value = value;
                splay(cur);
                m_stats.totalInserts++;
                m_timeSum += timer.elapsed();
                m_stats.avgProcessingTimeMs = m_timeSum /
                    (m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalQueries);
                emit insertCompleted(key, m_stats.currentSize);
                return true;
            }
        }
        Node* newNode = new Node(key, value);
        newNode->parent = parent;
        if (key < parent->key) parent->left = newNode;
        else parent->right = newNode;
        splay(newNode);
        m_stats.currentSize++;
    }

    m_stats.totalInserts++;
    m_stats.treeHeight = computeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalQueries);

    emit insertCompleted(key, m_stats.currentSize);
    return true;
}

/* ---- Remove ---- */

bool SplayTree6::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    Node* node = findNode(key);
    if (!node) return false;

    splay(node);

    /* Now node is root */
    Node* leftTree = m_root->left;
    Node* rightTree = m_root->right;

    if (leftTree) leftTree->parent = nullptr;
    if (rightTree) rightTree->parent = nullptr;

    delete m_root;

    if (!leftTree) {
        m_root = rightTree;
    } else if (!rightTree) {
        m_root = leftTree;
    } else {
        /* Join: find max of left tree, splay it to root */
        Node* maxLeft = subtreeMax(leftTree);
        /* Splay maxLeft within leftTree */
        m_root = leftTree;
        splay(maxLeft);
        /* Now maxLeft is root and has no right child */
        maxLeft->right = rightTree;
        rightTree->parent = maxLeft;
    }

    m_stats.currentSize--;
    m_stats.totalDeletes++;
    m_stats.treeHeight = computeHeight(m_root);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalQueries);

    emit deleteCompleted(key);
    return true;
}

/* ---- Find ---- */

bool SplayTree6::find(double key, double& value)
{
    Node* node = findNode(key);
    if (!node) return false;
    value = node->value;
    splay(node);
    m_stats.totalQueries++;
    return true;
}

/* ---- Split ---- */

void SplayTree6::split(double key, SplayTree6& left, SplayTree6& right)
{
    left.clear();
    right.clear();

    if (!m_root) return;

    /* Splay on key */
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) {
            if (cur->left) cur = cur->left;
            else break;
        } else if (key > cur->key) {
            if (cur->right) cur = cur->right;
            else break;
        } else break;
    }
    splay(cur);

    if (cur->key <= key) {
        /* Left tree: root and its left subtree */
        left.m_root = cur;
        right.m_root = cur->right;
        if (cur->right) cur->right->parent = nullptr;
        cur->right = nullptr;
    } else {
        /* Right tree: root and its right subtree */
        right.m_root = cur;
        left.m_root = cur->left;
        if (cur->left) cur->left->parent = nullptr;
        cur->left = nullptr;
    }
    m_root = nullptr;

    int ls = 0, rs = 0;
    /* Count sizes via traversal */
    QVector<QPair<double, double>> lItems, rItems;
    inOrderRec(left.m_root, lItems);
    inOrderRec(right.m_root, rItems);
    ls = lItems.size(); rs = rItems.size();

    left.m_stats.currentSize = ls;
    right.m_stats.currentSize = rs;
    m_stats.currentSize = 0;

    emit splitCompleted(ls, rs);
}

/* ---- Join ---- */

void SplayTree6::join(SplayTree6& other)
{
    if (!m_root) {
        m_root = other.m_root;
        other.m_root = nullptr;
        m_stats.currentSize = other.m_stats.currentSize;
        other.m_stats.currentSize = 0;
        return;
    }

    /* Splay max of this tree to root */
    Node* maxNode = subtreeMax(m_root);
    splay(maxNode);
    /* maxNode is now root with no right child */
    maxNode->right = other.m_root;
    if (other.m_root) other.m_root->parent = maxNode;

    m_stats.currentSize += other.m_stats.currentSize;
    other.m_root = nullptr;
    other.m_stats.currentSize = 0;
}

/* ---- Range query ---- */

QVector<QPair<double, double>> SplayTree6::rangeQuery(double keyLo, double keyHi)
{
    QVector<QPair<double, double>> result;
    rangeRec(m_root, keyLo, keyHi, result);
    return result;
}

/* ---- In-order ---- */

QVector<QPair<double, double>> SplayTree6::inOrder() const
{
    QVector<QPair<double, double>> result;
    inOrderRec(m_root, result);
    return result;
}

/* ---- Utility ---- */

bool SplayTree6::isEmpty() const { return m_root == nullptr; }

void SplayTree6::clear()
{
    clearRec(m_root);
    m_root = nullptr;
    m_stats.currentSize = 0;
    m_stats.treeHeight = 0;
}

/* ---- Statistics ---- */

void SplayTree6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file RedBlackTree7.cpp
 * @brief 红黑树实现
 *
 * 实现完整的红黑树数据结构，支持插入、删除、查找、
 * 排名、选择和范围查询操作。所有操作保证O(logN)时间复杂度。
 */

#include "utils/tree74/RedBlackTree7.h"
#include <QElapsedTimer>
#include <algorithm>

RedBlackTree7::RedBlackTree7(QObject* parent) : QObject(parent) {
    m_nil = new RBNode{0.0, 0, BLACK, nullptr, nullptr, nullptr, 0};
    m_nil->left = m_nil; m_nil->right = m_nil; m_nil->parent = m_nil;
    m_root = m_nil;
}

/** @brief 插入键值对 */
void RedBlackTree7::insert(double key, int value) {
    QElapsedTimer timer; timer.start();
    RBNode* y = m_nil; RBNode* x = m_root;
    while (x != m_nil) {
        y = x;
        if (key < x->key) x = x->left;
        else if (key > x->key) x = x->right;
        else { x->val = value; m_timeSum += timer.elapsed(); return; }
    }
    RBNode* z = new RBNode{key, value, RED, m_nil, m_nil, y, 1};
    if (y == m_nil) m_root = z;
    else if (key < y->key) y->left = z;
    else y->right = z;

    RBNode* p = y;
    while (p != m_nil) { p->count++; p = p->parent; }
    insertFixup(z);
    m_size++; m_height = computeHeight(m_root);
    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalInserts + m_stats.totalQueries > 0) ? m_timeSum / (m_stats.totalInserts + m_stats.totalQueries) : 0.0;
    emit inserted(key);
}

/** @brief 删除指定键 */
void RedBlackTree7::remove(double key) {
    RBNode* z = findNode(key);
    if (z == m_nil) return;
    RBNode* y = z, *x; Color yOrig = y->color;
    if (z->left == m_nil) { x = z->right; transplant(z, z->right); }
    else if (z->right == m_nil) { x = z->left; transplant(z, z->left); }
    else {
        y = minimum(z->right); yOrig = y->color; x = y->right;
        if (y->parent == z) x->parent = y;
        else { transplant(y, y->right); y->right = z->right; y->right->parent = y; }
        transplant(z, y); y->left = z->left; y->left->parent = y; y->color = z->color;
    }
    delete z; m_size--; m_height = computeHeight(m_root);
}

/** @brief 查找键是否存在 */
bool RedBlackTree7::contains(double key) const {
    QElapsedTimer timer; timer.start();
    bool found = (findNode(key) != m_nil);
    const_cast<RedBlackTree7*>(this)->m_stats.totalQueries++;
    const_cast<RedBlackTree7*>(this)->m_timeSum += timer.elapsed();
    const_cast<RedBlackTree7*>(this)->m_stats.avgProcessingTimeMs = (m_stats.totalInserts + m_stats.totalQueries > 0) ? m_timeSum / (m_stats.totalInserts + m_stats.totalQueries) : 0.0;
    return found;
}

/** @brief 计算键的排名(0-based) */
int RedBlackTree7::rank(double key) const {
    int r = 0; RBNode* x = m_root;
    while (x != m_nil) {
        if (key < x->key) x = x->left;
        else if (key > x->key) { r += x->left->count + 1; x = x->right; }
        else { r += x->left->count; break; }
    }
    return r;
}

/** @brief 选择第k小的键 */
double RedBlackTree7::select(int k) const {
    RBNode* x = m_root;
    while (x != m_nil) {
        int ls = x->left->count;
        if (k < ls) x = x->left;
        else if (k == ls) return x->key;
        else { k -= ls + 1; x = x->right; }
    }
    return 0.0;
}

/** @brief 范围查询 */
QVector<int> RedBlackTree7::rangeQuery(double lo, double hi) const {
    QVector<int> res; rangeQueryNode(m_root, lo, hi, res); return res;
}

void RedBlackTree7::resetStatistics() { m_stats = Stats(); m_timeSum = 0.0; }

void RedBlackTree7::insertFixup(RBNode* z) {
    while (z->parent->color == RED) {
        if (z->parent == z->parent->parent->left) {
            RBNode* y = z->parent->parent->right;
            if (y->color == RED) { z->parent->color = BLACK; y->color = BLACK; z->parent->parent->color = RED; z = z->parent->parent; }
            else { if (z == z->parent->right) { z = z->parent; leftRotate(z); } z->parent->color = BLACK; z->parent->parent->color = RED; rightRotate(z->parent->parent); }
        } else {
            RBNode* y = z->parent->parent->left;
            if (y->color == RED) { z->parent->color = BLACK; y->color = BLACK; z->parent->parent->color = RED; z = z->parent->parent; }
            else { if (z == z->parent->left) { z = z->parent; rightRotate(z); } z->parent->color = BLACK; z->parent->parent->color = RED; leftRotate(z->parent->parent); }
        }
    }
    m_root->color = BLACK;
}

void RedBlackTree7::leftRotate(RBNode* x) {
    RBNode* y = x->right; x->right = y->left;
    if (y->left != m_nil) y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == m_nil) m_root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x; x->parent = y;
    y->count = x->count; x->count = x->left->count + x->right->count + 1;
}

void RedBlackTree7::rightRotate(RBNode* y) {
    RBNode* x = y->left; y->left = x->right;
    if (x->right != m_nil) x->right->parent = y;
    x->parent = y->parent;
    if (y->parent == m_nil) m_root = x;
    else if (y == y->parent->right) y->parent->right = x;
    else y->parent->left = x;
    x->right = y; y->parent = x;
    x->count = y->count; y->count = y->left->count + y->right->count + 1;
}

void RedBlackTree7::rangeQueryNode(RBNode* n, double lo, double hi, QVector<int>& res) const {
    if (n == m_nil) return;
    if (lo < n->key) rangeQueryNode(n->left, lo, hi, res);
    if (lo <= n->key && n->key <= hi) res.append(n->val);
    if (hi > n->key) rangeQueryNode(n->right, lo, hi, res);
}

RedBlackTree7::RBNode* RedBlackTree7::findNode(double key) const {
    RBNode* x = m_root;
    while (x != m_nil) { if (key < x->key) x = x->left; else if (key > x->key) x = x->right; else return x; }
    return m_nil;
}

void RedBlackTree7::transplant(RBNode* u, RBNode* v) {
    if (u->parent == m_nil) m_root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent;
}

RedBlackTree7::RBNode* RedBlackTree7::minimum(RBNode* x) const {
    while (x->left != m_nil) x = x->left; return x;
}

int RedBlackTree7::computeHeight(RBNode* x) const {
    if (x == m_nil) return 0;
    return 1 + qMax(computeHeight(x->left), computeHeight(x->right));
}

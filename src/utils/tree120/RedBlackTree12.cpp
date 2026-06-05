#include "RedBlackTree12.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 红黑树节点
 */
struct RBNode12 {
    int key;
    double value;
    bool isRed;
    RBNode12* left;
    RBNode12* right;
    RBNode12* parent;
    explicit RBNode12(int k, double v)
        : key(k), value(v), isRed(true), left(nullptr), right(nullptr), parent(nullptr) {}
};

static RBNode12* rbRoot12 = nullptr;

static void rbRotL12(RBNode12* x)
{
    RBNode12* y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent) rbRoot12 = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x; x->parent = y;
}

static void rbRotR12(RBNode12* x)
{
    RBNode12* y = x->left;
    x->left = y->right;
    if (y->right) y->right->parent = x;
    y->parent = x->parent;
    if (!x->parent) rbRoot12 = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->right = x; x->parent = y;
}

static void rbFix12(RBNode12* z)
{
    while (z->parent && z->parent->isRed) {
        if (z->parent == z->parent->parent->left) {
            RBNode12* u = z->parent->parent->right;
            if (u && u->isRed) {
                z->parent->isRed = false; u->isRed = false;
                z->parent->parent->isRed = true; z = z->parent->parent;
            } else {
                if (z == z->parent->right) { z = z->parent; rbRotL12(z); }
                z->parent->isRed = false; z->parent->parent->isRed = true;
                rbRotR12(z->parent->parent);
            }
        } else {
            RBNode12* u = z->parent->parent->left;
            if (u && u->isRed) {
                z->parent->isRed = false; u->isRed = false;
                z->parent->parent->isRed = true; z = z->parent->parent;
            } else {
                if (z == z->parent->left) { z = z->parent; rbRotR12(z); }
                z->parent->isRed = false; z->parent->parent->isRed = true;
                rbRotL12(z->parent->parent);
            }
        }
    }
    rbRoot12->isRed = false;
}

static RBNode12* rbMin12(RBNode12* n) { while (n && n->left) n = n->left; return n; }

static void rbTrans12(RBNode12* u, RBNode12* v)
{
    if (!u->parent) rbRoot12 = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    if (v) v->parent = u->parent;
}

static void rbDelFix12(RBNode12* x)
{
    while (x && x != rbRoot12 && !x->isRed) {
        if (x == x->parent->left) {
            RBNode12* w = x->parent->right;
            if (w && w->isRed) { w->isRed = false; x->parent->isRed = true; rbRotL12(x->parent); w = x->parent->right; }
            if (w && (!w->left || !w->left->isRed) && (!w->right || !w->right->isRed)) {
                w->isRed = true; x = x->parent;
            } else {
                if (w && (!w->right || !w->right->isRed)) { if (w->left) w->left->isRed = false; w->isRed = true; rbRotR12(w); w = x->parent->right; }
                if (w) { w->isRed = x->parent->isRed; x->parent->isRed = false; if (w->right) w->right->isRed = false; rbRotL12(x->parent); }
                x = rbRoot12;
            }
        } else {
            RBNode12* w = x->parent->left;
            if (w && w->isRed) { w->isRed = false; x->parent->isRed = true; rbRotR12(x->parent); w = x->parent->left; }
            if (w && (!w->right || !w->right->isRed) && (!w->left || !w->left->isRed)) {
                w->isRed = true; x = x->parent;
            } else {
                if (w && (!w->left || !w->left->isRed)) { if (w->right) w->right->isRed = false; w->isRed = true; rbRotL12(w); w = x->parent->left; }
                if (w) { w->isRed = x->parent->isRed; x->parent->isRed = false; if (w->left) w->left->isRed = false; rbRotR12(x->parent); }
                x = rbRoot12;
            }
        }
    }
    if (x) x->isRed = false;
}

static void rbInOrder12(RBNode12* n, QVector<QPair<int, double>>& out)
{
    if (!n) return;
    rbInOrder12(n->left, out);
    out.append({n->key, n->value});
    rbInOrder12(n->right, out);
}

static int rbSz12(RBNode12* n) { return n ? 1 + rbSz12(n->left) + rbSz12(n->right) : 0; }

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
RedBlackTree12::RedBlackTree12(QObject* parent) : QObject(parent) {}

/**
 * @brief 重置所有统计信息
 */
void RedBlackTree12::resetStatistics() { m_stats = Stats(); m_timeSum = 0.0; }

/**
 * @brief 插入键值对
 * @param key 键
 * @param value 值
 */
void RedBlackTree12::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    RBNode12* par = nullptr;
    RBNode12* cur = rbRoot12;
    while (cur) {
        par = cur;
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else { cur->value = value; goto done; }
    }
    {
        RBNode12* z = new RBNode12(key, value);
        z->parent = par;
        if (!par) rbRoot12 = z;
        else if (key < par->key) par->left = z;
        else par->right = z;
        rbFix12(z);
    }
done:
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTreeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTreeOps;
    emit treeOperationCompleted(rbSz12(rbRoot12));
}

/**
 * @brief 删除指定键
 * @param key 待删除的键
 * @return 是否删除成功
 */
bool RedBlackTree12::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    RBNode12* z = rbRoot12;
    while (z) { if (key < z->key) z = z->left; else if (key > z->key) z = z->right; else break; }
    if (!z) {
        qint64 elapsed = timer.elapsed();
        m_timeSum += elapsed; m_stats.totalTreeOps++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTreeOps;
        emit treeOperationCompleted(rbSz12(rbRoot12));
        return false;
    }

    RBNode12* y = z; bool yRed = y->isRed; RBNode12* x = nullptr;
    if (!z->left) { x = z->right; rbTrans12(z, z->right); }
    else if (!z->right) { x = z->left; rbTrans12(z, z->left); }
    else {
        y = rbMin12(z->right); yRed = y->isRed; x = y->right;
        if (y->parent == z) { if (x) x->parent = y; }
        else { rbTrans12(y, y->right); y->right = z->right; y->right->parent = y; }
        rbTrans12(z, y); y->left = z->left; y->left->parent = y; y->isRed = z->isRed;
    }
    delete z;
    if (!yRed && x) rbDelFix12(x);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTreeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTreeOps;
    emit treeOperationCompleted(rbSz12(rbRoot12));
    return true;
}

/**
 * @brief 查找指定键对应的值
 * @param key 待查找的键
 * @return 键对应的值，未找到返回0.0
 */
double RedBlackTree12::search(int key) const
{
    RBNode12* cur = rbRoot12;
    while (cur) { if (key < cur->key) cur = cur->left; else if (key > cur->key) cur = cur->right; else return cur->value; }
    return 0.0;
}

/**
 * @brief 查找指定键的前驱节点
 * @param key 目标键
 * @return 前驱键值对，不存在则返回 (-1, 0.0)
 */
QPair<int, double> RedBlackTree12::predecessor(int key) const
{
    QPair<int, double> result = {-1, 0.0};
    RBNode12* cur = rbRoot12;
    while (cur) {
        if (cur->key < key) { result = {cur->key, cur->value}; cur = cur->right; }
        else cur = cur->left;
    }
    return result;
}

/**
 * @brief 中序遍历获取所有键值对
 * @return 有序键值对列表
 */
QVector<QPair<int, double>> RedBlackTree12::inOrderTraversal() const
{
    QVector<QPair<int, double>> result;
    rbInOrder12(rbRoot12, result);
    return result;
}

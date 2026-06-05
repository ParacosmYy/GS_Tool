#include "RedBlackTree11.h"
#include <QElapsedTimer>
#include <QVariant>
#include <QtMath>
#include <algorithm>

/**
 * @brief 红黑树节点
 */
struct RBNode11 {
    int key;
    QVariant value;
    bool isRed;
    RBNode11* left;
    RBNode11* right;
    RBNode11* parent;
    explicit RBNode11(int k, const QVariant& v)
        : key(k), value(v), isRed(true), left(nullptr), right(nullptr), parent(nullptr) {}
};

static RBNode11* rbRoot11 = nullptr;

/* 左旋转 */
static void rbRotateLeft11(RBNode11* x)
{
    RBNode11* y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent) rbRoot11 = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
}

/* 右旋转 */
static void rbRotateRight11(RBNode11* x)
{
    RBNode11* y = x->left;
    x->left = y->right;
    if (y->right) y->right->parent = x;
    y->parent = x->parent;
    if (!x->parent) rbRoot11 = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->right = x;
    x->parent = y;
}

/* 插入修复 */
static void rbInsertFix11(RBNode11* z)
{
    while (z->parent && z->parent->isRed) {
        if (z->parent == z->parent->parent->left) {
            RBNode11* u = z->parent->parent->right;
            if (u && u->isRed) {
                z->parent->isRed = false;
                u->isRed = false;
                z->parent->parent->isRed = true;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) { z = z->parent; rbRotateLeft11(z); }
                z->parent->isRed = false;
                z->parent->parent->isRed = true;
                rbRotateRight11(z->parent->parent);
            }
        } else {
            RBNode11* u = z->parent->parent->left;
            if (u && u->isRed) {
                z->parent->isRed = false;
                u->isRed = false;
                z->parent->parent->isRed = true;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) { z = z->parent; rbRotateRight11(z); }
                z->parent->isRed = false;
                z->parent->parent->isRed = true;
                rbRotateLeft11(z->parent->parent);
            }
        }
    }
    rbRoot11->isRed = false;
}

/* 子树最小节点 */
static RBNode11* rbMin11(RBNode11* n)
{
    while (n && n->left) n = n->left;
    return n;
}

/* 替换子树 */
static void rbTransplant11(RBNode11* u, RBNode11* v)
{
    if (!u->parent) rbRoot11 = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    if (v) v->parent = u->parent;
}

/* 删除修复 */
static void rbDeleteFix11(RBNode11* x)
{
    while (x && x != rbRoot11 && !x->isRed) {
        if (x == x->parent->left) {
            RBNode11* w = x->parent->right;
            if (w && w->isRed) { w->isRed = false; x->parent->isRed = true; rbRotateLeft11(x->parent); w = x->parent->right; }
            if (w && (!w->left || !w->left->isRed) && (!w->right || !w->right->isRed)) {
                w->isRed = true; x = x->parent;
            } else {
                if (w && (!w->right || !w->right->isRed)) { if (w->left) w->left->isRed = false; w->isRed = true; rbRotateRight11(w); w = x->parent->right; }
                if (w) { w->isRed = x->parent->isRed; x->parent->isRed = false; if (w->right) w->right->isRed = false; rbRotateLeft11(x->parent); }
                x = rbRoot11;
            }
        } else {
            RBNode11* w = x->parent->left;
            if (w && w->isRed) { w->isRed = false; x->parent->isRed = true; rbRotateRight11(x->parent); w = x->parent->left; }
            if (w && (!w->right || !w->right->isRed) && (!w->left || !w->left->isRed)) {
                w->isRed = true; x = x->parent;
            } else {
                if (w && (!w->left || !w->left->isRed)) { if (w->right) w->right->isRed = false; w->isRed = true; rbRotateLeft11(w); w = x->parent->left; }
                if (w) { w->isRed = x->parent->isRed; x->parent->isRed = false; if (w->left) w->left->isRed = false; rbRotateRight11(x->parent); }
                x = rbRoot11;
            }
        }
    }
    if (x) x->isRed = false;
}

/* 范围查询辅助 */
static void rbRange11(RBNode11* n, int lo, int hi, QVector<QPair<int, QVariant>>& out)
{
    if (!n) return;
    if (lo < n->key) rbRange11(n->left, lo, hi, out);
    if (lo <= n->key && n->key <= hi) out.append({n->key, n->value});
    if (hi > n->key) rbRange11(n->right, lo, hi, out);
}

/* 计算子树大小 */
static int rbSize11(RBNode11* n) { return n ? 1 + rbSize11(n->left) + rbSize11(n->right) : 0; }

/**
 * @brief 构造函数，初始化红黑树
 * @param parent 父对象指针
 */
RedBlackTree11::RedBlackTree11(QObject* parent) : QObject(parent) {}

/**
 * @brief 重置所有统计信息
 */
void RedBlackTree11::resetStatistics() { m_stats = Stats(); m_timeSum = 0.0; }

/**
 * @brief 插入键值对，插入后通过旋转和重新着色保持平衡
 * @param key 键
 * @param value 值
 */
void RedBlackTree11::insert(int key, const QVariant& value)
{
    QElapsedTimer timer;
    timer.start();

    RBNode11* par = nullptr;
    RBNode11* cur = rbRoot11;
    while (cur) {
        par = cur;
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else { cur->value = value; goto done; }
    }
    {
        RBNode11* z = new RBNode11(key, value);
        z->parent = par;
        if (!par) rbRoot11 = z;
        else if (key < par->key) par->left = z;
        else par->right = z;
        rbInsertFix11(z);
    }
done:
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit operationCompleted(QStringLiteral("insert"), rbSize11(rbRoot11));
}

/**
 * @brief 查找指定键，返回关联值
 * @param key 待查找的键
 * @return 关联值，未找到返回无效QVariant
 */
QVariant RedBlackTree11::search(int key) const
{
    RBNode11* cur = rbRoot11;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return cur->value;
    }
    return QVariant();
}

/**
 * @brief 删除指定键，删除后自动修复红黑性质
 * @param key 待删除的键
 * @return 是否删除成功
 */
bool RedBlackTree11::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    RBNode11* z = rbRoot11;
    while (z) {
        if (key < z->key) z = z->left;
        else if (key > z->key) z = z->right;
        else break;
    }
    if (!z) {
        qint64 elapsed = timer.elapsed();
        m_timeSum += elapsed; m_stats.totalOperations++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
        emit operationCompleted(QStringLiteral("remove"), rbSize11(rbRoot11));
        return false;
    }

    RBNode11* y = z;
    bool yOrigRed = y->isRed;
    RBNode11* x = nullptr;
    if (!z->left) { x = z->right; rbTransplant11(z, z->right); }
    else if (!z->right) { x = z->left; rbTransplant11(z, z->left); }
    else {
        y = rbMin11(z->right);
        yOrigRed = y->isRed;
        x = y->right;
        if (y->parent == z) { if (x) x->parent = y; }
        else { rbTransplant11(y, y->right); y->right = z->right; y->right->parent = y; }
        rbTransplant11(z, y);
        y->left = z->left; y->left->parent = y;
        y->isRed = z->isRed;
    }
    delete z;
    if (!yOrigRed && x) rbDeleteFix11(x);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit operationCompleted(QStringLiteral("remove"), rbSize11(rbRoot11));
    return true;
}

/**
 * @brief 范围查询，返回[minKey, maxKey]区间内的所有键值对
 * @param minKey 最小键
 * @param maxKey 最大键
 * @return 范围内的键值对列表
 */
QVector<QPair<int, QVariant>> RedBlackTree11::rangeQuery(int minKey, int maxKey) const
{
    QVector<QPair<int, QVariant>> result;
    rbRange11(rbRoot11, minKey, maxKey, result);
    return result;
}

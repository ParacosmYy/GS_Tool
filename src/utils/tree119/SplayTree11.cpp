#include "SplayTree11.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 伸展树节点
 */
struct SplayNode11 {
    int key;
    double value;
    SplayNode11* left;
    SplayNode11* right;
    SplayNode11* parent;
    explicit SplayNode11(int k, double v)
        : key(k), value(v), left(nullptr), right(nullptr), parent(nullptr) {}
};

static SplayNode11* spRoot11 = nullptr;

static void spRotR11(SplayNode11* x)
{
    SplayNode11* y = x->left;
    x->left = y->right;
    if (y->right) y->right->parent = x;
    y->parent = x->parent;
    if (!x->parent) spRoot11 = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->right = x; x->parent = y;
}

static void spRotL11(SplayNode11* x)
{
    SplayNode11* y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent) spRoot11 = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x; x->parent = y;
}

static void splay11(SplayNode11* x)
{
    while (x->parent) {
        if (!x->parent->parent) {
            if (x == x->parent->left) spRotR11(x->parent);
            else spRotL11(x->parent);
        } else if (x == x->parent->left && x->parent == x->parent->parent->left) {
            spRotR11(x->parent->parent); spRotR11(x->parent);
        } else if (x == x->parent->right && x->parent == x->parent->parent->right) {
            spRotL11(x->parent->parent); spRotL11(x->parent);
        } else if (x == x->parent->left) {
            spRotR11(x->parent); spRotL11(x->parent);
        } else {
            spRotL11(x->parent); spRotR11(x->parent);
        }
    }
}

static int spSz11(SplayNode11* n) { return n ? 1 + spSz11(n->left) + spSz11(n->right) : 0; }

static void spRange11(SplayNode11* n, int lo, int hi, QVector<QPair<int, double>>& out)
{
    if (!n) return;
    if (lo < n->key) spRange11(n->left, lo, hi, out);
    if (lo <= n->key && n->key <= hi) out.append({n->key, n->value});
    if (hi > n->key) spRange11(n->right, lo, hi, out);
}

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
SplayTree11::SplayTree11(QObject* parent) : QObject(parent) {}

/**
 * @brief 重置所有统计信息
 */
void SplayTree11::resetStatistics() { m_stats = Stats(); m_timeSum = 0.0; }

/**
 * @brief 插入键值对
 * @param key 键
 * @param value 值
 */
void SplayTree11::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (!spRoot11) {
        spRoot11 = new SplayNode11(key, value);
    } else {
        SplayNode11* cur = spRoot11;
        SplayNode11* par = nullptr;
        while (cur) {
            par = cur;
            if (key < cur->key) cur = cur->left;
            else if (key > cur->key) cur = cur->right;
            else { cur->value = value; splay11(cur); goto done; }
        }
        SplayNode11* node = new SplayNode11(key, value);
        node->parent = par;
        if (key < par->key) par->left = node;
        else par->right = node;
        splay11(node);
    }
done:
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTreeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTreeOps;
    emit treeOperationCompleted(spSz11(spRoot11));
}

/**
 * @brief 查找指定键并伸展至根
 * @param key 待查找的键
 * @return 键对应的值，未找到返回0.0
 */
double SplayTree11::search(int key)
{
    QElapsedTimer timer;
    timer.start();

    double result = 0.0;
    SplayNode11* cur = spRoot11;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else { splay11(cur); result = cur->value; break; }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTreeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTreeOps;
    emit treeOperationCompleted(spSz11(spRoot11));
    return result;
}

/**
 * @brief 删除指定键
 * @param key 待删除的键
 * @return 是否删除成功
 */
bool SplayTree11::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    SplayNode11* cur = spRoot11;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else break;
    }
    if (!cur) {
        qint64 elapsed = timer.elapsed();
        m_timeSum += elapsed; m_stats.totalTreeOps++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTreeOps;
        emit treeOperationCompleted(spSz11(spRoot11));
        return false;
    }

    splay11(cur);
    SplayNode11* lt = cur->left;
    SplayNode11* rt = cur->right;
    if (lt) lt->parent = nullptr;
    if (rt) rt->parent = nullptr;
    delete cur;

    if (!lt) { spRoot11 = rt; }
    else if (!rt) { spRoot11 = lt; }
    else {
        SplayNode11* mx = lt;
        while (mx->right) mx = mx->right;
        splay11(mx);
        /* mx现在是lt的根且没有右子树 */
        /* 重新获取lt根 */
        spRoot11 = mx;
        mx->right = rt;
        rt->parent = mx;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTreeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTreeOps;
    emit treeOperationCompleted(spSz11(spRoot11));
    return true;
}

/**
 * @brief 查找区间 [minKey, maxKey] 内的所有键值对
 * @param minKey 区间左端
 * @param maxKey 区间右端
 * @return 范围内的键值对列表
 */
QVector<QPair<int, double>> SplayTree11::rangeQuery(int minKey, int maxKey)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<int, double>> result;
    spRange11(spRoot11, minKey, maxKey, result);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTreeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTreeOps;
    emit treeOperationCompleted(spSz11(spRoot11));
    return result;
}

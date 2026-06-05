#include "SplayTree10.h"
#include <QElapsedTimer>
#include <QVariant>
#include <QRandomGenerator>
#include <QtMath>
#include <algorithm>

/**
 * @brief 伸展树节点
 */
struct SplayNode10 {
    int key;
    QVariant value;
    SplayNode10* left;
    SplayNode10* right;
    SplayNode10* parent;
    explicit SplayNode10(int k, const QVariant& v)
        : key(k), value(v), left(nullptr), right(nullptr), parent(nullptr) {}
};

static SplayNode10* spRoot10 = nullptr;

/* 右旋转 */
static void spRotR10(SplayNode10* x)
{
    SplayNode10* y = x->left;
    x->left = y->right;
    if (y->right) y->right->parent = x;
    y->parent = x->parent;
    if (!x->parent) spRoot10 = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->right = x; x->parent = y;
}

/* 左旋转 */
static void spRotL10(SplayNode10* x)
{
    SplayNode10* y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent) spRoot10 = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x; x->parent = y;
}

/* 伸展操作：将节点旋转至根 */
static void splay10(SplayNode10* x)
{
    while (x->parent) {
        if (!x->parent->parent) {
            if (x == x->parent->left) spRotR10(x->parent);
            else spRotL10(x->parent);
        } else if (x == x->parent->left && x->parent == x->parent->parent->left) {
            spRotR10(x->parent->parent); spRotR10(x->parent);
        } else if (x == x->parent->right && x->parent == x->parent->parent->right) {
            spRotL10(x->parent->parent); spRotL10(x->parent);
        } else if (x == x->parent->left) {
            spRotR10(x->parent); spRotL10(x->parent);
        } else {
            spRotL10(x->parent); spRotR10(x->parent);
        }
    }
}

/* 计算子树大小 */
static int spSz10(SplayNode10* n) { return n ? 1 + spSz10(n->left) + spSz10(n->right) : 0; }

/* 中序遍历辅助 */
static void spInOrder10(SplayNode10* n, QVector<QPair<int, QVariant>>& result)
{
    if (!n) return;
    spInOrder10(n->left, result);
    result.append({n->key, n->value});
    spInOrder10(n->right, result);
}

/**
 * @brief 构造函数，初始化伸展树
 * @param parent 父对象指针
 */
SplayTree10::SplayTree10(QObject* parent) : QObject(parent) {}

/**
 * @brief 重置所有统计信息
 */
void SplayTree10::resetStatistics() { m_stats = Stats(); m_timeSum = 0.0; }

/**
 * @brief 插入键值对，插入后自动将节点伸展至根
 * @param key 键
 * @param value 值
 */
void SplayTree10::insert(int key, const QVariant& value)
{
    QElapsedTimer timer;
    timer.start();

    if (!spRoot10) {
        spRoot10 = new SplayNode10(key, value);
    } else {
        SplayNode10* cur = spRoot10;
        SplayNode10* par = nullptr;
        while (cur) {
            par = cur;
            if (key < cur->key) cur = cur->left;
            else if (key > cur->key) cur = cur->right;
            else { cur->value = value; splay10(cur); goto done; }
        }
        SplayNode10* node = new SplayNode10(key, value);
        node->parent = par;
        if (key < par->key) par->left = node;
        else par->right = node;
        splay10(node);
    }
done:
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit operationCompleted(QStringLiteral("insert"), spSz10(spRoot10));
}

/**
 * @brief 查找指定键，找到后伸展至根并返回值
 * @param key 待查找的键
 * @return 关联值，未找到返回无效QVariant
 */
QVariant SplayTree10::search(int key)
{
    QElapsedTimer timer;
    timer.start();

    QVariant result;
    SplayNode10* cur = spRoot10;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else { splay10(cur); result = cur->value; break; }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit operationCompleted(QStringLiteral("search"), spSz10(spRoot10));
    return result;
}

/**
 * @brief 删除指定键，删除后合并左右子树
 * @param key 待删除的键
 * @return 是否删除成功
 */
bool SplayTree10::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    SplayNode10* cur = spRoot10;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else break;
    }
    if (!cur) {
        qint64 elapsed = timer.elapsed();
        m_timeSum += elapsed; m_stats.totalOperations++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
        emit operationCompleted(QStringLiteral("remove"), spSz10(spRoot10));
        return false;
    }

    splay10(cur);
    SplayNode10* lt = cur->left;
    SplayNode10* rt = cur->right;
    if (lt) lt->parent = nullptr;
    if (rt) rt->parent = nullptr;
    delete cur;

    if (!lt) { spRoot10 = rt; }
    else if (!rt) { spRoot10 = lt; }
    else {
        SplayNode10* mx = lt;
        while (mx->right) mx = mx->right;
        splay10(mx);
        spRoot10 = mx;
        mx->right = rt;
        rt->parent = mx;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit operationCompleted(QStringLiteral("remove"), spSz10(spRoot10));
    return true;
}

/**
 * @brief 中序遍历返回所有键值对，用于调试和序列化
 * @return 有序键值对列表
 */
QVector<QPair<int, QVariant>> SplayTree10::inOrderTraversal() const
{
    QVector<QPair<int, QVariant>> result;
    spInOrder10(spRoot10, result);
    return result;
}

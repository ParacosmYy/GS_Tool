#include "Treap11.h"
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QtMath>
#include <algorithm>

/**
 * @brief Treap节点
 */
struct TreapNode11 {
    int key;
    double value;
    int priority;
    TreapNode11* left;
    TreapNode11* right;
    explicit TreapNode11(int k, double v, int prio)
        : key(k), value(v), priority(prio), left(nullptr), right(nullptr) {}
};

static TreapNode11* tpRoot11 = nullptr;

static TreapNode11* tpRotR11(TreapNode11* y)
{
    TreapNode11* x = y->left;
    y->left = x->right;
    x->right = y;
    return x;
}

static TreapNode11* tpRotL11(TreapNode11* x)
{
    TreapNode11* y = x->right;
    x->right = y->left;
    y->left = x;
    return y;
}

static TreapNode11* tpInsert11(TreapNode11* n, int key, double value)
{
    if (!n) return new TreapNode11(key, value, QRandomGenerator::global()->bounded(100000));
    if (key < n->key) {
        n->left = tpInsert11(n->left, key, value);
        if (n->left->priority > n->priority) n = tpRotR11(n);
    } else if (key > n->key) {
        n->right = tpInsert11(n->right, key, value);
        if (n->right->priority > n->priority) n = tpRotL11(n);
    } else { n->value = value; }
    return n;
}

static TreapNode11* tpDelete11(TreapNode11* n, int key, bool& found)
{
    if (!n) { found = false; return nullptr; }
    if (key < n->key) { n->left = tpDelete11(n->left, key, found); }
    else if (key > n->key) { n->right = tpDelete11(n->right, key, found); }
    else {
        found = true;
        if (!n->left || !n->right) {
            TreapNode11* tmp = n->left ? n->left : n->right;
            delete n; return tmp;
        }
        if (n->left->priority > n->right->priority) {
            n = tpRotR11(n); n->right = tpDelete11(n->right, key, found);
        } else {
            n = tpRotL11(n); n->left = tpDelete11(n->left, key, found);
        }
    }
    return n;
}

static int tpSz11(TreapNode11* n) { return n ? 1 + tpSz11(n->left) + tpSz11(n->right) : 0; }

static void tpInOrder11(TreapNode11* n, QVector<QPair<int, double>>& out)
{
    if (!n) return;
    tpInOrder11(n->left, out);
    out.append({n->key, n->value});
    tpInOrder11(n->right, out);
}

static void tpSplitCollect11(TreapNode11* n, int splitKey,
    QVector<QPair<int, double>>& le, QVector<QPair<int, double>>& gt)
{
    if (!n) return;
    tpSplitCollect11(n->left, splitKey, le, gt);
    if (n->key <= splitKey) le.append({n->key, n->value});
    else gt.append({n->key, n->value});
    tpSplitCollect11(n->right, splitKey, le, gt);
}

/* 中序第k个 */
static bool tpKth11(TreapNode11* n, int k, int& cnt, QPair<int, double>& result)
{
    if (!n) return false;
    if (tpKth11(n->left, k, cnt, result)) return true;
    ++cnt;
    if (cnt == k) { result = {n->key, n->value}; return true; }
    return tpKth11(n->right, k, cnt, result);
}

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
Treap11::Treap11(QObject* parent) : QObject(parent) {}

/**
 * @brief 重置所有统计信息
 */
void Treap11::resetStatistics() { m_stats = Stats(); m_timeSum = 0.0; }

/**
 * @brief 插入键值对（随机优先级）
 * @param key 键
 * @param value 值
 */
void Treap11::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();
    tpRoot11 = tpInsert11(tpRoot11, key, value);
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTreeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTreeOps;
    emit treeOperationCompleted(tpSz11(tpRoot11));
}

/**
 * @brief 删除指定键
 * @param key 待删除的键
 * @return 是否删除成功
 */
bool Treap11::remove(int key)
{
    QElapsedTimer timer;
    timer.start();
    bool found = false;
    tpRoot11 = tpDelete11(tpRoot11, key, found);
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTreeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTreeOps;
    emit treeOperationCompleted(tpSz11(tpRoot11));
    return found;
}

/**
 * @brief 查找指定键对应的值
 * @param key 待查找的键
 * @return 键对应的值，未找到返回0.0
 */
double Treap11::search(int key) const
{
    TreapNode11* cur = tpRoot11;
    while (cur) { if (key < cur->key) cur = cur->left; else if (key > cur->key) cur = cur->right; else return cur->value; }
    return 0.0;
}

/**
 * @brief 按键值分裂为两棵树
 * @param splitKey 分裂键值
 * @return 两棵子树的键值对 (≤splitKey, >splitKey)
 */
QPair<QVector<QPair<int, double>>, QVector<QPair<int, double>>> Treap11::split(int splitKey)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<int, double>> le, gt;
    tpSplitCollect11(tpRoot11, splitKey, le, gt);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTreeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTreeOps;
    emit treeOperationCompleted(tpSz11(tpRoot11));
    return {le, gt};
}

/**
 * @brief 获取第k小的键值对
 * @param k 排名（1-based）
 * @return 第k小的键值对
 */
QPair<int, double> Treap11::kthElement(int k) const
{
    QPair<int, double> result = {-1, 0.0};
    int cnt = 0;
    tpKth11(tpRoot11, k, cnt, result);
    return result;
}

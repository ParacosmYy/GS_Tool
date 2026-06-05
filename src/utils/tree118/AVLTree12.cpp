#include "AVLTree12.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief AVL树节点
 */
struct AVLNode12 {
    int key;
    double value;
    int height;
    AVLNode12* left;
    AVLNode12* right;
    explicit AVLNode12(int k, double v)
        : key(k), value(v), height(1), left(nullptr), right(nullptr) {}
};

static AVLNode12* avlRoot12 = nullptr;

static int avlH12(AVLNode12* n) { return n ? n->height : 0; }
static int avlBal12(AVLNode12* n) { return n ? avlH12(n->left) - avlH12(n->right) : 0; }
static void avlUpd12(AVLNode12* n) { if (n) n->height = 1 + qMax(avlH12(n->left), avlH12(n->right)); }

static AVLNode12* avlRotR12(AVLNode12* y)
{
    AVLNode12* x = y->left;
    y->left = x->right;
    x->right = y;
    avlUpd12(y); avlUpd12(x);
    return x;
}

static AVLNode12* avlRotL12(AVLNode12* x)
{
    AVLNode12* y = x->right;
    x->right = y->left;
    y->left = x;
    avlUpd12(x); avlUpd12(y);
    return y;
}

static AVLNode12* avlBalNode12(AVLNode12* n)
{
    avlUpd12(n);
    int bal = avlBal12(n);
    if (bal > 1) {
        if (avlBal12(n->left) < 0) n->left = avlRotL12(n->left);
        return avlRotR12(n);
    }
    if (bal < -1) {
        if (avlBal12(n->right) > 0) n->right = avlRotR12(n->right);
        return avlRotL12(n);
    }
    return n;
}

static AVLNode12* avlInsert12(AVLNode12* n, int key, double value)
{
    if (!n) return new AVLNode12(key, value);
    if (key < n->key) n->left = avlInsert12(n->left, key, value);
    else if (key > n->key) n->right = avlInsert12(n->right, key, value);
    else { n->value = value; return n; }
    return avlBalNode12(n);
}

static AVLNode12* avlMin12(AVLNode12* n) { while (n && n->left) n = n->left; return n; }

static AVLNode12* avlRemove12(AVLNode12* n, int key, bool& found)
{
    if (!n) { found = false; return nullptr; }
    if (key < n->key) n->left = avlRemove12(n->left, key, found);
    else if (key > n->key) n->right = avlRemove12(n->right, key, found);
    else {
        found = true;
        if (!n->left || !n->right) {
            AVLNode12* tmp = n->left ? n->left : n->right;
            delete n;
            return tmp;
        }
        AVLNode12* succ = avlMin12(n->right);
        n->key = succ->key; n->value = succ->value;
        bool dummy = true;
        n->right = avlRemove12(n->right, succ->key, dummy);
    }
    return avlBalNode12(n);
}

static int avlSize12(AVLNode12* n) { return n ? 1 + avlSize12(n->left) + avlSize12(n->right) : 0; }

static void avlRange12(AVLNode12* n, int lo, int hi, QVector<QPair<int, double>>& out)
{
    if (!n) return;
    if (lo < n->key) avlRange12(n->left, lo, hi, out);
    if (lo <= n->key && n->key <= hi) out.append({n->key, n->value});
    if (hi > n->key) avlRange12(n->right, lo, hi, out);
}

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
AVLTree12::AVLTree12(QObject* parent) : QObject(parent) {}

/**
 * @brief 重置所有统计信息
 */
void AVLTree12::resetStatistics() { m_stats = Stats(); m_timeSum = 0.0; }

/**
 * @brief 插入键值对
 * @param key 键
 * @param value 值
 */
void AVLTree12::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();
    avlRoot12 = avlInsert12(avlRoot12, key, value);
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTreeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTreeOps;
    emit treeOperationCompleted(avlSize12(avlRoot12));
}

/**
 * @brief 删除指定键
 * @param key 待删除的键
 * @return 是否删除成功
 */
bool AVLTree12::remove(int key)
{
    QElapsedTimer timer;
    timer.start();
    bool found = false;
    avlRoot12 = avlRemove12(avlRoot12, key, found);
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTreeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTreeOps;
    emit treeOperationCompleted(avlSize12(avlRoot12));
    return found;
}

/**
 * @brief 查找指定键对应的值
 * @param key 待查找的键
 * @return 键对应的值，未找到返回0.0
 */
double AVLTree12::search(int key) const
{
    AVLNode12* cur = avlRoot12;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return cur->value;
    }
    return 0.0;
}

/**
 * @brief 范围查询 [minKey, maxKey] 内的所有键值对
 * @param minKey 最小键
 * @param maxKey 最大键
 * @return 范围内的键值对列表
 */
QVector<QPair<int, double>> AVLTree12::rangeQuery(int minKey, int maxKey) const
{
    QVector<QPair<int, double>> result;
    avlRange12(avlRoot12, minKey, maxKey, result);
    return result;
}

/**
 * @brief 获取树的节点总数
 * @return 节点数量
 */
int AVLTree12::size() const { return avlSize12(avlRoot12); }

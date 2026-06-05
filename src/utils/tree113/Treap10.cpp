#include "Treap10.h"
#include <QElapsedTimer>
#include <QVariant>
#include <QRandomGenerator>
#include <QtMath>
#include <algorithm>

/**
 * @brief Treap节点
 */
struct TreapNode10 {
    int key;
    QVariant value;
    int priority;
    TreapNode10* left;
    TreapNode10* right;
    explicit TreapNode10(int k, const QVariant& v, int prio)
        : key(k), value(v), priority(prio), left(nullptr), right(nullptr) {}
};

static TreapNode10* treapRoot10 = nullptr;

/* 右旋转 */
static TreapNode10* treapRotRight10(TreapNode10* y)
{
    TreapNode10* x = y->left;
    y->left = x->right;
    x->right = y;
    return x;
}

/* 左旋转 */
static TreapNode10* treapRotLeft10(TreapNode10* x)
{
    TreapNode10* y = x->right;
    x->right = y->left;
    y->left = x;
    return y;
}

/* 递归插入 */
static TreapNode10* treapInsert10(TreapNode10* root, int key, const QVariant& value)
{
    if (!root) return new TreapNode10(key, value, QRandomGenerator::global()->bounded(100000));
    if (key < root->key) {
        root->left = treapInsert10(root->left, key, value);
        if (root->left->priority > root->priority) root = treapRotRight10(root);
    } else if (key > root->key) {
        root->right = treapInsert10(root->right, key, value);
        if (root->right->priority > root->priority) root = treapRotLeft10(root);
    } else {
        root->value = value;
    }
    return root;
}

/* 递归删除 */
static TreapNode10* treapDelete10(TreapNode10* root, int key, bool& found)
{
    if (!root) { found = false; return nullptr; }
    if (key < root->key) {
        root->left = treapDelete10(root->left, key, found);
    } else if (key > root->key) {
        root->right = treapDelete10(root->right, key, found);
    } else {
        found = true;
        if (!root->left || !root->right) {
            TreapNode10* tmp = root->left ? root->left : root->right;
            delete root;
            return tmp;
        }
        if (root->left->priority > root->right->priority) {
            root = treapRotRight10(root);
            root->right = treapDelete10(root->right, key, found);
        } else {
            root = treapRotLeft10(root);
            root->left = treapDelete10(root->left, key, found);
        }
    }
    return root;
}

/* 中序收集 */
static void treapInOrder10(TreapNode10* n, QVector<QPair<int, QVariant>>& out)
{
    if (!n) return;
    treapInOrder10(n->left, out);
    out.append({n->key, n->value});
    treapInOrder10(n->right, out);
}

/* 按键分裂 */
static void treapSplit10(TreapNode10* n, int splitKey,
    QVector<QPair<int, QVariant>>& left, QVector<QPair<int, QVariant>>& right)
{
    if (!n) return;
    treapSplit10(n->left, splitKey, left, right);
    if (n->key <= splitKey) left.append({n->key, n->value});
    else right.append({n->key, n->value});
    treapSplit10(n->right, splitKey, left, right);
}

/* 子树大小 */
static int treapSz10(TreapNode10* n) { return n ? 1 + treapSz10(n->left) + treapSz10(n->right) : 0; }

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
Treap10::Treap10(QObject* parent) : QObject(parent) {}

/**
 * @brief 重置所有统计信息
 */
void Treap10::resetStatistics() { m_stats = Stats(); m_timeSum = 0.0; }

/**
 * @brief 插入键值对，随机优先级自动维持平衡
 * @param key 键
 * @param value 值
 */
void Treap10::insert(int key, const QVariant& value)
{
    QElapsedTimer timer;
    timer.start();

    treapRoot10 = treapInsert10(treapRoot10, key, value);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit operationCompleted(QStringLiteral("insert"), treapSz10(treapRoot10));
}

/**
 * @brief 查找指定键并返回关联值
 * @param key 待查找的键
 * @return 关联值，未找到返回无效QVariant
 */
QVariant Treap10::search(int key) const
{
    TreapNode10* cur = treapRoot10;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return cur->value;
    }
    return QVariant();
}

/**
 * @brief 删除指定键并保持Treap性质
 * @param key 待删除的键
 * @return 是否删除成功
 */
bool Treap10::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    bool found = false;
    treapRoot10 = treapDelete10(treapRoot10, key, found);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit operationCompleted(QStringLiteral("remove"), treapSz10(treapRoot10));
    return found;
}

/**
 * @brief 按序分裂为两棵Treap：键<=splitKey和键>splitKey
 * @param splitKey 分裂键值
 * @return 两棵子树的中序遍历结果
 */
QPair<QVector<QPair<int, QVariant>>, QVector<QPair<int, QVariant>>> Treap10::split(int splitKey)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<int, QVariant>> left, right;
    treapSplit10(treapRoot10, splitKey, left, right);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit operationCompleted(QStringLiteral("split"), treapSz10(treapRoot10));
    return {left, right};
}

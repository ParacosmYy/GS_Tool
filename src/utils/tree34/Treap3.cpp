/**
 * @file Treap3.cpp
 * @brief Treap增强实现 — 顺序统计/区间加/区间求和/懒标记下推
 */

#include "utils/tree34/Treap3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <random>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
Treap3::Treap3(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_size(0)
{
}

/** @brief 析构函数 — 释放所有节点 */
Treap3::~Treap3()
{
    destroyTree(m_root);
    m_root = nullptr;
}

/** @brief 插入键值对 @param key 键 @param value 值 */
void Treap3::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    m_root = Treap3::insert(m_root, key, value);
    ++m_size;

    ++m_stats.totalInsertions;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalInsertions
            + m_stats.totalDeletions + m_stats.totalQueries));

    emit inserted(key);
}

/** @brief 删除键 @param key 键 @return 是否成功 */
bool Treap3::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) return false;

    /* 先检查key是否存在 */
    Node* cur = m_root;
    while (cur) {
        pushDown(cur);
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else break;
    }
    if (!cur) return false;

    m_root = Treap3::remove(m_root, key);
    --m_size;

    ++m_stats.totalDeletions;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalInsertions
            + m_stats.totalDeletions + m_stats.totalQueries));

    emit removed(key);
    return true;
}

/** @brief 查找键对应的值 @param key 键 @return 值(未找到返回0) */
int Treap3::find(double key) const
{
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return cur->value;
    }
    return 0;
}

/** @brief 选择第k小的键 @param k 排名(0-based) @return 键值 */
double Treap3::selectKth(int k) const
{
    if (k < 0 || k >= m_size) return 0.0;

    Node* cur = m_root;
    while (cur) {
        /* 注意: const方法中调用非const的pushDown */
        int leftSz = cur->left ? cur->left->size : 0;
        if (k < leftSz) {
            cur = cur->left;
        } else if (k == leftSz) {
            return cur->key;
        } else {
            k -= leftSz + 1;
            cur = cur->right;
        }
    }
    return 0.0;
}

/** @brief 计算键的排名 @param key 键 @return 排名(0-based) */
int Treap3::rank(double key) const
{
    int r = 0;
    Node* cur = m_root;
    while (cur) {
        int leftSz = cur->left ? cur->left->size : 0;
        if (key < cur->key) {
            cur = cur->left;
        } else if (key > cur->key) {
            r += leftSz + 1;
            cur = cur->right;
        } else {
            r += leftSz;
            return r;
        }
    }
    return r;
}

/** @brief 区间求和[lo, hi] @param lo 下界 @param hi 上界 @return 和 */
double Treap3::rangeSum(double lo, double hi) const
{
    if (lo > hi || !m_root) return 0.0;
    /* sum(hi的rank) - sum(lo-1的rank) 通过分裂实现 */
    /* 简化实现: 中序遍历累加 */
    double sum = 0.0;
    rangeSumHelper(m_root, lo, hi, sum);
    return sum;
}

/** @brief 区间加[lo, hi] @param lo 下界 @param hi 上界 @param delta 增量 */
void Treap3::rangeAdd(double lo, double hi, double delta)
{
    if (lo > hi || !m_root) return;
    rangeAddHelper(m_root, lo, hi, delta);
    pullUp(m_root);
}

/** @brief 清空树 */
void Treap3::clear()
{
    destroyTree(m_root);
    m_root = nullptr;
    m_size = 0;
}

/** @brief 中序遍历 @return 排序后的键值对列表 */
QVector<QPair<double, int>> Treap3::inOrderTraversal() const
{
    QVector<QPair<double, int>> result;
    result.reserve(m_size);
    inOrderCollect(m_root, result);
    return result;
}

/** @brief 懒标记下推 @param n 节点 */
void Treap3::pushDown(Node* n)
{
    if (!n || qFabs(n->lazy) < 1e-15) return;

    /* 将lazy标记下推到子节点 */
    if (n->left) {
        n->left->key += n->lazy;
        n->left->lazy += n->lazy;
    }
    if (n->right) {
        n->right->key += n->lazy;
        n->right->lazy += n->lazy;
    }
    n->lazy = 0.0;
}

/** @brief 向上更新聚合信息 @param n 节点 */
void Treap3::pullUp(Node* n)
{
    if (!n) return;
    n->size = 1;
    n->sum = n->key;
    if (n->left) {
        n->size += n->left->size;
        n->sum += n->left->sum;
    }
    if (n->right) {
        n->size += n->right->size;
        n->sum += n->right->sum;
    }
}

/** @brief 右旋转 @param y 旋转根 @return 新根 */
Treap3::Node* Treap3::rotateRight(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    x->right = y;
    pullUp(y);
    pullUp(x);
    return x;
}

/** @brief 左旋转 @param x 旋转根 @return 新根 */
Treap3::Node* Treap3::rotateLeft(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    y->left = x;
    pullUp(x);
    pullUp(y);
    return y;
}

/** @brief 递归插入 @param root 当前根 @param key 键 @param value 值 @return 新根 */
Treap3::Node* Treap3::insert(Node* root, double key, int value)
{
    if (!root) {
        /* 创建新节点 */
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<int> dist(1, 100000000);

        Node* n = new Node();
        n->key = key;
        n->value = value;
        n->priority = dist(gen);
        n->size = 1;
        n->sum = key;
        n->lazy = 0.0;
        n->left = nullptr;
        n->right = nullptr;
        return n;
    }

    pushDown(root);

    if (key < root->key) {
        root->left = insert(root->left, key, value);
        if (root->left && root->left->priority > root->priority) {
            root = rotateRight(root);
        }
    } else {
        root->right = insert(root->right, key, value);
        if (root->right && root->right->priority > root->priority) {
            root = rotateLeft(root);
        }
    }

    pullUp(root);
    return root;
}

/** @brief 递归删除 @param root 当前根 @param key 键 @return 新根 */
Treap3::Node* Treap3::remove(Node* root, double key)
{
    if (!root) return nullptr;

    pushDown(root);

    if (key < root->key) {
        root->left = remove(root->left, key);
    } else if (key > root->key) {
        root->right = remove(root->right, key);
    } else {
        /* 找到目标节点 */
        if (!root->left && !root->right) {
            delete root;
            return nullptr;
        }
        if (!root->left) {
            Node* r = root->right;
            root->right = nullptr;
            delete root;
            return r;
        }
        if (!root->right) {
            Node* l = root->left;
            root->left = nullptr;
            delete root;
            return l;
        }
        /* 两个子节点都存在: 按优先级旋转 */
        if (root->left->priority > root->right->priority) {
            root = rotateRight(root);
            root->right = remove(root->right, key);
        } else {
            root = rotateLeft(root);
            root->left = remove(root->left, key);
        }
    }

    pullUp(root);
    return root;
}

/** @brief 递归销毁子树 @param n 根节点 */
void Treap3::destroyTree(Node* n)
{
    if (!n) return;
    destroyTree(n->left);
    destroyTree(n->right);
    delete n;
}

/** @brief 区间求和辅助: 中序遍历并累加范围内的键 @param n 节点 @param lo 下界 @param hi 上界 @param sum 累加引用 */
void Treap3::rangeSumHelper(Node* n, double lo, double hi, double& sum) const
{
    if (!n) return;
    if (n->key > lo) rangeSumHelper(n->left, lo, hi, sum);
    if (n->key >= lo && n->key <= hi) sum += n->key;
    if (n->key < hi) rangeSumHelper(n->right, lo, hi, sum);
}

/** @brief 区间加辅助: 给范围内的节点键加上delta @param n 节点 @param lo 下界 @param hi 上界 @param delta 增量 */
void Treap3::rangeAddHelper(Node* n, double lo, double hi, double delta)
{
    if (!n) return;
    if (n->key > lo) rangeAddHelper(n->left, lo, hi, delta);
    if (n->key >= lo && n->key <= hi) n->key += delta;
    if (n->key < hi) rangeAddHelper(n->right, lo, hi, delta);
}

/** @brief 中序遍历收集 @param n 节点 @param result 输出列表 */
void Treap3::inOrderCollect(Node* n, QVector<QPair<double, int>>& result) const
{
    if (!n) return;
    inOrderCollect(n->left, result);
    result.append(qMakePair(n->key, n->value));
    inOrderCollect(n->right, result);
}

/** @brief 重置统计 */
void Treap3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

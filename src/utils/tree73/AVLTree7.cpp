/**
 * @file AVLTree7.cpp
 * @brief AVL平衡搜索树实现
 *
 * 实现支持插入、删除、排名查询和范围查询的AVL树，
 * 通过旋转保持树的高度平衡。
 */

#include "utils/tree73/AVLTree7.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
AVLTree7::AVLTree7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 插入键值对
 * @param key 键
 * @param value 值
 */
void AVLTree7::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertNode(m_root, key, value);
    m_size++;

    qint64 elapsed = timer.elapsed();
    m_stats.totalInserts++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);

    emit inserted(key);
}

/**
 * @brief 删除键
 * @param key 要删除的键
 */
void AVLTree7::remove(double key)
{
    m_root = removeNode(m_root, key);
    if (m_root != nullptr) m_size--;
}

/**
 * @brief 检查是否包含指定键
 * @param key 待查找的键
 * @return 是否存在
 */
bool AVLTree7::contains(double key) const
{
    AVLNode* cur = m_root;
    while (cur != nullptr) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return true;
    }
    return false;
}

/**
 * @brief 计算键的排名
 * @param key 待查询的键
 * @return 排名（小于key的元素数量）
 */
int AVLTree7::rank(double key) const
{
    QElapsedTimer timer;
    timer.start();

    int r = 0;
    AVLNode* cur = m_root;
    while (cur != nullptr) {
        if (key < cur->key) {
            cur = cur->left;
        } else if (key > cur->key) {
            r += nodeCount(cur->left) + 1;
            cur = cur->right;
        } else {
            r += nodeCount(cur->left);
            break;
        }
    }

    const_cast<AVLTree7*>(this)->m_stats.totalQueries++;
    const_cast<AVLTree7*>(this)->m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);

    return r;
}

/**
 * @brief 选择第k小的元素
 * @param k 排名（0-indexed）
 * @return 第k小元素的键
 */
double AVLTree7::select(int k) const
{
    if (k < 0 || k >= m_size) return 0.0;

    AVLNode* cur = m_root;
    while (cur != nullptr) {
        int leftSize = nodeCount(cur->left);
        if (k < leftSize) {
            cur = cur->left;
        } else if (k > leftSize) {
            k -= leftSize + 1;
            cur = cur->right;
        } else {
            return cur->key;
        }
    }
    return 0.0;
}

/**
 * @brief 范围查询
 * @param lo 下界
 * @param hi 上界
 * @return 范围内所有值
 */
QVector<int> AVLTree7::rangeQuery(double lo, double hi) const
{
    QVector<int> result;
    rangeQueryNode(m_root, lo, hi, result);
    return result;
}

/**
 * @brief 重置统计信息
 */
void AVLTree7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 递归插入节点
 * @param n 当前子树根
 * @param key 键
 * @param val 值
 * @return 新的子树根
 */
AVLTree7::AVLNode* AVLTree7::insertNode(AVLNode* n, double key, int val)
{
    if (n == nullptr) {
        return new AVLNode{key, val, 1, 1, nullptr, nullptr};
    }

    if (key < n->key) {
        n->left = insertNode(n->left, key, val);
    } else if (key > n->key) {
        n->right = insertNode(n->right, key, val);
    } else {
        n->val = val;
        return n;
    }

    // 更新高度和计数
    n->height = 1 + qMax(nodeHeight(n->left), nodeHeight(n->right));
    n->count = 1 + nodeCount(n->left) + nodeCount(n->right);

    return balance(n);
}

/**
 * @brief 递归删除节点
 * @param n 当前子树根
 * @param key 要删除的键
 * @return 新的子树根
 */
AVLTree7::AVLNode* AVLTree7::removeNode(AVLNode* n, double key)
{
    if (n == nullptr) return nullptr;

    if (key < n->key) {
        n->left = removeNode(n->left, key);
    } else if (key > n->key) {
        n->right = removeNode(n->right, key);
    } else {
        if (n->left == nullptr || n->right == nullptr) {
            AVLNode* child = (n->left != nullptr) ? n->left : n->right;
            delete n;
            return child;
        }
        // 找中序后继
        AVLNode* succ = n->right;
        while (succ->left != nullptr) succ = succ->left;
        n->key = succ->key;
        n->val = succ->val;
        n->right = removeNode(n->right, succ->key);
    }

    n->height = 1 + qMax(nodeHeight(n->left), nodeHeight(n->right));
    n->count = 1 + nodeCount(n->left) + nodeCount(n->right);

    return balance(n);
}

/**
 * @brief 平衡节点
 * @param n 待平衡节点
 * @return 平衡后的子树根
 */
AVLTree7::AVLNode* AVLTree7::balance(AVLNode* n)
{
    int bf = balanceFactor(n);

    if (bf > 1) {
        if (balanceFactor(n->left) < 0) n->left = rotateLeft(n->left);
        return rotateRight(n);
    }
    if (bf < -1) {
        if (balanceFactor(n->right) > 0) n->right = rotateRight(n->right);
        return rotateLeft(n);
    }

    return n;
}

/**
 * @brief 获取节点高度
 * @param n 节点指针
 * @return 节点高度，空节点返回0
 */
int AVLTree7::nodeHeight(AVLNode* n) const
{
    return (n != nullptr) ? n->height : 0;
}

/**
 * @brief 计算平衡因子
 * @param n 节点指针
 * @return 平衡因子（左子树高-右子树高）
 */
int AVLTree7::balanceFactor(AVLNode* n) const
{
    return (n != nullptr) ? nodeHeight(n->left) - nodeHeight(n->right) : 0;
}

/**
 * @brief 左旋
 * @param n 旋转节点
 * @return 旋转后的新根
 */
AVLTree7::AVLNode* AVLTree7::rotateLeft(AVLNode* n)
{
    AVLNode* r = n->right;
    n->right = r->left;
    r->left = n;

    n->height = 1 + qMax(nodeHeight(n->left), nodeHeight(n->right));
    r->height = 1 + qMax(nodeHeight(r->left), nodeHeight(r->right));
    n->count = 1 + nodeCount(n->left) + nodeCount(n->right);
    r->count = 1 + nodeCount(r->left) + nodeCount(r->right);

    return r;
}

/**
 * @brief 右旋
 * @param n 旋转节点
 * @return 旋转后的新根
 */
AVLTree7::AVLNode* AVLTree7::rotateRight(AVLNode* n)
{
    AVLNode* l = n->left;
    n->left = l->right;
    l->right = n;

    n->height = 1 + qMax(nodeHeight(n->left), nodeHeight(n->right));
    l->height = 1 + qMax(nodeHeight(l->left), nodeHeight(l->right));
    n->count = 1 + nodeCount(n->left) + nodeCount(n->right);
    l->count = 1 + nodeCount(l->left) + nodeCount(l->right);

    return l;
}

/**
 * @brief 范围查询递归
 * @param n 当前节点
 * @param lo 下界
 * @param hi 上界
 * @param res 结果容器
 */
void AVLTree7::rangeQueryNode(AVLNode* n, double lo, double hi, QVector<int>& res) const
{
    if (n == nullptr) return;
    if (lo < n->key) rangeQueryNode(n->left, lo, hi, res);
    if (lo <= n->key && n->key <= hi) res.append(n->val);
    if (n->key < hi) rangeQueryNode(n->right, lo, hi, res);
}

/**
 * @brief 获取节点子树大小
 */
static int nodeCountHelper(AVLTree7::AVLNode* n) { return (n) ? n->count : 0; }

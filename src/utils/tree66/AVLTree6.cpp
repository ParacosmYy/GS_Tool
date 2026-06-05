/**
 * @file AVLTree6.cpp
 * @brief AVL平衡二叉搜索树实现（第6版）
 *
 * 实现自平衡AVL树，支持插入、删除、查找、排名查询、选择操作
 * 以及范围查询。每个节点维护子树大小(count)以支持顺序统计操作。
 * 所有修改操作后自动进行旋转平衡。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/tree66/AVLTree6.h"

#include <QElapsedTimer>
#include <algorithm>
#include <QStack>

/**
 * @brief 构造函数，初始化AVL树
 * @param parent 父QObject对象指针
 */
AVLTree6::AVLTree6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 获取节点高度
 * @param n 节点指针
 * @return 节点高度，空节点返回0
 */
int AVLTree6::height(AVLNode* n) const
{
    return n ? n->height : 0;
}

/**
 * @brief 计算节点平衡因子
 * @param n 节点指针
 * @return 左子树高度减去右子树高度
 */
int AVLTree6::balanceFactor(AVLNode* n) const
{
    return n ? height(n->left) - height(n->right) : 0;
}

/**
 * @brief 更新节点的height和count
 * @param n 待更新的节点
 */
static void updateNode(AVLTree6::AVLNode* n)
{
    if (!n) return;
    int lh = n->left ? n->left->height : 0;
    int rh = n->right ? n->right->height : 0;
    n->height = 1 + qMax(lh, rh);

    int lc = n->left ? n->left->count : 0;
    int rc = n->right ? n->right->count : 0;
    n->count = 1 + lc + rc;
}

/**
 * @brief 右旋转
 * @param n 不平衡节点
 * @return 旋转后的新根节点
 */
AVLTree6::AVLNode* AVLTree6::rotateRight(AVLNode* n)
{
    AVLNode* newRoot = n->left;
    n->left = newRoot->right;
    newRoot->right = n;
    updateNode(n);
    updateNode(newRoot);
    return newRoot;
}

/**
 * @brief 左旋转
 * @param n 不平衡节点
 * @return 旋转后的新根节点
 */
AVLTree6::AVLNode* AVLTree6::rotateLeft(AVLNode* n)
{
    AVLNode* newRoot = n->right;
    n->right = newRoot->left;
    newRoot->left = n;
    updateNode(n);
    updateNode(newRoot);
    return newRoot;
}

/**
 * @brief 平衡节点
 * @param n 可能不平衡的节点
 * @return 平衡后的节点
 */
AVLTree6::AVLNode* AVLTree6::balance(AVLNode* n)
{
    updateNode(n);
    int bf = balanceFactor(n);

    /* 左重 */
    if (bf > 1) {
        if (balanceFactor(n->left) < 0) {
            n->left = rotateLeft(n->left);
        }
        return rotateRight(n);
    }

    /* 右重 */
    if (bf < -1) {
        if (balanceFactor(n->right) > 0) {
            n->right = rotateRight(n->right);
        }
        return rotateLeft(n);
    }

    return n;
}

/**
 * @brief 递归插入节点
 * @param n 当前子树根节点
 * @param key 插入的键
 * @param val 插入的值
 * @return 插入并平衡后的子树根节点
 */
AVLTree6::AVLNode* AVLTree6::insertNode(AVLNode* n, double key, int val)
{
    if (!n) {
        AVLNode* node = new AVLNode{key, val, 1, 1, nullptr, nullptr};
        return node;
    }

    if (key < n->key) {
        n->left = insertNode(n->left, key, val);
    } else if (key > n->key) {
        n->right = insertNode(n->right, key, val);
    } else {
        /* 键已存在，更新值 */
        n->val = val;
        return n;
    }

    return balance(n);
}

/**
 * @brief 插入键值对
 * @param key 键
 * @param value 值
 */
void AVLTree6::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    int oldSize = m_size;
    m_root = insertNode(m_root, key, value);
    m_size = m_root ? m_root->count : 0;

    /* 更新统计 */
    m_stats.totalInserts++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInserts + m_stats.totalQueries);

    emit inserted(key);
}

/**
 * @brief 递归删除节点
 * @param n 当前子树根节点
 * @param key 待删除的键
 * @return 删除并平衡后的子树根节点
 */
AVLTree6::AVLNode* AVLTree6::removeNode(AVLNode* n, double key)
{
    if (!n) return nullptr;

    if (key < n->key) {
        n->left = removeNode(n->left, key);
    } else if (key > n->key) {
        n->right = removeNode(n->right, key);
    } else {
        /* 找到要删除的节点 */
        if (!n->left || !n->right) {
            AVLNode* child = n->left ? n->left : n->right;
            delete n;
            return child;
        }

        /* 找到中序后继 */
        AVLNode* succ = n->right;
        while (succ->left) succ = succ->left;
        n->key = succ->key;
        n->val = succ->val;
        n->right = removeNode(n->right, succ->key);
    }

    return balance(n);
}

/**
 * @brief 删除指定键的节点
 * @param key 待删除的键
 */
void AVLTree6::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    m_root = removeNode(m_root, key);
    m_size = m_root ? m_root->count : 0;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
}

/**
 * @brief 查找键是否存在
 * @param key 待查找的键
 * @return 存在返回true
 */
bool AVLTree6::contains(double key) const
{
    AVLNode* cur = m_root;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return true;
    }
    return false;
}

/**
 * @brief 范围查询，返回[lo, hi]区间内的所有值
 * @param lo 下界
 * @param hi 上界
 * @return 区间内的值集合
 */
QVector<int> AVLTree6::rangeQuery(double lo, double hi) const
{
    QVector<int> res;
    rangeQueryNode(m_root, lo, hi, res);
    return res;
}

/**
 * @brief 递归范围查询
 * @param n 当前节点
 * @param lo 下界
 * @param hi 上界
 * @param res 结果集合
 */
void AVLTree6::rangeQueryNode(AVLNode* n, double lo, double hi,
                                QVector<int>& res) const
{
    if (!n) return;
    if (lo < n->key) rangeQueryNode(n->left, lo, hi, res);
    if (lo <= n->key && n->key <= hi) res.append(n->val);
    if (hi > n->key) rangeQueryNode(n->right, lo, hi, res);
}

/**
 * @brief 计算键的排名（小于key的元素个数）
 * @param key 目标键
 * @return 排名值
 */
int AVLTree6::rank(double key) const
{
    QElapsedTimer timer;
    timer.start();

    int r = 0;
    AVLNode* cur = m_root;
    while (cur) {
        if (key < cur->key) {
            cur = cur->left;
        } else if (key > cur->key) {
            int lc = cur->left ? cur->left->count : 0;
            r += lc + 1;
            cur = cur->right;
        } else {
            int lc = cur->left ? cur->left->count : 0;
            r += lc;
            break;
        }
    }

    m_stats.totalQueries++;
    return r;
}

/**
 * @brief 选择第k小的元素对应的键
 * @param k 排名（0-indexed）
 * @return 第k小的键
 */
double AVLTree6::select(int k) const
{
    QElapsedTimer timer;
    timer.start();

    AVLNode* cur = m_root;
    while (cur) {
        int lc = cur->left ? cur->left->count : 0;
        if (k < lc) {
            cur = cur->left;
        } else if (k == lc) {
            return cur->key;
        } else {
            k -= lc + 1;
            cur = cur->right;
        }
    }

    return 0.0; /* 未找到 */
}

/**
 * @brief 获取当前统计信息
 * @return 操作统计结构
 */
AVLTree6::Stats AVLTree6::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void AVLTree6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

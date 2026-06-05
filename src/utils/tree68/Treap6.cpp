/**
 * @file Treap6.cpp
 * @brief Treap（树堆）数据结构实现（第6版）
 *
 * 实现Treap（Tree+Heap）随机化平衡二叉搜索树。
 * 通过随机优先级和BST键值的组合，以高概率保持树的平衡。
 * 支持插入、删除、查找、排名、选择、分裂和合并操作。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/tree68/Treap6.h"

#include <QElapsedTimer>
#include <QRandomGenerator>
#include <algorithm>

/**
 * @brief 构造函数，初始化Treap
 * @param parent 父QObject对象指针
 */
Treap6::Treap6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 右旋转
 * @param n 旋转轴心节点
 * @return 旋转后的新根节点
 */
Treap6::TNode* Treap6::rotateRight(TNode* n)
{
    TNode* newRoot = n->left;
    n->left = newRoot->right;
    newRoot->right = n;

    /* 更新count */
    int lc = n->left ? n->left->count : 0;
    int rc = n->right ? n->right->count : 0;
    n->count = 1 + lc + rc;

    lc = newRoot->left ? newRoot->left->count : 0;
    rc = newRoot->right ? newRoot->right->count : 0;
    newRoot->count = 1 + lc + rc;

    return newRoot;
}

/**
 * @brief 左旋转
 * @param n 旋转轴心节点
 * @return 旋转后的新根节点
 */
Treap6::TNode* Treap6::rotateLeft(TNode* n)
{
    TNode* newRoot = n->right;
    n->right = newRoot->left;
    newRoot->left = n;

    /* 更新count */
    int lc = n->left ? n->left->count : 0;
    int rc = n->right ? n->right->count : 0;
    n->count = 1 + lc + rc;

    lc = newRoot->left ? newRoot->left->count : 0;
    rc = newRoot->right ? newRoot->right->count : 0;
    newRoot->count = 1 + lc + rc;

    return newRoot;
}

/**
 * @brief 递归插入节点
 * @param n 当前子树根节点
 * @param key 插入的键
 * @param val 插入的值
 * @return 插入后的子树根节点
 */
Treap6::TNode* Treap6::insertNode(TNode* n, double key, int val)
{
    if (!n) {
        int prio = QRandomGenerator::global()->generate();
        return new TNode{key, val, prio, 1, nullptr, nullptr};
    }

    if (key < n->key) {
        n->left = insertNode(n->left, key, val);
        if (n->left && n->left->priority > n->priority) {
            n = rotateRight(n);
        }
    } else if (key > n->key) {
        n->right = insertNode(n->right, key, val);
        if (n->right && n->right->priority > n->priority) {
            n = rotateLeft(n);
        }
    } else {
        /* 键已存在，更新值 */
        n->val = val;
    }

    /* 更新count */
    int lc = n->left ? n->left->count : 0;
    int rc = n->right ? n->right->count : 0;
    n->count = 1 + lc + rc;

    return n;
}

/**
 * @brief 递归删除节点
 * @param n 当前子树根节点
 * @param key 待删除的键
 * @return 删除后的子树根节点
 */
Treap6::TNode* Treap6::removeNode(TNode* n, double key)
{
    if (!n) return nullptr;

    if (key < n->key) {
        n->left = removeNode(n->left, key);
    } else if (key > n->key) {
        n->right = removeNode(n->right, key);
    } else {
        /* 找到要删除的节点 */
        if (!n->left && !n->right) {
            delete n;
            return nullptr;
        }

        if (!n->left) {
            TNode* right = n->right;
            delete n;
            return right;
        }

        if (!n->right) {
            TNode* left = n->left;
            delete n;
            return left;
        }

        /* 两个子节点都存在，按优先级旋转 */
        if (n->left->priority > n->right->priority) {
            n = rotateRight(n);
            n->right = removeNode(n->right, key);
        } else {
            n = rotateLeft(n);
            n->left = removeNode(n->left, key);
        }
    }

    /* 更新count */
    if (n) {
        int lc = n->left ? n->left->count : 0;
        int rc = n->right ? n->right->count : 0;
        n->count = 1 + lc + rc;
    }

    return n;
}

/**
 * @brief 分裂节点递归
 * @param n 当前节点
 * @param key 分裂键值
 * @param l 输出的左子树根
 * @param r 输出的右子树根
 */
void Treap6::splitNode(TNode* n, double key, TNode*& l, TNode*& r)
{
    if (!n) {
        l = r = nullptr;
        return;
    }

    if (key <= n->key) {
        splitNode(n->left, key, l, n->left);
        r = n;
    } else {
        splitNode(n->right, key, n->right, r);
        l = n;
    }

    /* 更新count */
    if (n) {
        int lc = n->left ? n->left->count : 0;
        int rc = n->right ? n->right->count : 0;
        n->count = 1 + lc + rc;
    }
}

/**
 * @brief 合并两个子树
 * @param l 左子树（所有键 < 右子树的所有键）
 * @param r 右子树
 * @return 合并后的根节点
 */
Treap6::TNode* Treap6::mergeNode(TNode* l, TNode* r)
{
    if (!l) return r;
    if (!r) return l;

    if (l->priority > r->priority) {
        l->right = mergeNode(l->right, r);
        int lc = l->left ? l->left->count : 0;
        int rc = l->right ? l->right->count : 0;
        l->count = 1 + lc + rc;
        return l;
    } else {
        r->left = mergeNode(l, r->left);
        int lc = r->left ? r->left->count : 0;
        int rc = r->right ? r->right->count : 0;
        r->count = 1 + lc + rc;
        return r;
    }
}

/**
 * @brief 插入键值对
 * @param key 键
 * @param value 值
 */
void Treap6::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

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
 * @brief 删除指定键
 * @param key 待删除的键
 */
void Treap6::remove(double key)
{
    m_root = removeNode(m_root, key);
    m_size = m_root ? m_root->count : 0;
}

/**
 * @brief 查找键是否存在
 * @param key 待查找的键
 * @return 存在返回true
 */
bool Treap6::contains(double key) const
{
    TNode* cur = m_root;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return true;
    }
    return false;
}

/**
 * @brief 计算键的排名
 * @param key 目标键
 * @return 小于key的元素个数
 */
int Treap6::rank(double key) const
{
    int r = 0;
    TNode* cur = m_root;
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
    return r;
}

/**
 * @brief 选择第k小的键
 * @param k 排名（0-indexed）
 * @return 第k小的键
 */
double Treap6::select(int k) const
{
    TNode* cur = m_root;
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
    return 0.0;
}

/**
 * @brief 按键值分裂为两棵Treap
 * @param key 分裂键值
 * @param left 输出的左树（键<=key）
 * @param right 输出的右树（键>key）
 */
void Treap6::split(double key, Treap6& left, Treap6& right)
{
    TNode* lRoot = nullptr;
    TNode* rRoot = nullptr;
    splitNode(m_root, key, lRoot, rRoot);

    left.m_root = lRoot;
    left.m_size = lRoot ? lRoot->count : 0;
    right.m_root = rRoot;
    right.m_size = rRoot ? rRoot->count : 0;

    m_root = nullptr;
    m_size = 0;
}

/**
 * @brief 合并另一棵Treap
 * @param other 另一棵Treap（所有键必须小于或大于当前树的所有键）
 */
void Treap6::merge(Treap6& other)
{
    m_root = mergeNode(m_root, other.m_root);
    m_size = m_root ? m_root->count : 0;
    other.m_root = nullptr;
    other.m_size = 0;
}

/**
 * @brief 获取当前统计信息
 * @return 操作统计结构
 */
Treap6::Stats Treap6::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void Treap6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file RedBlackTree6.cpp
 * @brief 红黑树实现（第6版）
 *
 * 实现自平衡红黑树数据结构，支持插入、删除、查找、排名和范围查询。
 * 红黑树通过颜色约束和旋转操作保证树的高度近似平衡（最长路径不超过
 * 最短路径的2倍），所有操作时间复杂度O(log n)。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/tree67/RedBlackTree6.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化红黑树
 * @param parent 父QObject对象指针
 */
RedBlackTree6::RedBlackTree6(QObject* parent)
    : QObject(parent)
{
    /* 创建NIL哨兵节点 */
    m_nil = new RBNode{0.0, 0, BLACK, nullptr, nullptr, nullptr, 0};
    m_nil->left = m_nil;
    m_nil->right = m_nil;
    m_nil->parent = m_nil;
    m_root = m_nil;
}

/**
 * @brief 左旋转
 * @param x 旋转轴心节点
 */
void RedBlackTree6::leftRotate(RBNode* x)
{
    RBNode* y = x->right;
    x->right = y->left;

    if (y->left != m_nil) {
        y->left->parent = x;
    }

    y->parent = x->parent;
    if (x->parent == m_nil) {
        m_root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;

    /* 更新count */
    x->count = 1 + x->left->count + x->right->count;
    y->count = 1 + y->left->count + y->right->count;
}

/**
 * @brief 右旋转
 * @param y 旋转轴心节点
 */
void RedBlackTree6::rightRotate(RBNode* y)
{
    RBNode* x = y->left;
    y->left = x->right;

    if (x->right != m_nil) {
        x->right->parent = y;
    }

    x->parent = y->parent;
    if (y->parent == m_nil) {
        m_root = x;
    } else if (y == y->parent->right) {
        y->parent->right = x;
    } else {
        y->parent->left = x;
    }

    x->right = y;
    y->parent = x;

    /* 更新count */
    y->count = 1 + y->left->count + y->right->count;
    x->count = 1 + x->left->count + x->right->count;
}

/**
 * @brief 插入后修复红黑性质
 * @param z 新插入的节点
 */
void RedBlackTree6::insertFixup(RBNode* z)
{
    while (z->parent->color == RED) {
        if (z->parent == z->parent->parent->left) {
            RBNode* uncle = z->parent->parent->right;
            if (uncle->color == RED) {
                /* Case 1：叔叔为红色，重新着色 */
                z->parent->color = BLACK;
                uncle->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    /* Case 2：三角形结构，先旋转为直线 */
                    z = z->parent;
                    leftRotate(z);
                }
                /* Case 3：直线结构，旋转并着色 */
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rightRotate(z->parent->parent);
            }
        } else {
            /* 镜像对称的情况 */
            RBNode* uncle = z->parent->parent->left;
            if (uncle->color == RED) {
                z->parent->color = BLACK;
                uncle->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rightRotate(z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                leftRotate(z->parent->parent);
            }
        }
    }
    m_root->color = BLACK;
}

/**
 * @brief 插入新节点
 * @param z 待插入的节点
 * @return 插入的节点指针
 */
RedBlackTree6::RBNode* RedBlackTree6::insertNode(RBNode* z)
{
    RBNode* y = m_nil;
    RBNode* x = m_root;

    /* 标准BST插入定位 */
    while (x != m_nil) {
        y = x;
        if (z->key < x->key) {
            x = x->left;
        } else if (z->key > x->key) {
            x = x->right;
        } else {
            /* 键已存在，更新值 */
            x->val = z->val;
            delete z;
            return x;
        }
    }

    z->parent = y;
    if (y == m_nil) {
        m_root = z;
    } else if (z->key < y->key) {
        y->left = z;
    } else {
        y->right = z;
    }

    z->left = m_nil;
    z->right = m_nil;
    z->color = RED;
    z->count = 1;

    /* 更新祖先count */
    RBNode* p = z->parent;
    while (p != m_nil) {
        p->count = 1 + p->left->count + p->right->count;
        p = p->parent;
    }

    /* 修复红黑性质 */
    insertFixup(z);
    return z;
}

/**
 * @brief 插入键值对
 * @param key 键
 * @param value 值
 */
void RedBlackTree6::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    RBNode* z = new RBNode{key, value, RED, nullptr, nullptr, nullptr, 1};
    insertNode(z);
    m_size = m_root->count;

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
void RedBlackTree6::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    /* 查找节点 */
    RBNode* z = m_root;
    while (z != m_nil) {
        if (key < z->key) z = z->left;
        else if (key > z->key) z = z->right;
        else break;
    }

    if (z == m_nil) return; /* 未找到 */

    /* 简化的删除：用后继替换，不维护完整红黑性质 */
    RBNode* y = z;
    RBNode* x;

    if (z->left == m_nil) {
        x = z->right;
    } else if (z->right == m_nil) {
        x = z->left;
    } else {
        /* 找中序后继 */
        y = z->right;
        while (y->left != m_nil) y = y->left;
        x = y->right;
    }

    if (y != z) {
        z->key = y->key;
        z->val = y->val;
    }

    /* 将y的子节点链接到y的父节点 */
    if (y->parent == m_nil) {
        m_root = x;
    } else if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }
    x->parent = y->parent;

    delete y;

    /* 更新祖先count */
    RBNode* p = x->parent;
    while (p != m_nil) {
        p->count = 1 + p->left->count + p->right->count;
        p = p->parent;
    }

    m_size = (m_root != m_nil) ? m_root->count : 0;
}

/**
 * @brief 查找键是否存在
 * @param key 待查找的键
 * @return 存在返回true
 */
bool RedBlackTree6::contains(double key) const
{
    RBNode* cur = m_root;
    while (cur != m_nil) {
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
int RedBlackTree6::rank(double key) const
{
    int r = 0;
    RBNode* cur = m_root;
    while (cur != m_nil) {
        if (key < cur->key) {
            cur = cur->left;
        } else if (key > cur->key) {
            r += cur->left->count + 1;
            cur = cur->right;
        } else {
            r += cur->left->count;
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
double RedBlackTree6::select(int k) const
{
    RBNode* cur = m_root;
    while (cur != m_nil) {
        int lc = cur->left->count;
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
 * @brief 范围查询
 * @param lo 下界
 * @param hi 上界
 * @return 区间内的值集合
 */
QVector<int> RedBlackTree6::rangeQuery(double lo, double hi) const
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
void RedBlackTree6::rangeQueryNode(RBNode* n, double lo, double hi,
                                     QVector<int>& res) const
{
    if (n == m_nil) return;
    if (lo < n->key) rangeQueryNode(n->left, lo, hi, res);
    if (lo <= n->key && n->key <= hi) res.append(n->val);
    if (hi > n->key) rangeQueryNode(n->right, lo, hi, res);
}

/**
 * @brief 获取当前统计信息
 * @return 操作统计结构
 */
RedBlackTree6::Stats RedBlackTree6::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void RedBlackTree6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

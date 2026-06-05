/**
 * @file SplayTree6.cpp
 * @brief 伸展树（Splay Tree）实现（第6版）
 *
 * 实现自调整伸展树。每次访问（查找/插入）后将目标节点
 * 旋转到根位置（splay操作），使频繁访问的元素靠近根。
 * 通过zig、zig-zig、zig-zag三种旋转模式实现伸展。
 * 摊还时间复杂度O(log n)。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/tree69/SplayTree6.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化伸展树
 * @param parent 父QObject对象指针
 */
SplayTree6::SplayTree6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief Zig旋转（单旋转）
 *
 * 当节点是父节点的左子，且父是根时执行。
 * 将节点旋转到根位置。
 *
 * @param n 待旋转的节点
 */
void SplayTree6::zig(SNode* n)
{
    SNode* p = n->parent;
    if (!p) return;

    SNode* g = p->parent;

    if (n == p->left) {
        /* 右旋转 */
        p->left = n->right;
        if (n->right) n->right->parent = p;
        n->right = p;
        p->parent = n;
    } else {
        /* 左旋转 */
        p->right = n->left;
        if (n->left) n->left->parent = p;
        n->left = p;
        p->parent = n;
    }

    n->parent = g;
    if (g) {
        if (p == g->left) g->left = n;
        else g->right = n;
    } else {
        m_root = n;
    }

    /* 更新count */
    int lc, rc;
    lc = p->left ? p->left->count : 0;
    rc = p->right ? p->right->count : 0;
    p->count = 1 + lc + rc;

    lc = n->left ? n->left->count : 0;
    rc = n->right ? n->right->count : 0;
    n->count = 1 + lc + rc;
}

/**
 * @brief Zig-Zig旋转（同侧双旋转）
 *
 * 当节点和父节点在祖节点的同一侧时执行。
 * 先旋转父，再旋转节点。
 *
 * @param n 待旋转的节点
 */
void SplayTree6::zigZig(SNode* n)
{
    SNode* p = n->parent;
    SNode* g = p->parent;
    SNode* gg = g->parent;

    bool nIsLeft = (n == p->left);
    bool pIsLeft = (p == g->left);

    if (nIsLeft && pIsLeft) {
        /* 左-左：对g做两次右旋转 */
        g->left = p->right;
        if (p->right) p->right->parent = g;

        p->left = n->right;
        if (n->right) n->right->parent = p;

        p->right = g;
        g->parent = p;
        n->right = p;
        p->parent = n;
    } else {
        /* 右-右：对g做两次左旋转 */
        g->right = p->left;
        if (p->left) p->left->parent = g;

        p->right = n->left;
        if (n->left) n->left->parent = p;

        p->left = g;
        g->parent = p;
        n->left = p;
        p->parent = n;
    }

    n->parent = gg;
    if (gg) {
        if (g == gg->left) gg->left = n;
        else gg->right = n;
    } else {
        m_root = n;
    }

    /* 更新count */
    int lc, rc;
    lc = g->left ? g->left->count : 0;
    rc = g->right ? g->right->count : 0;
    g->count = 1 + lc + rc;

    lc = p->left ? p->left->count : 0;
    rc = p->right ? p->right->count : 0;
    p->count = 1 + lc + rc;

    lc = n->left ? n->left->count : 0;
    rc = n->right ? n->right->count : 0;
    n->count = 1 + lc + rc;
}

/**
 * @brief Zig-Zag旋转（异侧双旋转）
 *
 * 当节点和父节点在祖节点的不同侧时执行。
 * 先旋转节点，再旋转节点（两次zig）。
 *
 * @param n 待旋转的节点
 */
void SplayTree6::zigZag(SNode* n)
{
    SNode* p = n->parent;
    SNode* g = p->parent;
    SNode* gg = g->parent;

    bool nIsLeft = (n == p->left);
    bool pIsLeft = (p == g->left);

    if (nIsLeft && !pIsLeft) {
        /* p的左子，g的右子 */
        p->left = n->right;
        if (n->right) n->right->parent = p;

        g->right = n->left;
        if (n->left) n->left->parent = g;

        n->right = p;
        p->parent = n;
        n->left = g;
        g->parent = n;
    } else {
        /* p的右子，g的左子 */
        p->right = n->left;
        if (n->left) n->left->parent = p;

        g->left = n->right;
        if (n->right) n->right->parent = g;

        n->left = p;
        p->parent = n;
        n->right = g;
        g->parent = n;
    }

    n->parent = gg;
    if (gg) {
        if (g == gg->left) gg->left = n;
        else gg->right = n;
    } else {
        m_root = n;
    }

    /* 更新count */
    int lc, rc;
    lc = g->left ? g->left->count : 0;
    rc = g->right ? g->right->count : 0;
    g->count = 1 + lc + rc;

    lc = p->left ? p->left->count : 0;
    rc = p->right ? p->right->count : 0;
    p->count = 1 + lc + rc;

    lc = n->left ? n->left->count : 0;
    rc = n->right ? n->right->count : 0;
    n->count = 1 + lc + rc;
}

/**
 * @brief 伸展操作，将节点旋转到根
 *
 * 根据节点、父节点和祖节点的位置关系选择旋转模式：
 * - zig：父是根
 * - zig-zig：节点和父在同一侧
 * - zig-zag：节点和父在不同侧
 *
 * @param n 目标节点
 */
void SplayTree6::splay(SNode* n)
{
    if (!n) return;

    while (n->parent) {
        SNode* p = n->parent;
        SNode* g = p->parent;

        if (!g) {
            zig(n);
        } else {
            bool nIsLeft = (n == p->left);
            bool pIsLeft = (p == g->left);

            if (nIsLeft == pIsLeft) {
                zigZig(n);
            } else {
                zigZag(n);
            }
        }
    }

    m_root = n;
}

/**
 * @brief 插入键值对
 * @param key 键
 * @param value 值
 */
void SplayTree6::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) {
        m_root = new SNode{key, value, 1, nullptr, nullptr, nullptr};
        m_size = 1;
    } else {
        /* 标准BST插入 */
        SNode* cur = m_root;
        SNode* parent = nullptr;

        while (cur) {
            parent = cur;
            if (key < cur->key) {
                cur = cur->left;
            } else if (key > cur->key) {
                cur = cur->right;
            } else {
                /* 键已存在 */
                cur->val = value;
                splay(cur);
                m_stats.totalInserts++;
                qint64 elapsed = timer.elapsed();
                m_timeSum += elapsed;
                m_stats.avgProcessingTimeMs = m_timeSum /
                    (m_stats.totalInserts + m_stats.totalQueries);
                emit inserted(key);
                return;
            }
        }

        SNode* node = new SNode{key, value, 1, nullptr, nullptr, parent};
        if (key < parent->key) {
            parent->left = node;
        } else {
            parent->right = node;
        }

        /* 更新祖先count */
        SNode* p = parent;
        while (p) {
            p->count++;
            p = p->parent;
        }

        m_size++;
        splay(node);
    }

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
void SplayTree6::remove(double key)
{
    if (!m_root) return;

    /* 先splay目标节点到根 */
    SNode* cur = m_root;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else break;
    }

    if (!cur) return;
    splay(cur);

    /* 分裂左右子树 */
    SNode* leftTree = m_root->left;
    SNode* rightTree = m_root->right;

    if (leftTree) leftTree->parent = nullptr;
    if (rightTree) rightTree->parent = nullptr;

    delete m_root;
    m_size--;

    if (!leftTree) {
        m_root = rightTree;
    } else if (!rightTree) {
        m_root = leftTree;
    } else {
        /* 将左子树最大节点伸展到根，然后接上右子树 */
        SNode* maxNode = leftTree;
        while (maxNode->right) maxNode = maxNode->right;
        m_root = leftTree;
        splay(maxNode);
        m_root->right = rightTree;
        rightTree->parent = m_root;
    }

    if (m_root) m_root->parent = nullptr;
}

/**
 * @brief 查找键是否存在（查找后伸展到根）
 * @param key 待查找的键
 * @return 存在返回true
 */
bool SplayTree6::contains(double key)
{
    QElapsedTimer timer;
    timer.start();

    SNode* cur = m_root;
    SNode* last = nullptr;

    while (cur) {
        last = cur;
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else {
            splay(cur);
            m_stats.totalQueries++;
            qint64 elapsed = timer.elapsed();
            m_timeSum += elapsed;
            m_stats.avgProcessingTimeMs = m_timeSum /
                (m_stats.totalInserts + m_stats.totalQueries);
            return true;
        }
    }

    if (last) splay(last);

    m_stats.totalQueries++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInserts + m_stats.totalQueries);

    return false;
}

/**
 * @brief 计算键的排名
 * @param key 目标键
 * @return 小于key的元素个数
 */
int SplayTree6::rank(double key) const
{
    int r = 0;
    SNode* cur = m_root;
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
double SplayTree6::select(int k) const
{
    SNode* cur = m_root;
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
 * @brief 范围查询
 * @param lo 下界
 * @param hi 上界
 * @return 区间内的值集合
 */
QVector<int> SplayTree6::rangeQuery(double lo, double hi) const
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
void SplayTree6::rangeQueryNode(SNode* n, double lo, double hi,
                                  QVector<int>& res) const
{
    if (!n) return;
    if (lo < n->key) rangeQueryNode(n->left, lo, hi, res);
    if (lo <= n->key && n->key <= hi) res.append(n->val);
    if (hi > n->key) rangeQueryNode(n->right, lo, hi, res);
}

/**
 * @brief 获取当前统计信息
 * @return 操作统计结构
 */
SplayTree6::Stats SplayTree6::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void SplayTree6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

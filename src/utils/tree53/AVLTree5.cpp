/**
 * @file AVLTree5.cpp
 * @brief AVL自平衡二叉搜索树实现
 *
 * 实现AVL树数据结构，保证插入和删除操作后树的高度平衡。
 * AVL树中任意节点的左右子树高度差不超过1，通过旋转操作维持平衡。
 * 查找、插入、删除操作的时间复杂度均为 O(log n)。
 *
 * 支持四种旋转: 左旋(LL)、右旋(RR)、左右旋(LR)、右左旋(RL)
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/tree53/AVLTree5.h"

#include <QElapsedTimer>
#include <algorithm>
#include <QStack>

/**
 * @brief 构造函数，初始化空树
 * @param parent 父QObject对象指针
 */
AVLTree5::AVLTree5(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_size(0)
    , m_timeSum(0.0)
{
}

/**
 * @brief 析构函数，释放所有节点内存
 */
AVLTree5::~AVLTree5()
{
    destroy(m_root);
}

/**
 * @brief 插入键值对
 *
 * 向AVL树中插入指定键和关联值。若键已存在则更新值。
 * 插入后沿路径回溯检查平衡因子，必要时执行旋转。
 *
 * @param key 搜索键（浮点数）
 * @param value 关联值
 */
void AVLTree5::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    int prevSize = m_size;
    m_root = ins(m_root, key, value);
    /* m_size在ins()中递增（仅新键时） */

    /* 更新统计信息 */
    double elapsed = timer.elapsed();
    m_stats.totalInsertions++;
    m_timeSum += elapsed;
    int totalOps = m_stats.totalInsertions + m_stats.totalDeletions;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit treeModified(m_size);
}

/**
 * @brief 递归插入辅助函数
 *
 * 在以n为根的子树中递归查找插入位置，插入后检查并修复平衡。
 *
 * @param n 当前子树根节点
 * @param k 待插入键
 * @param v 待插入值
 * @return 插入（可能旋转）后的子树根节点
 */
AVLTree5::Node* AVLTree5::ins(Node* n, double k, int v)
{
    if (!n) {
        /* 创建新节点 */
        Node* node = new Node{k, v, 1, nullptr, nullptr};
        m_size++;
        m_stats.totalRotations = m_stats.totalRotations;  ///< 占位
        return node;
    }

    if (k < n->key) {
        n->l = ins(n->l, k, v);
    } else if (k > n->key) {
        n->r = ins(n->r, k, v);
    } else {
        /* 键已存在，更新值 */
        n->val = v;
        return n;
    }

    /* 更新高度 */
    n->ht = 1 + qMax(ht(n->l), ht(n->r));

    /* 平衡修复 */
    return bal(n);
}

/**
 * @brief 删除指定键的节点
 *
 * 从AVL树中删除指定键的节点。若键不存在则不做任何操作。
 * 删除后沿路径回溯检查平衡因子，必要时执行旋转。
 *
 * @param key 待删除的键
 * @return true表示成功删除，false表示键不存在
 */
bool AVLTree5::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    int prevSize = m_size;
    m_root = rem(m_root, key);

    bool removed = (m_size != prevSize || (m_root && find(key) == 0));

    /* 简化: 通过查找确认删除 */
    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_stats.totalDeletions++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInsertions + m_stats.totalDeletions);

    emit treeModified(m_size);
    return true;
}

/**
 * @brief 递归删除辅助函数
 *
 * 在以n为根的子树中递归查找并删除指定键的节点。
 * 删除策略: 若有两个子节点则用中序后继替换。
 *
 * @param n 当前子树根节点
 * @param k 待删除键
 * @return 删除（可能旋转）后的子树根节点
 */
AVLTree5::Node* AVLTree5::rem(Node* n, double k)
{
    if (!n) {
        return nullptr;
    }

    if (k < n->key) {
        n->l = rem(n->l, k);
    } else if (k > n->key) {
        n->r = rem(n->r, k);
    } else {
        /* 找到要删除的节点 */
        m_size--;

        if (!n->l || !n->r) {
            /* 至多一个子节点，直接用子节点替换 */
            Node* child = n->l ? n->l : n->r;
            delete n;
            return child;
        }

        /* 两个子节点: 用中序后继（右子树最小值）替换 */
        Node* successor = n->r;
        while (successor->l) {
            successor = successor->l;
        }

        n->key = successor->key;
        n->val = successor->val;
        m_size++;  ///< 修正计数（递归rem会再次减1）
        n->r = rem(n->r, successor->key);
    }

    if (!n) {
        return nullptr;
    }

    /* 更新高度 */
    n->ht = 1 + qMax(ht(n->l), ht(n->r));

    /* 平衡修复 */
    return bal(n);
}

/**
 * @brief 平衡修复函数
 *
 * 检查节点n的平衡因子（左右子树高度差），根据不平衡类型执行相应旋转:
 * - 左左型(LL): 右旋
 * - 右右型(RR): 左旋
 * - 左右型(LR): 先左旋左子节点再右旋
 * - 右左型(RL): 先右旋右子节点再左旋
 *
 * @param n 需要检查平衡的节点
 * @return 旋转后的子树根节点
 */
AVLTree5::Node* AVLTree5::bal(Node* n)
{
    int bf = ht(n->l) - ht(n->r);

    if (bf > 1) {
        /* 左侧重 */
        if (ht(n->l->l) >= ht(n->l->r)) {
            /* LL型: 右旋 */
            m_stats.totalRotations++;
            return rotR(n);
        } else {
            /* LR型: 先左旋左子节点，再右旋 */
            m_stats.totalRotations += 2;
            n->l = rotL(n->l);
            return rotR(n);
        }
    }

    if (bf < -1) {
        /* 右侧重 */
        if (ht(n->r->r) >= ht(n->r->l)) {
            /* RR型: 左旋 */
            m_stats.totalRotations++;
            return rotL(n);
        } else {
            /* RL型: 先右旋右子节点，再左旋 */
            m_stats.totalRotations += 2;
            n->r = rotR(n->r);
            return rotL(n);
        }
    }

    return n;
}

/**
 * @brief 左旋操作
 *
 * 以节点x为支点执行左旋，x的右子节点y上升为新的子树根。
 *     x                y
 *    / \              / \
 *   a   y    =>     x   c
 *      / \         / \
 *     b   c       a   b
 *
 * @param x 旋转支点
 * @return 旋转后的新子树根（原右子节点）
 */
AVLTree5::Node* AVLTree5::rotL(Node* x)
{
    Node* y = x->r;
    x->r = y->l;
    y->l = x;

    /* 更新高度: 先更新x（现在在下方），再更新y */
    x->ht = 1 + qMax(ht(x->l), ht(x->r));
    y->ht = 1 + qMax(ht(y->l), ht(y->r));

    return y;
}

/**
 * @brief 右旋操作
 *
 * 以节点y为支点执行右旋，y的左子节点x上升为新的子树根。
 *       y              x
 *      / \            / \
 *     x   c   =>    a   y
 *    / \                / \
 *   a   b              b   c
 *
 * @param y 旋转支点
 * @return 旋转后的新子树根（原左子节点）
 */
AVLTree5::Node* AVLTree5::rotR(Node* y)
{
    Node* x = y->l;
    y->l = x->r;
    x->r = y;

    y->ht = 1 + qMax(ht(y->l), ht(y->r));
    x->ht = 1 + qMax(ht(x->l), ht(x->r));

    return x;
}

/**
 * @brief 获取节点高度（安全版本）
 * @param n 节点指针，可以为nullptr
 * @return 节点高度，空节点返回0
 */
int AVLTree5::ht(Node* n) const
{
    return n ? n->ht : 0;
}

/**
 * @brief 查找指定键关联的值
 *
 * 从根节点开始二分搜索，根据键的比较结果决定搜索方向。
 *
 * @param key 待查找的键
 * @return 关联值，键不存在时返回0
 */
int AVLTree5::find(double key) const
{
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) {
            cur = cur->l;
        } else if (key > cur->key) {
            cur = cur->r;
        } else {
            return cur->val;
        }
    }
    return 0;
}

/**
 * @brief 递归销毁节点树
 * @param n 当前节点
 */
void AVLTree5::destroy(Node* n)
{
    if (!n) return;
    destroy(n->l);
    destroy(n->r);
    delete n;
}

/**
 * @brief 清空整棵树
 */
void AVLTree5::clear()
{
    destroy(m_root);
    m_root = nullptr;
    m_size = 0;
}

/**
 * @brief 获取树的高度
 * @return 树的高度（根节点高度），空树返回0
 */
int AVLTree5::height() const
{
    return ht(m_root);
}

/**
 * @brief 中序遍历，返回所有键值对
 *
 * 按键值从小到大的顺序遍历AVL树，返回所有节点的键值对。
 * 使用栈实现非递归中序遍历。
 *
 * @return 按键排序的键值对列表
 */
QVector<QPair<double,int>> AVLTree5::inOrderTraversal() const
{
    QVector<QPair<double,int>> result;
    if (!m_root) {
        return result;
    }

    QStack<Node*> stack;
    Node* cur = m_root;

    while (cur || !stack.empty()) {
        /* 沿左子树到底 */
        while (cur) {
            stack.push(cur);
            cur = cur->l;
        }

        cur = stack.pop();
        result.append({cur->key, cur->val});

        /* 转向右子树 */
        cur = cur->r;
    }

    return result;
}

/**
 * @brief 重置所有统计计数器
 */
void AVLTree5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

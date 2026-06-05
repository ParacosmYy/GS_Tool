/**
 * @file Treap5.cpp
 * @brief Treap（树堆）数据结构实现，结合BST和堆的性质
 *
 * Treap是一种随机化平衡二叉搜索树，每个节点同时具有：
 * - key值：满足BST性质（左子树 < 根 < 右子树）
 * - priority值：满足堆性质（父节点优先级 >= 子节点）
 *
 * 通过随机优先级保证期望O(log n)的操作效率。
 * 支持插入、删除、查找、中序遍历等操作。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/tree55/Treap5.h"

#include <QElapsedTimer>
#include <QRandomGenerator>

/**
 * @brief 构造函数，初始化空Treap
 * @param parent 父QObject对象指针
 */
Treap5::Treap5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 析构函数，递归释放所有节点
 */
Treap5::~Treap5()
{
    destroy(m_root);
}

/**
 * @brief 递归销毁以n为根的子树
 * @param n 子树根节点
 */
void Treap5::destroy(Node* n)
{
    if (!n) return;
    destroy(n->l);
    destroy(n->r);
    delete n;
}

/**
 * @brief 左旋转操作
 *
 * 在Treap中，当右子节点优先级更高时执行左旋：
 *     x              y
 *    / \     =>     / \
 *   a   y          x   c
 *      / \        / \
 *     b   c      a   b
 *
 * @param x 旋转根节点
 * @return 旋转后的新根节点
 */
Treap5::Node* Treap5::rotL(Node* x)
{
    Node* y = x->r;
    x->r = y->l;
    y->l = x;
    return y;
}

/**
 * @brief 右旋转操作
 *
 * 在Treap中，当左子节点优先级更高时执行右旋：
 *       y           x
 *      / \   =>   / \
 *     x   c      a   y
 *    / \            / \
 *   a   b          b   c
 *
 * @param y 旋转根节点
 * @return 旋转后的新根节点
 */
Treap5::Node* Treap5::rotR(Node* y)
{
    Node* x = y->l;
    y->l = x->r;
    x->r = y;
    return x;
}

/**
 * @brief 递归插入节点
 *
 * 算法流程：
 * 1. 按BST性质找到插入位置
 * 2. 创建新节点并赋予随机优先级
 * 3. 通过旋转维护堆性质（向上冒泡）
 *
 * @param n 当前子树根节点
 * @param k 待插入键值
 * @param v 待插入值
 * @return 插入后的子树根节点
 */
Treap5::Node* Treap5::ins(Node* n, double k, int v)
{
    if (!n) {
        Node* node = new Node{k, v, QRandomGenerator::global()->bounded(1000000), nullptr, nullptr};
        return node;
    }

    if (k < n->key) {
        n->l = ins(n->l, k, v);
        /* 维护堆性质：左子节点优先级更高则右旋 */
        if (n->l && n->l->pri > n->pri)
            n = rotR(n);
    } else if (k > n->key) {
        n->r = ins(n->r, k, v);
        /* 维护堆性质：右子节点优先级更高则左旋 */
        if (n->r && n->r->pri > n->pri)
            n = rotL(n);
    } else {
        /* 键值已存在，更新值 */
        n->val = v;
    }

    return n;
}

/**
 * @brief 递归删除节点
 *
 * 算法流程：
 * 1. 按BST性质定位目标节点
 * 2. 将目标节点通过旋转下沉到叶子位置
 * 3. 在叶子位置直接删除
 *
 * @param n 当前子树根节点
 * @param k 待删除键值
 * @return 删除后的子树根节点
 */
Treap5::Node* Treap5::rem(Node* n, double k)
{
    if (!n) return nullptr;

    if (k < n->key) {
        n->l = rem(n->l, k);
    } else if (k > n->key) {
        n->r = rem(n->r, k);
    } else {
        /* 找到目标节点，通过旋转下沉 */
        if (!n->l && !n->r) {
            /* 叶子节点，直接删除 */
            delete n;
            return nullptr;
        }

        /* 选择优先级较高的子节点进行旋转 */
        if (!n->l || (n->r && n->r->pri > n->l->pri)) {
            /* 左旋，将右子节点提升 */
            n = rotL(n);
            n->l = rem(n->l, k);
        } else {
            /* 右旋，将左子节点提升 */
            n = rotR(n);
            n->r = rem(n->r, k);
        }
    }
    return n;
}

/**
 * @brief 插入键值对
 * @param key 键值
 * @param value 关联值
 */
void Treap5::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    m_root = ins(m_root, key, value);
    ++m_size;

    m_stats.totalInsertions++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInsertions + m_stats.totalDeletions);

    emit treeModified(m_size);
}

/**
 * @brief 删除指定键值的节点
 * @param key 待删除的键值
 * @return true删除成功，false键值不存在
 */
bool Treap5::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    /* 先检查是否存在 */
    Node* cur = m_root;
    bool found = false;
    while (cur) {
        if (key < cur->key) cur = cur->l;
        else if (key > cur->key) cur = cur->r;
        else { found = true; break; }
    }

    if (!found) return false;

    m_root = rem(m_root, key);
    --m_size;

    m_stats.totalDeletions++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInsertions + m_stats.totalDeletions);

    emit treeModified(m_size);
    return true;
}

/**
 * @brief 查找指定键值的关联值
 * @param key 待查找的键值
 * @return 关联值，键值不存在返回0
 */
int Treap5::find(double key) const
{
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) cur = cur->l;
        else if (key > cur->key) cur = cur->r;
        else return cur->val;
    }
    return 0;
}

/**
 * @brief 清空所有节点
 */
void Treap5::clear()
{
    destroy(m_root);
    m_root = nullptr;
    m_size = 0;
}

/**
 * @brief 中序遍历Treap，返回有序键值对列表
 * @return 按键值升序排列的(键, 值)对列表
 */
QVector<QPair<double, int>> Treap5::inOrderTraversal() const
{
    QVector<QPair<double, int>> result;

    /* 迭代式中序遍历（使用栈模拟递归） */
    QVector<Node*> stack;
    Node* cur = m_root;

    while (cur || !stack.isEmpty()) {
        while (cur) {
            stack.append(cur);
            cur = cur->l;
        }
        cur = stack.takeLast();
        result.append({cur->key, cur->val});
        cur = cur->r;
    }

    return result;
}

/**
 * @brief 重置所有统计数据
 */
void Treap5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

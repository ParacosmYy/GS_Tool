/**
 * @file SplayTree5.cpp
 * @brief 伸展树（Splay Tree）实现
 *
 * 实现自调整二叉搜索树（伸展树），通过splay操作将最近访问的节点
 * 移动到树根，利用局部性原理优化频繁访问模式。
 *
 * Splay操作（三种情况）:
 * - Zig: 当前节点是根的子节点（终止条件）
 * - Zig-Zig: 当前节点和父节点同侧（先旋父再旋己）
 * - Zig-Zag: 当前节点和父节点异侧（先旋己再旋己）
 *
 * 摊还复杂度: 插入/删除/查找均为 O(log n)
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/tree54/SplayTree5.h"

#include <QElapsedTimer>
#include <QStack>

/**
 * @brief 构造函数，初始化空树
 * @param parent 父QObject对象指针
 */
SplayTree5::SplayTree5(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_size(0)
    , m_timeSum(0.0)
{
}

/**
 * @brief 析构函数，释放所有节点内存
 */
SplayTree5::~SplayTree5()
{
    destroy(m_root);
}

/**
 * @brief 插入键值对
 *
 * 将指定键值对插入伸展树。插入后对目标节点执行splay操作
 * 将其移动到根位置。
 *
 * @param key 搜索键
 * @param value 关联值
 */
void SplayTree5::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    /* 空树: 直接创建根节点 */
    if (!m_root) {
        m_root = new Node{key, value, nullptr, nullptr, nullptr};
        m_size++;
        updateStats(timer);
        emit treeModified(m_size);
        return;
    }

    /* 查找插入位置 */
    Node* cur = m_root;
    Node* parent = nullptr;

    while (cur) {
        parent = cur;
        if (key < cur->key) {
            cur = cur->l;
        } else if (key > cur->key) {
            cur = cur->r;
        } else {
            /* 键已存在，更新值并splay */
            cur->val = value;
            splay(cur);
            updateStats(timer);
            emit treeModified(m_size);
            return;
        }
    }

    /* 创建新节点 */
    Node* node = new Node{key, value, nullptr, nullptr, parent};

    if (key < parent->key) {
        parent->l = node;
    } else {
        parent->r = node;
    }

    m_size++;

    /* Splay新节点到根 */
    splay(node);

    updateStats(timer);
    m_stats.totalInsertions++;
    emit treeModified(m_size);
}

/**
 * @brief 删除指定键的节点
 *
 * 先splay目标节点到根，然后删除根节点，将左右子树合并。
 * 合并策略: 将左子树的最大节点splay到左子树根，然后挂接右子树。
 *
 * @param key 待删除的键
 * @return true成功删除，false键不存在
 */
bool SplayTree5::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    /* 查找节点 */
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key) {
            cur = cur->l;
        } else if (key > cur->key) {
            cur = cur->r;
        } else {
            break;
        }
    }

    if (!cur) {
        /* 键不存在 */
        updateStats(timer);
        return false;
    }

    /* Splay到根 */
    splay(cur);
    m_root = cur;

    /* 分离左右子树 */
    Node* leftTree = m_root->l;
    Node* rightTree = m_root->r;

    /* 断开父指针 */
    if (leftTree) {
        leftTree->p = nullptr;
    }
    if (rightTree) {
        rightTree->p = nullptr;
    }

    /* 删除根节点 */
    delete m_root;
    m_size--;

    if (!leftTree) {
        /* 无左子树，右子树成为新根 */
        m_root = rightTree;
    } else if (!rightTree) {
        /* 无右子树，左子树成为新根 */
        m_root = leftTree;
    } else {
        /* 两棵子树都存在: 将左子树最大节点splay到根 */
        Node* maxLeft = leftTree;
        while (maxLeft->r) {
            maxLeft = maxLeft->r;
        }
        m_root = leftTree;
        splay(maxLeft);
        m_root = maxLeft;

        /* 挂接右子树 */
        m_root->r = rightTree;
        rightTree->p = m_root;
    }

    m_stats.totalDeletions++;
    updateStats(timer);
    emit treeModified(m_size);
    return true;
}

/**
 * @brief 查找指定键关联的值
 *
 * 查找指定键，找到后执行splay操作将其移动到根。
 * 未找到时splay最后访问的节点。
 *
 * @param key 待查找的键
 * @return 关联值，键不存在时返回0
 */
int SplayTree5::find(double key)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) {
        updateStats(timer);
        return 0;
    }

    Node* cur = m_root;
    Node* last = cur;

    while (cur) {
        last = cur;
        if (key < cur->key) {
            cur = cur->l;
        } else if (key > cur->key) {
            cur = cur->r;
        } else {
            /* 找到: splay到根 */
            splay(cur);
            m_root = cur;
            updateStats(timer);
            return cur->val;
        }
    }

    /* 未找到: splay最后访问的节点 */
    splay(last);
    m_root = last;

    updateStats(timer);
    return 0;
}

/**
 * @brief Splay操作: 将节点x移动到根
 *
 * 通过一系列旋转将节点x逐步移动到树根。根据x、x的父节点、
 * x的祖父节点之间的位置关系，选择适当的旋转方式:
 *
 * - Zig (x的父节点是根): 单次旋转
 * - Zig-Zig (x和父节点在同侧): 先旋转父节点，再旋转x
 * - Zig-Zag (x和父节点在异侧): 旋转x两次
 *
 * @param x 需要splay的节点
 */
void SplayTree5::splay(Node* x)
{
    if (!x) return;

    m_stats.totalSplays++;

    while (x->p) {
        Node* p = x->p;
        Node* g = p->p;

        if (!g) {
            /* Zig: 父节点是根，单次旋转 */
            if (x == p->l) {
                rotR(p);
            } else {
                rotL(p);
            }
        } else {
            bool xIsLeft = (x == p->l);
            bool pIsLeft = (p == g->l);

            if (xIsLeft && pIsLeft) {
                /* Zig-Zig (左-左): 先右旋祖父，再右旋父 */
                rotR(g);
                rotR(p);
            } else if (!xIsLeft && !pIsLeft) {
                /* Zig-Zig (右-右): 先左旋祖父，再左旋父 */
                rotL(g);
                rotL(p);
            } else if (xIsLeft && !pIsLeft) {
                /* Zig-Zag (左-右): 先右旋父，再左旋祖父 */
                rotR(p);
                rotL(g);
            } else {
                /* Zig-Zag (右-左): 先左旋父，再右旋祖父 */
                rotL(p);
                rotR(g);
            }
        }
    }

    m_root = x;
}

/**
 * @brief 左旋操作
 *
 * 以节点x为支点执行左旋:
 *     x              y
 *    / \            / \
 *   a   y   =>    x   c
 *      / \       / \
 *     b   c     a   b
 *
 * 维护父指针关系。
 *
 * @param x 旋转支点
 */
void SplayTree5::rotL(Node* x)
{
    if (!x || !x->r) return;

    Node* y = x->r;
    x->r = y->l;

    if (y->l) {
        y->l->p = x;
    }

    y->p = x->p;

    if (!x->p) {
        /* x是根 */
        m_root = y;
    } else if (x == x->p->l) {
        x->p->l = y;
    } else {
        x->p->r = y;
    }

    y->l = x;
    x->p = y;
}

/**
 * @brief 右旋操作
 *
 * 以节点x为支点执行右旋:
 *       x          y
 *      / \        / \
 *     y   c =>   a   x
 *    / \             / \
 *   a   b           b   c
 *
 * 维护父指针关系。
 *
 * @param x 旋转支点
 */
void SplayTree5::rotR(Node* x)
{
    if (!x || !x->l) return;

    Node* y = x->l;
    x->l = y->r;

    if (y->r) {
        y->r->p = x;
    }

    y->p = x->p;

    if (!x->p) {
        m_root = y;
    } else if (x == x->p->l) {
        x->p->l = y;
    } else {
        x->p->r = y;
    }

    y->r = x;
    x->p = y;
}

/**
 * @brief 递归销毁节点树
 * @param n 当前节点
 */
void SplayTree5::destroy(Node* n)
{
    if (!n) return;
    destroy(n->l);
    destroy(n->r);
    delete n;
}

/**
 * @brief 清空整棵树
 */
void SplayTree5::clear()
{
    destroy(m_root);
    m_root = nullptr;
    m_size = 0;
}

/**
 * @brief 中序遍历，返回所有键值对
 *
 * 使用栈实现非递归中序遍历，按键值从小到大返回。
 *
 * @return 按键排序的键值对列表
 */
QVector<QPair<double,int>> SplayTree5::inOrderTraversal() const
{
    QVector<QPair<double,int>> result;
    if (!m_root) return result;

    QStack<Node*> stack;
    Node* cur = m_root;

    while (cur || !stack.empty()) {
        while (cur) {
            stack.push(cur);
            cur = cur->l;
        }
        cur = stack.pop();
        result.append({cur->key, cur->val});
        cur = cur->r;
    }

    return result;
}

/**
 * @brief 更新统计信息
 * @param timer 计时器（已启动）
 */
void SplayTree5::updateStats(QElapsedTimer& timer)
{
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    int totalOps = m_stats.totalInsertions + m_stats.totalDeletions + 1;
    m_stats.avgProcessingTimeMs = m_timeSum / totalOps;
}

/**
 * @brief 重置所有统计计数器
 */
void SplayTree5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

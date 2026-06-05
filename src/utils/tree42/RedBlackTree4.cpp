/**
 * @file RedBlackTree4.cpp
 * @brief 红黑树4 — 内存池分配+迭代器支持实现
 *
 * 实现基于索引的红黑树，使用预分配内存池：
 * - 池化节点分配/释放（freeList管理）
 * - 左旋/右旋/插入修复/删除修复
 * - 子树大小维护
 * - 中序遍历
 * 所有运算带有QElapsedTimer计时和统计信息追踪。
 */

#include "utils/tree42/RedBlackTree4.h"

#include <QElapsedTimer>
#include <QStack>

/**
 * @brief 构造函数，初始化内存池
 * @param poolSize 池容量
 * @param parent 父对象
 */
RedBlackTree4::RedBlackTree4(int poolSize, QObject* parent)
    : QObject(parent)
    , m_root(-1)
    , m_poolUsed(0)
    , m_size(0)
{
    /* Node 0 作为哨兵节点(NIL) */
    m_pool.resize(poolSize + 1);
    m_pool[0] = {0.0, 0, Black, 0, 0, 0, 0};
    m_nil = 0;

    /* 构建空闲链表 */
    m_freeList.reserve(poolSize);
    for (int i = poolSize; i >= 1; --i)
        m_freeList.append(i);
}

/**
 * @brief 析构函数
 */
RedBlackTree4::~RedBlackTree4()
{
}

/**
 * @brief 从内存池分配一个节点
 * @return 节点索引，池满返回0(NIL)
 */
int RedBlackTree4::allocNode()
{
    if (m_freeList.isEmpty()) return m_nil;
    int idx = m_freeList.takeLast();
    Node& n = m_pool[idx];
    n.color = Red;
    n.subtreeSize = 1;
    n.left = m_nil;
    n.right = m_nil;
    n.parent = m_nil;
    m_poolUsed++;
    return idx;
}

/**
 * @brief 释放节点到内存池
 * @param idx 节点索引
 */
void RedBlackTree4::freeNode(int idx)
{
    if (idx == m_nil) return;
    m_freeList.append(idx);
    m_poolUsed--;
}

/**
 * @brief 更新子树大小
 * @param n 节点索引
 */
void RedBlackTree4::updateSize(int n)
{
    if (n == m_nil) return;
    m_pool[n].subtreeSize = 1 + m_pool[m_pool[n].left].subtreeSize
                              + m_pool[m_pool[n].right].subtreeSize;
}

/**
 * @brief 左旋
 * @param x 旋转节点索引
 */
void RedBlackTree4::leftRotate(int x)
{
    int y = m_pool[x].right;
    if (y == m_nil) return;

    m_pool[x].right = m_pool[y].left;
    if (m_pool[y].left != m_nil)
        m_pool[m_pool[y].left].parent = x;

    m_pool[y].parent = m_pool[x].parent;
    if (m_pool[x].parent == m_nil)
        m_root = y;
    else if (x == m_pool[m_pool[x].parent].left)
        m_pool[m_pool[x].parent].left = y;
    else
        m_pool[m_pool[x].parent].right = y;

    m_pool[y].left = x;
    m_pool[x].parent = y;
    updateSize(x);
    updateSize(y);
}

/**
 * @brief 右旋
 * @param y 旋转节点索引
 */
void RedBlackTree4::rightRotate(int y)
{
    int x = m_pool[y].left;
    if (x == m_nil) return;

    m_pool[y].left = m_pool[x].right;
    if (m_pool[x].right != m_nil)
        m_pool[m_pool[x].right].parent = y;

    m_pool[x].parent = m_pool[y].parent;
    if (m_pool[y].parent == m_nil)
        m_root = x;
    else if (y == m_pool[m_pool[y].parent].right)
        m_pool[m_pool[y].parent].right = x;
    else
        m_pool[m_pool[y].parent].left = x;

    m_pool[x].right = y;
    m_pool[y].parent = x;
    updateSize(y);
    updateSize(x);
}

/**
 * @brief 插入后修复红黑性质
 * @param z 新插入节点索引
 */
void RedBlackTree4::insertFixup(int z)
{
    while (m_pool[m_pool[z].parent].color == Red) {
        int parent = m_pool[z].parent;
        int grandpa = m_pool[parent].parent;
        if (parent == m_pool[grandpa].left) {
            int uncle = m_pool[grandpa].right;
            if (m_pool[uncle].color == Red) {
                /* Case 1：叔叔为红 → 重新着色 */
                m_pool[parent].color = Black;
                m_pool[uncle].color = Black;
                m_pool[grandpa].color = Red;
                z = grandpa;
            } else {
                if (z == m_pool[parent].right) {
                    /* Case 2：三角 → 左旋 */
                    z = parent;
                    leftRotate(z);
                    parent = m_pool[z].parent;
                    grandpa = m_pool[parent].parent;
                }
                /* Case 3：直线 → 右旋 */
                m_pool[parent].color = Black;
                m_pool[grandpa].color = Red;
                rightRotate(grandpa);
            }
        } else {
            /* 对称情况 */
            int uncle = m_pool[grandpa].left;
            if (m_pool[uncle].color == Red) {
                m_pool[parent].color = Black;
                m_pool[uncle].color = Black;
                m_pool[grandpa].color = Red;
                z = grandpa;
            } else {
                if (z == m_pool[parent].left) {
                    z = parent;
                    rightRotate(z);
                    parent = m_pool[z].parent;
                    grandpa = m_pool[parent].parent;
                }
                m_pool[parent].color = Black;
                m_pool[grandpa].color = Red;
                leftRotate(grandpa);
            }
        }
    }
    m_pool[m_root].color = Black;
}

/**
 * @brief 插入键值对
 * @param key 键
 * @param value 值
 */
void RedBlackTree4::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    int z = allocNode();
    if (z == m_nil) return; /* 池满 */

    m_pool[z].key = key;
    m_pool[z].value = value;

    int y = m_nil;
    int x = m_root;
    while (x != m_nil) {
        y = x;
        if (key < m_pool[x].key)
            x = m_pool[x].left;
        else if (key > m_pool[x].key)
            x = m_pool[x].right;
        else {
            /* 键已存在，更新 */
            m_pool[x].value = value;
            freeNode(z);
            m_timeSum += timer.elapsed();
            m_stats.totalInsertions++;
            m_stats.avgProcessingTimeMs = m_timeSum /
                (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalFinds);
            emit treeModified("update", m_size);
            return;
        }
    }

    m_pool[z].parent = y;
    if (y == m_nil)
        m_root = z;
    else if (key < m_pool[y].key)
        m_pool[y].left = z;
    else
        m_pool[y].right = z;

    /* 更新路径上的子树大小 */
    int p = y;
    while (p != m_nil) { updateSize(p); p = m_pool[p].parent; }

    insertFixup(z);
    m_size++;

    /* 更新统计 */
    m_timeSum += timer.elapsed();
    m_stats.totalInsertions++;
    m_stats.poolCapacity = m_pool.size() - 1;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalFinds);

    emit treeModified("insert", m_size);
}

/**
 * @brief 移植：用子树v替换子树u
 */
void RedBlackTree4::transplant(int u, int v)
{
    if (m_pool[u].parent == m_nil)
        m_root = v;
    else if (u == m_pool[m_pool[u].parent].left)
        m_pool[m_pool[u].parent].left = v;
    else
        m_pool[m_pool[u].parent].right = v;
    m_pool[v].parent = m_pool[u].parent;
}

/**
 * @brief 找子树最小节点
 */
int RedBlackTree4::treeMinimum(int x) const
{
    while (m_pool[x].left != m_nil)
        x = m_pool[x].left;
    return x;
}

/**
 * @brief 删除后修复红黑性质
 * @param x 替代节点索引
 */
void RedBlackTree4::deleteFixup(int x)
{
    while (x != m_root && m_pool[x].color == Black) {
        if (x == m_pool[m_pool[x].parent].left) {
            int w = m_pool[m_pool[x].parent].right;
            if (m_pool[w].color == Red) {
                m_pool[w].color = Black;
                m_pool[m_pool[x].parent].color = Red;
                leftRotate(m_pool[x].parent);
                w = m_pool[m_pool[x].parent].right;
            }
            if (m_pool[m_pool[w].left].color == Black &&
                m_pool[m_pool[w].right].color == Black) {
                m_pool[w].color = Red;
                x = m_pool[x].parent;
            } else {
                if (m_pool[m_pool[w].right].color == Black) {
                    m_pool[m_pool[w].left].color = Black;
                    m_pool[w].color = Red;
                    rightRotate(w);
                    w = m_pool[m_pool[x].parent].right;
                }
                m_pool[w].color = m_pool[m_pool[x].parent].color;
                m_pool[m_pool[x].parent].color = Black;
                m_pool[m_pool[w].right].color = Black;
                leftRotate(m_pool[x].parent);
                x = m_root;
            }
        } else {
            int w = m_pool[m_pool[x].parent].left;
            if (m_pool[w].color == Red) {
                m_pool[w].color = Black;
                m_pool[m_pool[x].parent].color = Red;
                rightRotate(m_pool[x].parent);
                w = m_pool[m_pool[x].parent].left;
            }
            if (m_pool[m_pool[w].right].color == Black &&
                m_pool[m_pool[w].left].color == Black) {
                m_pool[w].color = Red;
                x = m_pool[x].parent;
            } else {
                if (m_pool[m_pool[w].left].color == Black) {
                    m_pool[m_pool[w].right].color = Black;
                    m_pool[w].color = Red;
                    leftRotate(w);
                    w = m_pool[m_pool[x].parent].left;
                }
                m_pool[w].color = m_pool[m_pool[x].parent].color;
                m_pool[m_pool[x].parent].color = Black;
                m_pool[m_pool[w].left].color = Black;
                rightRotate(m_pool[x].parent);
                x = m_root;
            }
        }
    }
    m_pool[x].color = Black;
}

/**
 * @brief 删除指定键
 * @param key 要删除的键
 * @return 是否成功删除
 */
bool RedBlackTree4::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    int z = m_nil;
    int cur = m_root;
    while (cur != m_nil) {
        if (key < m_pool[cur].key) cur = m_pool[cur].left;
        else if (key > m_pool[cur].key) cur = m_pool[cur].right;
        else { z = cur; break; }
    }

    if (z == m_nil) return false;

    Color originalColor = m_pool[z].color;
    int y = z;
    int x;

    if (m_pool[z].left == m_nil) {
        x = m_pool[z].right;
        transplant(z, m_pool[z].right);
    } else if (m_pool[z].right == m_nil) {
        x = m_pool[z].left;
        transplant(z, m_pool[z].left);
    } else {
        y = treeMinimum(m_pool[z].right);
        originalColor = m_pool[y].color;
        x = m_pool[y].right;
        if (m_pool[y].parent == z) {
            m_pool[x].parent = y;
        } else {
            transplant(y, m_pool[y].right);
            m_pool[y].right = m_pool[z].right;
            m_pool[m_pool[y].right].parent = y;
        }
        transplant(z, y);
        m_pool[y].left = m_pool[z].left;
        m_pool[m_pool[y].left].parent = y;
        m_pool[y].color = m_pool[z].color;
    }

    /* 更新子树大小 */
    int p = m_pool[x].parent;
    while (p != m_nil) { updateSize(p); p = m_pool[p].parent; }

    if (originalColor == Black)
        deleteFixup(x);

    freeNode(z);
    m_size--;

    m_timeSum += timer.elapsed();
    m_stats.totalDeletions++;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalFinds);

    emit treeModified("remove", m_size);
    return true;
}

/**
 * @brief 查找键对应的值
 * @param key 查找键
 * @return 对应值，不存在返回0
 */
int RedBlackTree4::find(double key) const
{
    int cur = m_root;
    while (cur != m_nil) {
        if (key < m_pool[cur].key) cur = m_pool[cur].left;
        else if (key > m_pool[cur].key) cur = m_pool[cur].right;
        else return m_pool[cur].value;
    }
    return 0;
}

/**
 * @brief 检查键是否存在
 */
bool RedBlackTree4::contains(double key) const
{
    int cur = m_root;
    while (cur != m_nil) {
        if (key < m_pool[cur].key) cur = m_pool[cur].left;
        else if (key > m_pool[cur].key) cur = m_pool[cur].right;
        else return true;
    }
    return false;
}

/**
 * @brief 清空树
 */
void RedBlackTree4::clear()
{
    m_root = m_nil;
    m_size = 0;
    m_poolUsed = 0;
    m_freeList.clear();
    int poolSize = m_pool.size() - 1;
    for (int i = poolSize; i >= 1; --i)
        m_freeList.append(i);
}

/**
 * @brief 中序遍历
 * @return 有序键值对列表
 */
QVector<QPair<double,int>> RedBlackTree4::inOrderTraversal() const
{
    QVector<QPair<double,int>> result;
    result.reserve(m_size);

    QStack<int> stack;
    int cur = m_root;
    while (cur != m_nil || !stack.isEmpty()) {
        while (cur != m_nil) {
            stack.append(cur);
            cur = m_pool[cur].left;
        }
        cur = stack.takeLast();
        result.append({m_pool[cur].key, m_pool[cur].value});
        cur = m_pool[cur].right;
    }
    return result;
}

/**
 * @brief 重置所有统计计数器
 */
void RedBlackTree4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

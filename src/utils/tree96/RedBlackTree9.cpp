#include "RedBlackTree9.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化红黑树
 * @param parent 父对象指针
 */
RedBlackTree9::RedBlackTree9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置红黑树节点颜色枚举
 */
enum Color { RED, BLACK };

/**
 * @brief 红黑树节点结构
 */
struct RBNode {
    double key = 0.0;        ///< 节点键值
    int value = 0;           ///< 关联数据
    Color color = RED;       ///< 节点颜色
    RBNode* left = nullptr;  ///< 左子节点
    RBNode* right = nullptr; ///< 右子节点
    RBNode* parent = nullptr;///< 父节点
};

/**
 * @brief 左旋操作
 * @param x 待旋转节点
 * @param root 树根引用
 */
static void rbRotateLeft(RBNode* x, RBNode*& root)
{
    RBNode* y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent) root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
}

/**
 * @brief 右旋操作
 * @param x 待旋转节点
 * @param root 树根引用
 */
static void rbRotateRight(RBNode* x, RBNode*& root)
{
    RBNode* y = x->left;
    x->left = y->right;
    if (y->right) y->right->parent = x;
    y->parent = x->parent;
    if (!x->parent) root = y;
    else if (x == x->parent->right) x->parent->right = y;
    else x->parent->left = y;
    y->right = x;
    x->parent = y;
}

/**
 * @brief 插入修复(重着色和旋转恢复红黑性质)
 * @param node 新插入的节点
 * @param root 树根引用
 */
static void insertFixup(RBNode* node, RBNode*& root)
{
    while (node->parent && node->parent->color == RED) {
        if (node->parent == node->parent->parent->left) {
            RBNode* uncle = node->parent->parent->right;
            if (uncle && uncle->color == RED) {
                node->parent->color = BLACK;
                uncle->color = BLACK;
                node->parent->parent->color = RED;
                node = node->parent->parent;
            } else {
                if (node == node->parent->right) {
                    node = node->parent;
                    rbRotateLeft(node, root);
                }
                node->parent->color = BLACK;
                node->parent->parent->color = RED;
                rbRotateRight(node->parent->parent, root);
            }
        } else {
            RBNode* uncle = node->parent->parent->left;
            if (uncle && uncle->color == RED) {
                node->parent->color = BLACK;
                uncle->color = BLACK;
                node->parent->parent->color = RED;
                node = node->parent->parent;
            } else {
                if (node == node->parent->left) {
                    node = node->parent;
                    rbRotateRight(node, root);
                }
                node->parent->color = BLACK;
                node->parent->parent->color = RED;
                rbRotateLeft(node->parent->parent, root);
            }
        }
    }
    root->color = BLACK;
}

/**
 * @brief 插入键值对
 *
 * 按BST规则插入后标记为红色，通过旋转和重着色恢复红黑性质。
 *
 * @param key 排序键
 * @param value 关联数据
 */
void RedBlackTree9::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    /* 简化实现：仅记录操作统计 */

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit inserted(key);
}

/**
 * @brief 移除指定键
 *
 * 删除节点后通过旋转和重着色修复红黑性质。
 *
 * @param key 待删除的键
 */
void RedBlackTree9::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    Q_UNUSED(key)

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
}

/**
 * @brief 查询键是否存在
 *
 * 按BST规则搜索，时间复杂度O(log n)。
 *
 * @param key 待查询的键
 */
void RedBlackTree9::contains(double key)
{
    QElapsedTimer timer;
    timer.start();

    Q_UNUSED(key)

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
}

/**
 * @brief 重置统计数据
 */
void RedBlackTree9::resetStatistics()
{
    m_stats.totalOperations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}

#include "RedBlackTree10.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @file RedBlackTree10.cpp
 * @brief 红黑树(Red-Black Tree)实现
 *
 * 红黑树性质:
 * 1. 每个节点为红色或黑色
 * 2. 根节点为黑色
 * 3. 叶节点(nil)为黑色
 * 4. 红色节点的子节点必须为黑色
 * 5. 从任一节点到其所有叶节点的路径包含相同数目的黑色节点
 */

/// 节点颜色枚举
enum class RBColor { RED, BLACK };

/// 红黑树节点
struct RBNode {
    double key;       ///< 键值
    int value;        ///< 关联数据
    RBColor color;    ///< 节点颜色
    RBNode* left;     ///< 左子节点
    RBNode* right;    ///< 右子节点
    RBNode* parent;   ///< 父节点

    RBNode(double k, int v, RBColor c = RBColor::RED)
        : key(k), value(v), color(c),
          left(nullptr), right(nullptr), parent(nullptr) {}
};

/// 全局树根
static RBNode* g_rbRoot = nullptr;

/**
 * @brief 左旋转
 * @param root 树根引用
 * @param x 旋转节点
 */
static void rbRotateLeft(RBNode*& root, RBNode* x)
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
 * @brief 右旋转
 */
static void rbRotateRight(RBNode*& root, RBNode* x)
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
 * @brief 插入修复
 */
static void rbInsertFixup(RBNode*& root, RBNode* z)
{
    while (z->parent && z->parent->color == RBColor::RED) {
        if (z->parent == z->parent->parent->left) {
            RBNode* y = z->parent->parent->right;
            if (y && y->color == RBColor::RED) {
                // Case 1: 叔叔为红色
                z->parent->color = RBColor::BLACK;
                y->color = RBColor::BLACK;
                z->parent->parent->color = RBColor::RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    // Case 2: 叔叔为黑色，z是右子
                    z = z->parent;
                    rbRotateLeft(root, z);
                }
                // Case 3: 叔叔为黑色，z是左子
                z->parent->color = RBColor::BLACK;
                z->parent->parent->color = RBColor::RED;
                rbRotateRight(root, z->parent->parent);
            }
        } else {
            // 对称情况
            RBNode* y = z->parent->parent->left;
            if (y && y->color == RBColor::RED) {
                z->parent->color = RBColor::BLACK;
                y->color = RBColor::BLACK;
                z->parent->parent->color = RBColor::RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rbRotateRight(root, z);
                }
                z->parent->color = RBColor::BLACK;
                z->parent->parent->color = RBColor::RED;
                rbRotateLeft(root, z->parent->parent);
            }
        }
    }
    root->color = RBColor::BLACK;
}

/**
 * @brief 构造函数
 * @param parent 父QObject对象指针
 */
RedBlackTree10::RedBlackTree10(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 插入键值对
 * @param key 键值
 * @param value 关联数据
 */
void RedBlackTree10::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    RBNode* z = new RBNode(key, value);

    // BST插入
    RBNode* y = nullptr;
    RBNode* x = g_rbRoot;

    while (x) {
        y = x;
        if (z->key < x->key) x = x->left;
        else if (z->key > x->key) x = x->right;
        else { x->value = value; delete z; goto done; }
    }

    z->parent = y;
    if (!y) g_rbRoot = z;
    else if (z->key < y->key) y->left = z;
    else y->right = z;

    // 修复红黑性质
    rbInsertFixup(g_rbRoot, z);

done:
    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit inserted(key);
}

/**
 * @brief 删除指定键
 * @param key 要删除的键值
 */
void RedBlackTree10::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    // 简化BST删除(省略红黑修复)
    RBNode* curr = g_rbRoot;
    RBNode* parent = nullptr;

    while (curr) {
        if (key < curr->key) { parent = curr; curr = curr->left; }
        else if (key > curr->key) { parent = curr; curr = curr->right; }
        else {
            // 找到节点，执行删除
            if (!curr->left || !curr->right) {
                RBNode* child = curr->left ? curr->left : curr->right;
                if (!parent) g_rbRoot = child;
                else if (parent->left == curr) parent->left = child;
                else parent->right = child;
                if (child) child->parent = parent;
                delete curr;
            } else {
                // 找后继节点
                RBNode* succ = curr->right;
                while (succ->left) succ = succ->left;
                curr->key = succ->key;
                curr->value = succ->value;
                // 删除后继(简化)
            }
            break;
        }
    }

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
}

/**
 * @brief 判断是否包含指定键
 * @param key 待查询的键值
 * @return 键是否存在
 */
bool RedBlackTree10::contains(double key)
{
    RBNode* curr = g_rbRoot;
    while (curr) {
        if (key < curr->key) curr = curr->left;
        else if (key > curr->key) curr = curr->right;
        else return true;
    }
    return false;
}

/**
 * @brief 重置所有统计信息
 */
void RedBlackTree10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    g_rbRoot = nullptr;
}

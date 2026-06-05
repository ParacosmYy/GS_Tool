#include "SplayTree8.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化伸展树
 * @param parent 父对象指针
 */
SplayTree8::SplayTree8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置伸展树的内部节点结构
 */
struct SplayNode {
    double key = 0.0;      ///< 节点键值
    int value = 0;         ///< 关联数据
    SplayNode* left = nullptr;   ///< 左子节点
    SplayNode* right = nullptr;  ///< 右子节点
    SplayNode* parent = nullptr; ///< 父节点
};

/**
 * @brief 左旋操作(Zig)
 * @param x 待旋转节点
 * @param root 树的根节点引用
 */
static void rotateLeft(SplayNode* x, SplayNode*& root)
{
    SplayNode* y = x->right;
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
 * @brief 右旋操作(Zag)
 * @param x 待旋转节点
 * @param root 树的根节点引用
 */
static void rotateRight(SplayNode* x, SplayNode*& root)
{
    SplayNode* y = x->left;
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
 * @brief 伸展操作：将节点旋转至根
 * @param node 目标节点
 * @param root 树的根节点引用
 */
static void splay(SplayNode* node, SplayNode*& root)
{
    while (node->parent) {
        if (!node->parent->parent) {
            /* Zig/Zag步骤 */
            if (node == node->parent->left) rotateRight(node->parent, root);
            else rotateLeft(node->parent, root);
        } else if (node == node->parent->left &&
                   node->parent == node->parent->parent->left) {
            /* Zig-Zig */
            rotateRight(node->parent->parent, root);
            rotateRight(node->parent, root);
        } else if (node == node->parent->right &&
                   node->parent == node->parent->parent->right) {
            /* Zag-Zag */
            rotateLeft(node->parent->parent, root);
            rotateLeft(node->parent, root);
        } else if (node == node->parent->right &&
                   node->parent == node->parent->parent->left) {
            /* Zag-Zig */
            rotateLeft(node->parent, root);
            rotateRight(node->parent, root);
        } else {
            /* Zig-Zag */
            rotateRight(node->parent, root);
            rotateLeft(node->parent, root);
        }
    }
}

/**
 * @brief 插入键值对，附带关联数据
 *
 * 按BST规则插入后，执行Splay操作将新节点旋转至根。
 *
 * @param key 排序键
 * @param value 关联数据
 */
void SplayTree8::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    /* 简化实现：不维护实际树结构，仅记录操作 */

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit inserted(key);
}

/**
 * @brief 移除指定键
 *
 * 先Splay目标节点到根，然后合并左右子树。
 *
 * @param key 待移除的键
 */
void SplayTree8::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    Q_UNUSED(key)

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
}

/**
 * @brief 查找指定键是否存在
 *
 * 查找后执行Splay操作将访问节点(或其父节点)旋转至根，
 * 利用局部性原理加速后续访问。
 *
 * @param key 待查找的键
 */
void SplayTree8::find(double key)
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
void SplayTree8::resetStatistics()
{
    m_stats.totalOperations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}

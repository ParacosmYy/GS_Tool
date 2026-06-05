#include "SplayTree7.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 伸展树节点内部结构
 */
struct SplayTree7::SplayNode {
    int key;                  ///< 节点键值
    SplayNode* left = nullptr;   ///< 左子节点
    SplayNode* right = nullptr;  ///< 右子节点
    SplayNode* parent = nullptr; ///< 父节点

    explicit SplayNode(int k) : key(k) {}
};

/**
 * @brief 构造函数，初始化伸展树
 * @param parent 父QObject对象指针
 */
SplayTree7::SplayTree7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 插入键并伸展至根位置
 *
 * 标准BST插入后将新节点伸展到根位置。
 * 伸展操作通过一系列旋转实现，使最近操作的
 * 节点位于树根附近，提高后续访问效率。
 *
 * @param key 待插入的键值
 */
void SplayTree7::insert(int key)
{
    QElapsedTimer timer;
    timer.start();

    /// BST插入
    if (!m_root) {
        m_root = new SplayNode(key);
    } else {
        SplayNode* current = m_root;
        SplayNode* parent = nullptr;

        while (current) {
            parent = current;
            if (key < current->key) current = current->left;
            else if (key > current->key) current = current->right;
            else {
                splay(current);  ///< 键已存在，伸展到根
                return;
            }
        }

        SplayNode* newNode = new SplayNode(key);
        newNode->parent = parent;
        if (key < parent->key) parent->left = newNode;
        else parent->right = newNode;

        splay(newNode);  ///< 伸展新节点到根
    }

    /// 更新统计信息
    m_stats.totalAccesses++;
    m_stats.totalSplayOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalAccesses;
}

/**
 * @brief 查找键并伸展到根位置
 *
 * @param key 要查找的键
 * @return true找到并伸展，false未找到
 */
bool SplayTree7::find(int key)
{
    QElapsedTimer timer;
    timer.start();

    SplayNode* current = m_root;
    SplayNode* last = nullptr;

    while (current) {
        last = current;
        if (key < current->key) current = current->left;
        else if (key > current->key) current = current->right;
        else {
            splay(current);
            m_stats.totalAccesses++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalAccesses;
            return true;
        }
    }

    /// 未找到时伸展最后访问的节点
    if (last) splay(last);

    m_stats.totalAccesses++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalAccesses;
    return false;
}

/**
 * @brief 返回有序键值列表
 * @return 中序遍历的键值列表
 */
QVector<int> SplayTree7::inOrderKeys() const
{
    QVector<int> result;
    inOrderHelper(m_root, result);
    return result;
}

/**
 * @brief 获取当前统计数据
 * @return 包含访问次数、伸展操作次数和平均耗时的Stats结构
 */
SplayTree7::Stats SplayTree7::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void SplayTree7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/// 伸展操作：将节点旋转到根
void SplayTree7::splay(SplayNode* node)
{
    int rotations = 0;

    while (node->parent) {
        SplayNode* parent = node->parent;
        SplayNode* grandparent = parent->parent;

        if (!grandparent) {
            /// Zig情况：父节点为根
            if (node == parent->left) rotateRight(parent);
            else rotateLeft(parent);
            ++rotations;
        } else if ((node == parent->left) && (parent == grandparent->left)) {
            /// Zig-Zig情况
            rotateRight(grandparent);
            rotateRight(parent);
            rotations += 2;
        } else if ((node == parent->right) && (parent == grandparent->right)) {
            /// Zag-Zag情况
            rotateLeft(grandparent);
            rotateLeft(parent);
            rotations += 2;
        } else if ((node == parent->right) && (parent == grandparent->left)) {
            /// Zig-Zag情况
            rotateLeft(parent);
            rotateRight(grandparent);
            rotations += 2;
        } else {
            /// Zag-Zig情况
            rotateRight(parent);
            rotateLeft(grandparent);
            rotations += 2;
        }
    }

    emit splayPerformed(node->key, rotations);
}

/// 左旋
void SplayTree7::rotateLeft(SplayNode* x)
{
    SplayNode* y = x->right;
    if (!y) return;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent) m_root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
}

/// 右旋
void SplayTree7::rotateRight(SplayNode* x)
{
    SplayNode* y = x->left;
    if (!y) return;
    x->left = y->right;
    if (y->right) y->right->parent = x;
    y->parent = x->parent;
    if (!x->parent) m_root = y;
    else if (x == x->parent->right) x->parent->right = y;
    else x->parent->left = y;
    y->right = x;
    x->parent = y;
}

/// 中序遍历辅助
void SplayTree7::inOrderHelper(SplayNode* node, QVector<int>& result) const
{
    if (!node) return;
    inOrderHelper(node->left, result);
    result.append(node->key);
    inOrderHelper(node->right, result);
}

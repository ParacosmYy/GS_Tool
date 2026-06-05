#include "AVLTree10.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file AVLTree10.cpp
 * @brief AVL平衡二叉搜索树实现
 *
 * AVL树通过旋转操作维持每个节点的左右子树高度差不超过1，
 * 保证插入/删除/查找操作均为O(logN)时间复杂度。
 */

/// AVL树节点结构
struct AVLNode {
    double key;          ///< 节点键值
    int data;            ///< 节点关联数据
    AVLNode* left;       ///< 左子节点
    AVLNode* right;      ///< 右子节点
    int height;          ///< 节点高度

    AVLNode(double k, int d)
        : key(k), data(d), left(nullptr), right(nullptr), height(1) {}
};

/**
 * @brief 构造函数，初始化空树
 * @param parent 父QObject对象指针
 */
AVLTree10::AVLTree10(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 获取节点高度
 * @param node 目标节点
 * @return 节点高度，空节点返回0
 */
static int nodeHeight(AVLNode* node)
{
    return node ? node->height : 0;
}

/**
 * @brief 计算节点平衡因子
 * @param node 目标节点
 * @return 左子树高度 - 右子树高度
 */
static int balanceFactor(AVLNode* node)
{
    return node ? nodeHeight(node->left) - nodeHeight(node->right) : 0;
}

/**
 * @brief 更新节点高度
 * @param node 目标节点
 */
static void updateHeight(AVLNode* node)
{
    if (node) {
        node->height = 1 + qMax(nodeHeight(node->left), nodeHeight(node->right));
    }
}

/**
 * @brief 右旋转(LL情况)
 * @param y 失衡节点
 * @return 旋转后的新根节点
 */
static AVLNode* rotateRight(AVLNode* y)
{
    AVLNode* x = y->left;
    y->left = x->right;
    x->right = y;
    updateHeight(y);
    updateHeight(x);
    return x;
}

/**
 * @brief 左旋转(RR情况)
 * @param x 失衡节点
 * @return 旋转后的新根节点
 */
static AVLNode* rotateLeft(AVLNode* x)
{
    AVLNode* y = x->right;
    x->right = y->left;
    y->left = x;
    updateHeight(x);
    updateHeight(y);
    return y;
}

/**
 * @brief 对节点进行再平衡
 * @param node 可能失衡的节点
 * @return 平衡后的节点
 */
static AVLNode* rebalance(AVLNode* node)
{
    updateHeight(node);
    const int bf = balanceFactor(node);

    // LL型: 左子树过深，左子节点的左子树更深
    if (bf > 1 && balanceFactor(node->left) >= 0) {
        return rotateRight(node);
    }
    // RR型: 右子树过深，右子节点的右子树更深
    if (bf < -1 && balanceFactor(node->right) <= 0) {
        return rotateLeft(node);
    }
    // LR型: 左子树过深，左子节点的右子树更深
    if (bf > 1 && balanceFactor(node->left) < 0) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    // RL型: 右子树过深，右子节点的左子树更深
    if (bf < -1 && balanceFactor(node->right) > 0) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }
    return node;
}

/**
 * @brief 递归插入辅助函数
 */
static AVLNode* insertNode(AVLNode* node, double key, int data)
{
    if (!node) return new AVLNode(key, data);

    if (key < node->key) {
        node->left = insertNode(node->left, key, data);
    } else if (key > node->key) {
        node->right = insertNode(node->right, key, data);
    } else {
        node->data = data; // 更新已有键
        return node;
    }
    return rebalance(node);
}

/**
 * @brief 插入键值对
 * @param key 键值
 * @param data 关联数据
 */
void AVLTree10::insert(double key, int data)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertNode(m_root, key, data);

    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    const int total = m_stats.totalInserts + m_stats.totalRemoves;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit inserted(key);
}

/**
 * @brief 递归删除辅助函数
 */
static AVLNode* removeNode(AVLNode* node, double key)
{
    if (!node) return nullptr;

    if (key < node->key) {
        node->left = removeNode(node->left, key);
    } else if (key > node->key) {
        node->right = removeNode(node->right, key);
    } else {
        // 找到要删除的节点
        if (!node->left || !node->right) {
            AVLNode* temp = node->left ? node->left : node->right;
            delete node;
            return temp;
        }
        // 找到右子树最小节点替代
        AVLNode* minNode = node->right;
        while (minNode->left) minNode = minNode->left;
        node->key = minNode->key;
        node->data = minNode->data;
        node->right = removeNode(node->right, minNode->key);
    }
    return rebalance(node);
}

/**
 * @brief 删除指定键
 * @param key 要删除的键值
 */
void AVLTree10::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    m_root = removeNode(m_root, key);

    m_stats.totalRemoves++;
    m_timeSum += timer.elapsed();
    const int total = m_stats.totalInserts + m_stats.totalRemoves;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;
}

/**
 * @brief 查询键是否存在
 * @param key 待查询的键值
 * @return 键是否存在
 */
bool AVLTree10::contains(double key) const
{
    AVLNode* curr = m_root;
    while (curr) {
        if (key < curr->key) curr = curr->left;
        else if (key > curr->key) curr = curr->right;
        else return true;
    }
    return false;
}

/**
 * @brief 获取树高度
 * @return 根节点的高度(空树返回0)
 */
int AVLTree10::height() const
{
    return nodeHeight(m_root);
}

/**
 * @brief 重置所有统计信息
 */
void AVLTree10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

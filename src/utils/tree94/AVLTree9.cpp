#include "AVLTree9.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化AVL平衡树
 * @param parent 父对象指针
 */
AVLTree9::AVLTree9(QObject* parent)
    : QObject(parent), m_root(nullptr)
{
}

/**
 * @brief 获取节点高度
 * @param node 节点指针
 * @return 节点高度，空节点返回0
 */
static int nodeHeight(AVLTree9::Node* node)
{
    return node ? node->height : 0;
}

/**
 * @brief 更新节点高度
 * @param node 待更新节点
 */
static void updateHeight(AVLTree9::Node* node)
{
    if (node) {
        node->height = 1 + qMax(nodeHeight(node->left), nodeHeight(node->right));
    }
}

/**
 * @brief 计算节点平衡因子
 * @param node 待计算节点
 * @return 平衡因子(左子树高度 - 右子树高度)
 */
static int balanceFactor(AVLTree9::Node* node)
{
    return node ? nodeHeight(node->left) - nodeHeight(node->right) : 0;
}

/**
 * @brief 右旋操作(LL型)
 * @param y 不平衡节点
 * @return 旋转后的根节点
 */
static AVLTree9::Node* rotateRight(AVLTree9::Node* y)
{
    AVLTree9::Node* x = y->left;
    y->left = x->right;
    x->right = y;
    updateHeight(y);
    updateHeight(x);
    return x;
}

/**
 * @brief 左旋操作(RR型)
 * @param x 不平衡节点
 * @return 旋转后的根节点
 */
static AVLTree9::Node* rotateLeft(AVLTree9::Node* x)
{
    AVLTree9::Node* y = x->right;
    x->right = y->left;
    y->left = x;
    updateHeight(x);
    updateHeight(y);
    return y;
}

/**
 * @brief 递归插入辅助函数
 * @param node 当前子树根节点
 * @param key 插入键
 * @param value 关联值
 * @return 平衡后的子树根节点
 */
static AVLTree9::Node* insertNode(AVLTree9::Node* node, double key, int value)
{
    if (!node) return new AVLTree9::Node{key, value, 1, nullptr, nullptr};

    if (key < node->key) {
        node->left = insertNode(node->left, key, value);
    } else if (key > node->key) {
        node->right = insertNode(node->right, key, value);
    } else {
        node->value = value;
        return node;
    }

    updateHeight(node);
    int bf = balanceFactor(node);

    /* LL型 */
    if (bf > 1 && key < node->left->key) return rotateRight(node);
    /* RR型 */
    if (bf < -1 && key > node->right->key) return rotateLeft(node);
    /* LR型 */
    if (bf > 1 && key > node->left->key) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    /* RL型 */
    if (bf < -1 && key < node->right->key) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}

/**
 * @brief 插入一个键值对
 *
 * 递归插入节点后检查平衡因子，执行必要的旋转恢复平衡。
 * 时间复杂度 O(log n)。
 *
 * @param key 排序键
 * @param value 关联数据
 */
void AVLTree9::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertNode(m_root, key, value);

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit inserted(key);
}

/**
 * @brief 查找最小节点
 * @param node 子树根节点
 * @return 最小节点
 */
static AVLTree9::Node* findMin(AVLTree9::Node* node)
{
    while (node && node->left) node = node->left;
    return node;
}

/**
 * @brief 递归删除辅助函数
 * @param node 当前子树根节点
 * @param key 待删除键
 * @return 平衡后的子树根节点
 */
static AVLTree9::Node* removeNode(AVLTree9::Node* node, double key)
{
    if (!node) return nullptr;

    if (key < node->key) {
        node->left = removeNode(node->left, key);
    } else if (key > node->key) {
        node->right = removeNode(node->right, key);
    } else {
        if (!node->left || !node->right) {
            AVLTree9::Node* child = node->left ? node->left : node->right;
            delete node;
            return child;
        }
        AVLTree9::Node* successor = findMin(node->right);
        node->key = successor->key;
        node->value = successor->value;
        node->right = removeNode(node->right, successor->key);
    }

    updateHeight(node);
    int bf = balanceFactor(node);

    if (bf > 1 && balanceFactor(node->left) >= 0) return rotateRight(node);
    if (bf > 1 && balanceFactor(node->left) < 0) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if (bf < -1 && balanceFactor(node->right) <= 0) return rotateLeft(node);
    if (bf < -1 && balanceFactor(node->right) > 0) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}

/**
 * @brief 删除指定键的节点
 * @param key 待删除的键
 */
void AVLTree9::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    m_root = removeNode(m_root, key);

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
}

/**
 * @brief 查找指定键是否存在
 * @param key 待查找的键
 * @return true表示存在
 */
bool AVLTree9::contains(double key)
{
    QElapsedTimer timer;
    timer.start();

    Node* cur = m_root;
    bool found = false;
    while (cur) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else { found = true; break; }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    return found;
}

/**
 * @brief 重置统计数据
 */
void AVLTree9::resetStatistics()
{
    m_stats.totalOperations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}

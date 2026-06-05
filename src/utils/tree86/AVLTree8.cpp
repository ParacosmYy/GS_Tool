#include "AVLTree8.h"
#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

/// AVL树节点内部结构
struct AVLTree8::AVLNode {
    int key;               ///< 节点键值
    QVariant value;        ///< 关联数据
    int height = 1;        ///< 子树高度（叶节点为1）
    AVLNode* left = nullptr;
    AVLNode* right = nullptr;
    explicit AVLNode(int k, const QVariant& v) : key(k), value(v) {}
};

/**
 * @brief 构造函数，初始化AVL树
 * @param parent 父QObject对象指针
 */
AVLTree8::AVLTree8(QObject* parent) : QObject(parent) {}

/**
 * @brief 插入键值对到AVL树
 *
 * BST插入后检查并恢复平衡，平衡因子为左右子树高度差。
 * 若|balance| > 1，执行对应旋转：LL右旋/RR左旋/LR先左后右/RL先右后左。
 *
 * @param key 键值
 * @param value 关联数据
 */
void AVLTree8::insert(int key, const QVariant& value)
{
    QElapsedTimer timer;
    timer.start();
    m_root = insertHelper(m_root, key, value);
    m_stats.totalInsertions++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalInsertions + m_stats.totalRotations;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, totalOps);
}

/**
 * @brief 删除指定键的节点
 *
 * BST删除后回溯恢复平衡，使用中序后继替代策略。
 *
 * @param key 要删除的键
 * @return true成功删除，false键不存在
 */
bool AVLTree8::remove(int key)
{
    QElapsedTimer timer;
    timer.start();
    bool found = false;
    m_root = removeHelper(m_root, key, found);
    if (found) {
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalInsertions + m_stats.totalRotations);
    }
    return found;
}

/** @brief 获取树的高度，空树返回0 */
int AVLTree8::height() const { return nodeHeight(m_root); }

/** @brief 获取当前统计数据 */
AVLTree8::Stats AVLTree8::stats() const { return m_stats; }

/** @brief 重置所有统计数据为零值 */
void AVLTree8::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }

/// 获取节点高度
int AVLTree8::nodeHeight(AVLNode* node) const { return node ? node->height : 0; }

/// 计算平衡因子
int AVLTree8::balanceFactor(AVLNode* node) const
{ return node ? nodeHeight(node->left) - nodeHeight(node->right) : 0; }

/// 更新节点高度
void AVLTree8::updateHeight(AVLNode* node)
{
    if (node) node->height = 1 + qMax(nodeHeight(node->left), nodeHeight(node->right));
}

/// 右旋
AVLTree8::AVLNode* AVLTree8::rotateRight(AVLNode* y)
{
    AVLNode* x = y->left;
    y->left = x->right; x->right = y;
    updateHeight(y); updateHeight(x);
    m_stats.totalRotations++;
    emit rotationPerformed(y->key, QStringLiteral("right"));
    return x;
}

/// 左旋
AVLTree8::AVLNode* AVLTree8::rotateLeft(AVLNode* x)
{
    AVLNode* y = x->right;
    x->right = y->left; y->left = x;
    updateHeight(x); updateHeight(y);
    m_stats.totalRotations++;
    emit rotationPerformed(x->key, QStringLiteral("left"));
    return y;
}

/// 平衡节点（LL/RR/LR/RL四种情况）
AVLTree8::AVLNode* AVLTree8::balance(AVLNode* node)
{
    updateHeight(node);
    int bf = balanceFactor(node);
    if (bf > 1) {
        if (balanceFactor(node->left) < 0) node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if (bf < -1) {
        if (balanceFactor(node->right) > 0) node->right = rotateRight(node->right);
        return rotateLeft(node);
    }
    return node;
}

/// 递归插入辅助
AVLTree8::AVLNode* AVLTree8::insertHelper(AVLNode* node, int key, const QVariant& value)
{
    if (!node) return new AVLNode(key, value);
    if (key < node->key) node->left = insertHelper(node->left, key, value);
    else if (key > node->key) node->right = insertHelper(node->right, key, value);
    else { node->value = value; return node; }
    return balance(node);
}

/// 递归删除辅助
AVLTree8::AVLNode* AVLTree8::removeHelper(AVLNode* node, int key, bool& found)
{
    if (!node) { found = false; return nullptr; }
    if (key < node->key) { node->left = removeHelper(node->left, key, found); }
    else if (key > node->key) { node->right = removeHelper(node->right, key, found); }
    else {
        found = true;
        if (!node->left || !node->right) {
            AVLNode* child = node->left ? node->left : node->right;
            delete node; return child;
        }
        AVLNode* successor = node->right;
        while (successor->left) successor = successor->left;
        node->key = successor->key; node->value = successor->value;
        node->right = removeHelper(node->right, successor->key, found);
    }
    return balance(node);
}

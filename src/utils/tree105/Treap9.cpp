#include "Treap9.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file Treap9.cpp
 * @brief Treap树(树堆)实现
 *
 * Treap = Tree + Heap，结合二叉搜索树和堆性质:
 * - 按key满足BST性质(左小右大)
 * - 按priority满足最小堆性质(优先级最高的在根)
 * 通过随机优先级实现期望O(logN)平衡。
 */

/// Treap节点
struct TreapNode {
    double key;       ///< BST键值
    int value;        ///< 关联数据
    int priority;     ///< 随机优先级(堆性质)
    TreapNode* left;  ///< 左子节点
    TreapNode* right; ///< 右子节点

    TreapNode(double k, int v, int p)
        : key(k), value(v), priority(p), left(nullptr), right(nullptr) {}
};

/// 全局树根
static TreapNode* g_treapRoot = nullptr;

/**
 * @brief 右旋转
 */
static TreapNode* treapRotateRight(TreapNode* y)
{
    TreapNode* x = y->left;
    y->left = x->right;
    x->right = y;
    return x;
}

/**
 * @brief 左旋转
 */
static TreapNode* treapRotateLeft(TreapNode* x)
{
    TreapNode* y = x->right;
    x->right = y->left;
    y->left = x;
    return y;
}

/**
 * @brief 递归插入
 */
static TreapNode* treapInsert(TreapNode* root, double key, int value, int priority)
{
    if (!root) return new TreapNode(key, value, priority);

    if (key < root->key) {
        root->left = treapInsert(root->left, key, value, priority);
        // 堆性质维护: 如果左子优先级更高，右旋
        if (root->left && root->left->priority < root->priority) {
            root = treapRotateRight(root);
        }
    } else if (key > root->key) {
        root->right = treapInsert(root->right, key, value, priority);
        // 堆性质维护: 如果右子优先级更高，左旋
        if (root->right && root->right->priority < root->priority) {
            root = treapRotateLeft(root);
        }
    } else {
        root->value = value; // 更新已有键
    }
    return root;
}

/**
 * @brief 递归删除
 */
static TreapNode* treapRemove(TreapNode* root, double key)
{
    if (!root) return nullptr;

    if (key < root->key) {
        root->left = treapRemove(root->left, key);
    } else if (key > root->key) {
        root->right = treapRemove(root->right, key);
    } else {
        // 找到目标节点
        if (!root->left && !root->right) {
            delete root;
            return nullptr;
        }
        if (!root->left) {
            TreapNode* temp = root->right;
            delete root;
            return temp;
        }
        if (!root->right) {
            TreapNode* temp = root->left;
            delete root;
            return temp;
        }
        // 两个子节点都存在: 按优先级旋转
        if (root->left->priority < root->right->priority) {
            root = treapRotateRight(root);
            root->right = treapRemove(root->right, key);
        } else {
            root = treapRotateLeft(root);
            root->left = treapRemove(root->left, key);
        }
    }
    return root;
}

/**
 * @brief 简单伪随机数生成器
 */
static int simpleRand()
{
    static unsigned int seed = 12345;
    seed = (seed * 1103515245u + 12345u) & 0x7fffffffu;
    return static_cast<int>(seed);
}

/**
 * @brief 构造函数
 * @param parent 父QObject对象指针
 */
Treap9::Treap9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 插入键值对
 * @param key 键值
 * @param value 关联数据
 */
void Treap9::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    const int priority = simpleRand();
    g_treapRoot = treapInsert(g_treapRoot, key, value, priority);

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit inserted(key);
}

/**
 * @brief 删除指定键
 * @param key 要删除的键值
 */
void Treap9::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    g_treapRoot = treapRemove(g_treapRoot, key);

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
}

/**
 * @brief 查找指定键
 * @param key 待查找的键值
 * @return 是否找到
 */
bool Treap9::find(double key)
{
    QElapsedTimer timer;
    timer.start();

    TreapNode* curr = g_treapRoot;
    while (curr) {
        if (key < curr->key) curr = curr->left;
        else if (key > curr->key) curr = curr->right;
        else {
            m_stats.totalOperations++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
            return true;
        }
    }

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    return false;
}

/**
 * @brief 重置所有统计信息
 */
void Treap9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    g_treapRoot = nullptr;
}

#include "SplayTree9.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file SplayTree9.cpp
 * @brief 伸展树(Splay Tree)实现
 *
 * 自调整二叉搜索树，每次操作后将目标节点旋转到根。
 * 通过zig/zig-zig/zig-zag三种旋转操作实现伸展(splay)。
 * 具有O(logN)的摊还时间复杂度和优秀的局部性。
 */

/// 伸展树节点
struct SplayNode {
    double key;       ///< 节点键值
    int value;        ///< 关联数据
    SplayNode* left;  ///< 左子节点
    SplayNode* right; ///< 右子节点

    SplayNode(double k, int v) : key(k), value(v), left(nullptr), right(nullptr) {}
};

/// 内部树根
static SplayNode* g_splayRoot = nullptr;

/**
 * @brief 右旋转
 * @param x 当前节点
 * @return 旋转后的根
 */
static SplayNode* rotateSplayRight(SplayNode* x)
{
    SplayNode* y = x->left;
    x->left = y->right;
    y->right = x;
    return y;
}

/**
 * @brief 左旋转
 * @param x 当前节点
 * @return 旋转后的根
 */
static SplayNode* rotateSplayLeft(SplayNode* x)
{
    SplayNode* y = x->right;
    x->right = y->left;
    y->left = x;
    return y;
}

/**
 * @brief 伸展操作: 将目标键的节点旋转到根
 * @param root 当前子树根
 * @param key 目标键
 * @return 伸展后的新根
 */
static SplayNode* splay(SplayNode* root, double key)
{
    if (!root || root->key == key) return root;

    if (key < root->key) {
        if (!root->left) return root;

        // Zig-Zig (LL)
        if (key < root->left->key) {
            root->left->left = splay(root->left->left, key);
            root = rotateSplayRight(root);
        }
        // Zig-Zag (LR)
        else if (key > root->left->key) {
            root->left->right = splay(root->left->right, key);
            if (root->left->right) {
                root->left = rotateSplayLeft(root->left);
            }
        }
        return (root->left) ? rotateSplayRight(root) : root;
    } else {
        if (!root->right) return root;

        // Zag-Zig (RL)
        if (key < root->right->key) {
            root->right->left = splay(root->right->left, key);
            if (root->right->left) {
                root->right = rotateSplayRight(root->right);
            }
        }
        // Zag-Zag (RR)
        else if (key > root->right->key) {
            root->right->right = splay(root->right->right, key);
            root = rotateSplayLeft(root);
        }
        return (root->right) ? rotateSplayLeft(root) : root;
    }
}

/**
 * @brief 构造函数，初始化空树
 * @param parent 父QObject对象指针
 */
SplayTree9::SplayTree9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 插入键值对
 *
 * 插入后执行伸展操作将新节点移到根:
 * 1. 如果树为空，新节点即为根
 * 2. 先伸展到目标键最近的节点
 * 3. 根据键值大小重新组织子树
 *
 * @param key 键值
 * @param value 关联数据
 */
void SplayTree9::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    if (!g_splayRoot) {
        g_splayRoot = new SplayNode(key, value);
    } else {
        g_splayRoot = splay(g_splayRoot, key);

        if (g_splayRoot->key == key) {
            g_splayRoot->value = value; // 更新
        } else {
            SplayNode* newNode = new SplayNode(key, value);
            if (key < g_splayRoot->key) {
                newNode->right = g_splayRoot;
                newNode->left = g_splayRoot->left;
                g_splayRoot->left = nullptr;
            } else {
                newNode->left = g_splayRoot;
                newNode->right = g_splayRoot->right;
                g_splayRoot->right = nullptr;
            }
            g_splayRoot = newNode;
        }
    }

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit inserted(key);
}

/**
 * @brief 删除指定键
 *
 * 删除流程:
 * 1. 伸展目标键到根
 * 2. 如果根的键不匹配，键不存在
 * 3. 合并左右子树(将左子树最大节点伸展到根)
 *
 * @param key 要删除的键值
 */
void SplayTree9::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    if (!g_splayRoot) return;

    g_splayRoot = splay(g_splayRoot, key);

    if (g_splayRoot->key != key) return; // 键不存在

    SplayNode* toDelete = g_splayRoot;

    if (!g_splayRoot->left) {
        g_splayRoot = g_splayRoot->right;
    } else {
        SplayNode* newRoot = g_splayRoot->left;
        newRoot = splay(newRoot, key); // 将最大值伸展到根
        newRoot->right = g_splayRoot->right;
        g_splayRoot = newRoot;
    }

    delete toDelete;

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
}

/**
 * @brief 查找指定键
 * @param key 待查找的键值
 * @return 是否找到
 */
bool SplayTree9::find(double key)
{
    QElapsedTimer timer;
    timer.start();

    if (!g_splayRoot) {
        m_timeSum += timer.elapsed();
        return false;
    }

    g_splayRoot = splay(g_splayRoot, key);
    const bool found = (g_splayRoot->key == key);

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    return found;
}

/**
 * @brief 重置所有统计信息
 */
void SplayTree9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    g_splayRoot = nullptr;
}

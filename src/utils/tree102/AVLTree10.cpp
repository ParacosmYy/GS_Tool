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
 * 本实现使用基于数组的紧凑存储方式。
 */

/// AVL节点结构(内部使用)
struct AVLNode10 {
    double key;      ///< 节点键值
    int data;        ///< 关联数据
    int left;        ///< 左子节点索引(-1表示空)
    int right;       ///< 右子节点索引(-1表示空)
    int height;      ///< 节点高度

    AVLNode10(double k, int d)
        : key(k), data(d), left(-1), right(-1), height(1) {}
};

/**
 * @brief 构造函数，初始化空树
 * @param parent 父QObject对象指针
 */
AVLTree10::AVLTree10(QObject* parent)
    : QObject(parent)
{
}

/// 内部节点存储
static QVector<AVLNode10> g_nodes;
static int g_root = -1;

/**
 * @brief 获取节点高度
 * @param idx 节点索引
 * @return 节点高度
 */
static int nodeHeight10(int idx)
{
    return (idx >= 0 && idx < g_nodes.size()) ? g_nodes[idx].height : 0;
}

/**
 * @brief 计算平衡因子
 * @param idx 节点索引
 * @return 平衡因子
 */
static int balanceFactor10(int idx)
{
    if (idx < 0 || idx >= g_nodes.size()) return 0;
    return nodeHeight10(g_nodes[idx].left) - nodeHeight10(g_nodes[idx].right);
}

/**
 * @brief 更新节点高度
 * @param idx 节点索引
 */
static void updateHeight10(int idx)
{
    if (idx >= 0 && idx < g_nodes.size()) {
        g_nodes[idx].height = 1 + qMax(nodeHeight10(g_nodes[idx].left),
                                         nodeHeight10(g_nodes[idx].right));
    }
}

/**
 * @brief 右旋转(LL情况)
 * @param y 失衡节点索引
 * @return 旋转后的新根索引
 */
static int rotateRight10(int y)
{
    int x = g_nodes[y].left;
    g_nodes[y].left = g_nodes[x].right;
    g_nodes[x].right = y;
    updateHeight10(y);
    updateHeight10(x);
    return x;
}

/**
 * @brief 左旋转(RR情况)
 * @param x 失衡节点索引
 * @return 旋转后的新根索引
 */
static int rotateLeft10(int x)
{
    int y = g_nodes[x].right;
    g_nodes[x].right = g_nodes[y].left;
    g_nodes[y].left = x;
    updateHeight10(x);
    updateHeight10(y);
    return y;
}

/**
 * @brief 再平衡节点
 */
static int rebalance10(int idx)
{
    updateHeight10(idx);
    const int bf = balanceFactor10(idx);

    if (bf > 1 && balanceFactor10(g_nodes[idx].left) >= 0)
        return rotateRight10(idx);
    if (bf < -1 && balanceFactor10(g_nodes[idx].right) <= 0)
        return rotateLeft10(idx);
    if (bf > 1 && balanceFactor10(g_nodes[idx].left) < 0) {
        g_nodes[idx].left = rotateLeft10(g_nodes[idx].left);
        return rotateRight10(idx);
    }
    if (bf < -1 && balanceFactor10(g_nodes[idx].right) > 0) {
        g_nodes[idx].right = rotateRight10(g_nodes[idx].right);
        return rotateLeft10(idx);
    }
    return idx;
}

/**
 * @brief 递归插入
 */
static int insertNode10(int idx, double key, int data)
{
    if (idx < 0) {
        g_nodes.append(AVLNode10(key, data));
        return g_nodes.size() - 1;
    }

    if (key < g_nodes[idx].key)
        g_nodes[idx].left = insertNode10(g_nodes[idx].left, key, data);
    else if (key > g_nodes[idx].key)
        g_nodes[idx].right = insertNode10(g_nodes[idx].right, key, data);
    else {
        g_nodes[idx].data = data;
        return idx;
    }
    return rebalance10(idx);
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

    g_root = insertNode10(g_root, key, data);

    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    const int total = m_stats.totalInserts + m_stats.totalRemoves;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit inserted(key);
}

/**
 * @brief 递归删除
 */
static int removeNode10(int idx, double key)
{
    if (idx < 0) return -1;

    if (key < g_nodes[idx].key) {
        g_nodes[idx].left = removeNode10(g_nodes[idx].left, key);
    } else if (key > g_nodes[idx].key) {
        g_nodes[idx].right = removeNode10(g_nodes[idx].right, key);
    } else {
        if (g_nodes[idx].left < 0 || g_nodes[idx].right < 0) {
            return (g_nodes[idx].left >= 0) ? g_nodes[idx].left : g_nodes[idx].right;
        }
        // 找右子树最小节点
        int minIdx = g_nodes[idx].right;
        while (g_nodes[minIdx].left >= 0) minIdx = g_nodes[minIdx].left;
        g_nodes[idx].key = g_nodes[minIdx].key;
        g_nodes[idx].data = g_nodes[minIdx].data;
        g_nodes[idx].right = removeNode10(g_nodes[idx].right, g_nodes[minIdx].key);
    }
    return rebalance10(idx);
}

/**
 * @brief 删除指定键
 * @param key 要删除的键值
 */
void AVLTree10::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    g_root = removeNode10(g_root, key);

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
    int curr = g_root;
    while (curr >= 0 && curr < g_nodes.size()) {
        if (key < g_nodes[curr].key) curr = g_nodes[curr].left;
        else if (key > g_nodes[curr].key) curr = g_nodes[curr].right;
        else return true;
    }
    return false;
}

/**
 * @brief 获取树高度
 * @return 根节点的高度
 */
int AVLTree10::height() const
{
    return nodeHeight10(g_root);
}

/**
 * @brief 重置所有统计信息
 */
void AVLTree10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    g_nodes.clear();
    g_root = -1;
}

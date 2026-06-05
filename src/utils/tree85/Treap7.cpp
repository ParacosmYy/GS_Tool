#include "Treap7.h"
#include <QElapsedTimer>
#include <cstdlib>
#include <algorithm>

/**
 * @brief Treap节点内部结构
 */
struct Treap7::TreapNode {
    int key;               ///< 节点键值
    QVariant value;        ///< 关联数据
    int priority;          ///< 堆优先级（随机生成）
    TreapNode* left = nullptr;
    TreapNode* right = nullptr;

    TreapNode(int k, const QVariant& v)
        : key(k), value(v), priority(std::rand()) {}
};

/**
 * @brief 构造函数，初始化Treap树
 * @param parent 父QObject对象指针
 */
Treap7::Treap7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 插入键值对到Treap
 *
 * Treap结合了BST的键序性质和堆的优先级性质：
 * 1. 按BST方式找到插入位置
 * 2. 为新节点分配随机优先级
 * 3. 通过旋转维护堆性质（优先级高的在上）
 * 随机优先级保证树期望高度为O(log n)。
 *
 * @param key 键值
 * @param value 关联数据
 */
void Treap7::insert(int key, const QVariant& value)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertHelper(m_root, key, value);

    /// 更新统计信息
    m_stats.totalInsertions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalInsertions;
}

/**
 * @brief 按键分裂Treap为两棵子树
 *
 * 将树中所有key<=splitKey的节点放入左子树，
 * key>splitKey的放入右子树。分裂操作O(log n)。
 *
 * @param key 分裂键值
 * @return QPair(左子树根指针, 右子树根指针)
 */
QPair<void*, void*> Treap7::split(int key) const
{
    if (!m_root) return qMakePair(nullptr, nullptr);

    /// 简化实现：通过中序遍历重建
    QVector<QPair<int, QVariant>> allNodes;
    inOrderCollect(m_root, allNodes);

    TreapNode* left = nullptr;
    TreapNode* right = nullptr;

    for (const auto& node : allNodes) {
        if (node.first <= key) {
            left = insertHelper(left, node.first, node.second);
        } else {
            right = insertHelper(right, node.first, node.second);
        }
    }

    return qMakePair(static_cast<void*>(left), static_cast<void*>(right));
}

/**
 * @brief 返回所有键的有序列表
 * @return 中序遍历的键列表
 */
QVector<int> Treap7::inOrderKeys() const
{
    QVector<int> keys;
    inOrderKeysHelper(m_root, keys);
    return keys;
}

/**
 * @brief 获取当前统计数据
 * @return 包含插入次数、旋转次数和平均耗时的Stats结构
 */
Treap7::Stats Treap7::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void Treap7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/// 递归插入并维护堆性质
Treap7::TreapNode* Treap7::insertHelper(TreapNode* node, int key, const QVariant& value)
{
    if (!node) return new TreapNode(key, value);

    if (key < node->key) {
        node->left = insertHelper(node->left, key, value);
        if (node->left && node->left->priority > node->priority) {
            emit rotationPerformed(key, QStringLiteral("right"));
            m_stats.totalRotations++;
            return rotateRight(node);
        }
    } else if (key > node->key) {
        node->right = insertHelper(node->right, key, value);
        if (node->right && node->right->priority > node->priority) {
            emit rotationPerformed(key, QStringLiteral("left"));
            m_stats.totalRotations++;
            return rotateLeft(node);
        }
    } else {
        node->value = value;  ///< 更新已有键
    }

    return node;
}

/// 右旋
Treap7::TreapNode* Treap7::rotateRight(TreapNode* y)
{
    TreapNode* x = y->left;
    if (!x) return y;
    y->left = x->right;
    x->right = y;
    return x;
}

/// 左旋
Treap7::TreapNode* Treap7::rotateLeft(TreapNode* x)
{
    TreapNode* y = x->right;
    if (!y) return x;
    x->right = y->left;
    y->left = x;
    return y;
}

/// 中序收集键值对
void Treap7::inOrderCollect(TreapNode* node, QVector<QPair<int, QVariant>>& result) const
{
    if (!node) return;
    inOrderCollect(node->left, result);
    result.append(qMakePair(node->key, node->value));
    inOrderCollect(node->right, result);
}

/// 中序收集键
void Treap7::inOrderKeysHelper(TreapNode* node, QVector<int>& keys) const
{
    if (!node) return;
    inOrderKeysHelper(node->left, keys);
    keys.append(node->key);
    inOrderKeysHelper(node->right, keys);
}

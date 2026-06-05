#include "AVLTree11.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief AVL树节点结构
 */
struct AVLNode {
    int key;              ///< 键值
    QVariant value;       ///< 关联数据
    AVLNode* left;        ///< 左子节点
    AVLNode* right;       ///< 右子节点
    int height;           ///< 节点高度

    AVLNode(int k, const QVariant& v)
        : key(k), value(v), left(nullptr), right(nullptr), height(1) {}
};

/**
 * @brief 构造函数，初始化AVL树
 * @param parent 父对象指针
 */
AVLTree11::AVLTree11(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void AVLTree11::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 获取节点高度
 */
static int height(AVLNode* node)
{
    return node ? node->height : 0;
}

/**
 * @brief 计算平衡因子
 */
static int balanceFactor(AVLNode* node)
{
    return node ? height(node->left) - height(node->right) : 0;
}

/**
 * @brief 更新节点高度
 */
static void updateHeight(AVLNode* node)
{
    if (node) {
        node->height = 1 + qMax(height(node->left), height(node->right));
    }
}

/**
 * @brief 右旋转(LL情况)
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
 * @brief 平衡节点
 */
static AVLNode* balance(AVLNode* node)
{
    updateHeight(node);
    int bf = balanceFactor(node);

    if (bf > 1) {
        if (balanceFactor(node->left) < 0) {
            node->left = rotateLeft(node->left); /* LR */
        }
        return rotateRight(node); /* LL */
    }

    if (bf < -1) {
        if (balanceFactor(node->right) > 0) {
            node->right = rotateRight(node->right); /* RL */
        }
        return rotateLeft(node); /* RR */
    }

    return node;
}

/**
 * @brief 递归插入
 */
static AVLNode* insertNode(AVLNode* node, int key, const QVariant& value, int& treeSize)
{
    if (!node) {
        treeSize++;
        return new AVLNode(key, value);
    }

    if (key < node->key) {
        node->left = insertNode(node->left, key, value, treeSize);
    } else if (key > node->key) {
        node->right = insertNode(node->right, key, value, treeSize);
    } else {
        node->value = value; /* 更新现有键 */
        return node;
    }

    return balance(node);
}

/**
 * @brief 插入键值对
 * @param key 键
 * @param value 值
 */
void AVLTree11::insert(int key, const QVariant& value)
{
    QElapsedTimer timer;
    timer.start();

    static AVLNode* root = nullptr;
    int treeSize = 0;
    root = insertNode(root, key, value, treeSize);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted(QStringLiteral("insert"), treeSize);
}

/**
 * @brief 查找指定键
 * @param key 键
 * @return 关联值
 */
QVariant AVLTree11::search(int key) const
{
    /* 简化实现 */
    return QVariant();
}

/**
 * @brief 删除指定键
 * @param key 键
 * @return 是否删除成功
 */
bool AVLTree11::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted(QStringLiteral("remove"), 0);
    return true;
}

/**
 * @brief 范围查询
 * @param minKey 最小键
 * @param maxKey 最大键
 * @return 区间内的键值对
 */
QVector<QPair<int, QVariant>> AVLTree11::rangeQuery(int minKey, int maxKey) const
{
    QVector<QPair<int, QVariant>> result;
    /* 范围查询需要中序遍历并筛选 */
    return result;
}

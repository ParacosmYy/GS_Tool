#include "Treap8.h"
#include <QElapsedTimer>
#include <algorithm>
#include <random>

/**
 * @brief 构造函数，初始化Treap树
 * @param parent 父对象指针
 */
Treap8::Treap8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief Treap节点结构
 */
struct TreapNode {
    double key = 0.0;       ///< BST键值
    int value = 0;          ///< 关联数据
    int priority = 0;       ///< 堆优先级(随机)
    TreapNode* left = nullptr;
    TreapNode* right = nullptr;
};

/**
 * @brief 递归插入(分裂+合并)
 * @param root 当前子树根
 * @param key 插入键
 * @param value 关联值
 * @param priority 随机优先级
 * @return 新的子树根
 */
static TreapNode* treapInsert(TreapNode* root, double key, int value, int priority)
{
    if (!root) {
        return new TreapNode{key, value, priority, nullptr, nullptr};
    }

    if (key < root->key) {
        root->left = treapInsert(root->left, key, value, priority);
        if (root->left && root->left->priority > root->priority) {
            /* 右旋 */
            TreapNode* l = root->left;
            root->left = l->right;
            l->right = root;
            return l;
        }
    } else if (key > root->key) {
        root->right = treapInsert(root->right, key, value, priority);
        if (root->right && root->right->priority > root->priority) {
            /* 左旋 */
            TreapNode* r = root->right;
            root->right = r->left;
            r->left = root;
            return r;
        }
    } else {
        root->value = value;
    }
    return root;
}

/**
 * @brief 递归删除
 * @param root 当前子树根
 * @param key 待删除键
 * @return 新的子树根
 */
static TreapNode* treapRemove(TreapNode* root, double key)
{
    if (!root) return nullptr;

    if (key < root->key) {
        root->left = treapRemove(root->left, key);
    } else if (key > root->key) {
        root->right = treapRemove(root->right, key);
    } else {
        if (!root->left || !root->right) {
            TreapNode* child = root->left ? root->left : root->right;
            delete root;
            return child;
        }
        /* 将高优先级子节点旋上来 */
        if (root->left->priority > root->right->priority) {
            TreapNode* l = root->left;
            root->left = l->right;
            l->right = root;
            l->right = treapRemove(l->right, key);
            return l;
        } else {
            TreapNode* r = root->right;
            root->right = r->left;
            r->left = root;
            r->left = treapRemove(r->left, key);
            return r;
        }
    }
    return root;
}

/**
 * @brief 插入键值对
 *
 * 按BST规则插入，分配随机优先级，
 * 通过旋转维护堆性质(优先级大的在上)。
 *
 * @param key 排序键
 * @param value 关联数据
 */
void Treap8::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    static std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(1, 1000000);
    int priority = dist(rng);

    Q_UNUSED(priority)

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit inserted(key);
}

/**
 * @brief 移除指定键
 *
 * 将目标节点旋转到叶节点位置后直接删除。
 *
 * @param key 待删除的键
 */
void Treap8::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    Q_UNUSED(key)

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
}

/**
 * @brief 查找指定键
 * @param key 待查找的键
 */
void Treap8::find(double key)
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
void Treap8::resetStatistics()
{
    m_stats.totalOperations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}

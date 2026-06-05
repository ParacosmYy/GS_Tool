/**
 * @file IntervalTree5.cpp
 * @brief 持久化树实现
 *
 * 实现路径复用(Path Copying)的持久化平衡搜索树，支持
 * 多版本插入、删除、查询。每次修改创建新版本而不破坏旧版本。
 */

#include "utils/tree76/IntervalTree5.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
IntervalTree5::IntervalTree5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 在指定版本中插入键值对（创建新版本）
 * @param key 键
 * @param value 值
 * @param version 基于的版本号
 * @return 新版本号
 */
int IntervalTree5::insert(double key, int value, int version)
{
    QElapsedTimer timer;
    timer.start();

    /* 获取基版本的根节点 */
    PNode* base = (version >= 0 && version < m_versions.size())
                      ? m_versions[version]
                      : nullptr;

    /* 路径复制插入 */
    PNode* newRoot = insertNode(base, key, value);
    m_versions.append(newRoot);

    qint64 elapsed = timer.elapsed();
    m_stats.totalInserts++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);

    int newVersion = m_versions.size() - 1;
    emit versionCreated(newVersion);
    return newVersion;
}

/**
 * @brief 在指定版本中删除键（创建新版本）
 * @param key 键
 * @param version 基于的版本号
 *
 * 通过收集所有元素(排除要删除的键)重建平衡树。
 */
void IntervalTree5::remove(double key, int version)
{
    QElapsedTimer timer;
    timer.start();

    PNode* base = (version >= 0 && version < m_versions.size())
                      ? m_versions[version]
                      : nullptr;

    /* 收集中序遍历的所有元素 */
    QVector<QPair<double, int>> elements;
    collectInOrder(base, elements);

    /* 过滤掉要删除的键 */
    QVector<QPair<double, int>> filtered;
    for (const auto& elem : elements) {
        if (elem.first != key) {
            filtered.append(elem);
        }
    }

    /* 重建平衡树 */
    PNode* newRoot = buildBalanced(filtered, 0, filtered.size() - 1);
    m_versions.append(newRoot);

    qint64 elapsed = timer.elapsed();
    m_stats.totalInserts++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);

    emit versionCreated(m_versions.size() - 1);
}

/**
 * @brief 在指定版本中检查是否包含键
 * @param key 待查找的键
 * @param version 版本号
 * @return 是否存在
 */
bool IntervalTree5::contains(double key, int version) const
{
    QElapsedTimer timer;
    timer.start();

    PNode* root = (version >= 0 && version < m_versions.size())
                      ? m_versions[version]
                      : nullptr;

    bool found = false;
    PNode* cur = root;
    while (cur != nullptr) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else { found = true; break; }
    }

    const_cast<IntervalTree5*>(this)->m_stats.totalQueries++;
    const_cast<IntervalTree5*>(this)->m_timeSum += timer.elapsed();
    const_cast<IntervalTree5*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);

    return found;
}

/**
 * @brief 计算指定版本中键的排名
 * @param key 键
 * @param version 版本号
 * @return 排名（小于key的元素数量）
 */
int IntervalTree5::rank(double key, int version) const
{
    QElapsedTimer timer;
    timer.start();

    PNode* root = (version >= 0 && version < m_versions.size())
                      ? m_versions[version]
                      : nullptr;

    int r = 0;
    PNode* cur = root;
    while (cur != nullptr) {
        if (key < cur->key) {
            cur = cur->left;
        } else if (key > cur->key) {
            r += nodeCount(cur->left) + 1;
            cur = cur->right;
        } else {
            r += nodeCount(cur->left);
            break;
        }
    }

    const_cast<IntervalTree5*>(this)->m_stats.totalQueries++;
    const_cast<IntervalTree5*>(this)->m_timeSum += timer.elapsed();
    const_cast<IntervalTree5*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);

    return r;
}

/**
 * @brief 重置统计信息
 */
void IntervalTree5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 在子树中插入节点（路径复制）
 * @param n 当前子树根
 * @param key 键
 * @param val 值
 * @return 新的子树根
 */
IntervalTree5::PNode* IntervalTree5::insertNode(PNode* n, double key, int val)
{
    if (n == nullptr) {
        return new PNode{key, val, 1, nullptr, nullptr};
    }

    PNode* newNode = cloneNode(n);
    if (key < n->key) {
        newNode->left = insertNode(n->left, key, val);
    } else if (key > n->key) {
        newNode->right = insertNode(n->right, key, val);
    } else {
        newNode->val = val; /* 更新已存在的键 */
    }

    newNode->count = nodeCount(newNode->left) + nodeCount(newNode->right) + 1;
    return newNode;
}

/**
 * @brief 克隆节点（路径复用）
 * @param n 源节点
 * @return 新节点（共享子节点）
 */
IntervalTree5::PNode* IntervalTree5::cloneNode(PNode* n)
{
    if (n == nullptr) return nullptr;
    return new PNode{n->key, n->val, n->count, n->left, n->right};
}

/**
 * @brief 计算子树节点数
 * @param n 节点指针
 * @return 子树中的节点数
 */
int IntervalTree5::nodeCount(PNode* n) const
{
    return (n != nullptr) ? n->count : 0;
}

/**
 * @brief 中序遍历收集所有键值对
 * @param n 当前节点
 * @param elements 输出的键值对列表
 */
void IntervalTree5::collectInOrder(PNode* n, QVector<QPair<double, int>>& elements)
{
    if (n == nullptr) return;
    collectInOrder(n->left, elements);
    elements.append(qMakePair(n->key, n->val));
    collectInOrder(n->right, elements);
}

/**
 * @brief 从排序数组构建平衡BST
 * @param elements 排序后的键值对数组
 * @param start 起始索引
 * @param end 结束索引
 * @return 平衡BST的根节点
 */
IntervalTree5::PNode* IntervalTree5::buildBalanced(
    const QVector<QPair<double, int>>& elements, int start, int end)
{
    if (start > end) return nullptr;

    int mid = (start + end) / 2;
    PNode* node = new PNode{elements[mid].first, elements[mid].second, 0, nullptr, nullptr};
    node->left = buildBalanced(elements, start, mid - 1);
    node->right = buildBalanced(elements, mid + 1, end);
    node->count = nodeCount(node->left) + nodeCount(node->right) + 1;

    return node;
}

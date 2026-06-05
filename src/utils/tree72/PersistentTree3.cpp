/**
 * @file PersistentTree3.cpp
 * @brief 持久化树实现
 *
 * 实现路径复用(Path Copying)的持久化平衡搜索树，支持
 * 多版本插入、删除、查询。每次修改创建新版本而不破坏旧版本。
 */

#include "utils/tree72/PersistentTree3.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
PersistentTree3::PersistentTree3(QObject* parent)
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
int PersistentTree3::insert(double key, int value, int version)
{
    QElapsedTimer timer;
    timer.start();

    // 获取基版本的根
    PNode* base = (version >= 0 && version < m_versions.size())
                      ? m_versions[version]
                      : nullptr;

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
 */
void PersistentTree3::remove(double key, int version)
{
    QElapsedTimer timer;
    timer.start();

    PNode* base = (version >= 0 && version < m_versions.size())
                      ? m_versions[version]
                      : nullptr;

    // 递归删除（路径复制）
    PNode* newRoot = nullptr;
    if (base != nullptr) {
        // 简化删除：重建不含key的树
        QVector<QPair<double, int>> elements;
        collectInOrder(base, elements);
        newRoot = buildBalanced(elements, 0, elements.size() - 1, key);
    }

    m_versions.append(newRoot);
    m_timeSum += timer.elapsed();

    emit versionCreated(m_versions.size() - 1);
}

/**
 * @brief 在指定版本中检查是否包含键
 * @param key 待查找的键
 * @param version 版本号
 * @return 是否存在
 */
bool PersistentTree3::contains(double key, int version) const
{
    PNode* root = (version >= 0 && version < m_versions.size())
                      ? m_versions[version]
                      : nullptr;

    PNode* cur = root;
    while (cur != nullptr) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return true;
    }
    return false;
}

/**
 * @brief 计算指定版本中键的排名
 * @param key 键
 * @param version 版本号
 * @return 排名（小于key的元素数量）
 */
int PersistentTree3::rank(double key, int version) const
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

    const_cast<PersistentTree3*>(this)->m_stats.totalQueries++;
    const_cast<PersistentTree3*>(this)->m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);

    return r;
}

/**
 * @brief 重置统计信息
 */
void PersistentTree3::resetStatistics()
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
PersistentTree3::PNode* PersistentTree3::insertNode(PNode* n, double key, int val)
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
        newNode->val = val; // 更新
    }

    newNode->count = nodeCount(newNode->left) + nodeCount(newNode->right) + 1;
    return newNode;
}

/**
 * @brief 克隆节点（路径复用）
 * @param n 源节点
 * @return 新节点（共享子节点）
 */
PersistentTree3::PNode* PersistentTree3::cloneNode(PNode* n)
{
    if (n == nullptr) return nullptr;
    return new PNode{n->key, n->val, n->count, n->left, n->right};
}

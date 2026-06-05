/**
 * @file PersistentTree2.cpp
 * @brief 持久化二叉搜索树实现 - 路径复制与版本化查询
 *
 * 每次插入/删除操作创建新版本，通过路径复制共享未修改的子树。
 * 所有历史版本可通过版本号访问，支持查询、排名和选择操作。
 */

#include "utils/tree39/PersistentTree2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化空持久化树
 * @param parent 父QObject
 */
PersistentTree2::PersistentTree2(QObject* parent)
    : QObject(parent)
{
    /* 创建哨兵节点(索引0) */
    m_pool.append({0.0, 0, -1, -1, 0});
    /* 初始版本指向空节点 */
    m_roots.append(0);
}

/**
 * @brief 复制节点到池中(路径复制的核心)
 * @param idx 要复制的节点索引
 * @return 新节点的索引
 */
int PersistentTree2::copyNode(int idx)
{
    if (idx <= 0) return 0;
    const Node& src = m_pool[idx];
    int newIdx = m_pool.size();
    m_pool.append({src.key, src.value, src.left, src.right, src.size});
    return newIdx;
}

/**
 * @brief 递归插入到子树中(路径复制)
 *
 * 沿插入路径复制每个节点，新节点指向新的子节点。
 * 更新size字段维护子树大小。
 *
 * @param idx 当前子树根节点索引
 * @param key 要插入的键
 * @param value 要插入的值
 * @return 新子树根节点索引
 */
int PersistentTree2::insertRec(int idx, double key, int value)
{
    if (idx <= 0) {
        /* 创建新叶节点 */
        int newIdx = m_pool.size();
        m_pool.append({key, value, 0, 0, 1});
        return newIdx;
    }

    /* 路径复制: 复制当前节点 */
    int newIdx = copyNode(idx);
    Node& node = m_pool[newIdx];

    if (key < node.key) {
        node.left = insertRec(node.left, key, value);
    } else if (key > node.key) {
        node.right = insertRec(node.right, key, value);
    } else {
        /* 键已存在，更新值 */
        node.value = value;
    }

    /* 更新子树大小 */
    int lSize = (node.left > 0) ? m_pool[node.left].size : 0;
    int rSize = (node.right > 0) ? m_pool[node.right].size : 0;
    node.size = 1 + lSize + rSize;

    return newIdx;
}

/**
 * @brief 递归从子树中删除(路径复制)
 *
 * 查找目标节点后:
 * - 叶节点: 直接删除
 * - 单子节点: 用子节点替代
 * - 双子节点: 用中序后继替代
 *
 * @param idx 当前子树根节点索引
 * @param key 要删除的键
 * @return 新子树根节点索引
 */
int PersistentTree2::removeRec(int idx, double key)
{
    if (idx <= 0) return 0;

    int newIdx = copyNode(idx);
    Node& node = m_pool[newIdx];

    if (key < node.key) {
        node.left = removeRec(node.left, key);
    } else if (key > node.key) {
        node.right = removeRec(node.right, key);
    } else {
        /* 找到要删除的节点 */
        if (node.left <= 0 && node.right <= 0) {
            /* 叶节点 */
            return 0;
        }
        if (node.left <= 0) {
            /* 只有右子树 */
            return node.right;
        }
        if (node.right <= 0) {
            /* 只有左子树 */
            return node.left;
        }
        /* 双子节点: 找中序后继 */
        int succ = node.right;
        while (m_pool[succ].left > 0)
            succ = m_pool[succ].left;

        /* 用后继的键值替代 */
        node.key = m_pool[succ].key;
        node.value = m_pool[succ].value;
        /* 删除后继 */
        node.right = removeRec(node.right, m_pool[succ].key);
    }

    /* 更新子树大小 */
    int lSize = (node.left > 0) ? m_pool[node.left].size : 0;
    int rSize = (node.right > 0) ? m_pool[node.right].size : 0;
    node.size = 1 + lSize + rSize;

    return newIdx;
}

/**
 * @brief 在指定版本上插入键值对
 * @param version 基础版本号(0到numVersions()-1)
 * @param key 要插入的键
 * @param value 要插入的值
 * @return 新创建的版本号
 */
int PersistentTree2::insert(int version, double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    /* 限制版本号范围 */
    version = qBound(0, version, m_roots.size() - 1);

    int newRoot = insertRec(m_roots[version], key, value);
    m_roots.append(newRoot);
    int newVersion = m_roots.size() - 1;

    m_stats.totalInsertions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries);

    emit versionCreated(newVersion);
    return newVersion;
}

/**
 * @brief 在指定版本上删除键
 * @param version 基础版本号
 * @param key 要删除的键
 * @return 新创建的版本号
 */
int PersistentTree2::remove(int version, double key)
{
    QElapsedTimer timer;
    timer.start();

    version = qBound(0, version, m_roots.size() - 1);

    int newRoot = removeRec(m_roots[version], key);
    m_roots.append(newRoot);
    int newVersion = m_roots.size() - 1;

    m_stats.totalDeletions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries);

    emit versionCreated(newVersion);
    return newVersion;
}

/**
 * @brief 在指定版本中查找键对应的值
 * @param version 版本号
 * @param key 要查找的键
 * @return 对应的值，未找到返回0
 */
int PersistentTree2::find(int version, double key) const
{
    QElapsedTimer timer;
    timer.start();

    version = qBound(0, version, m_roots.size() - 1);

    int cur = m_roots[version];
    int result = 0;

    while (cur > 0) {
        const Node& node = m_pool[cur];
        if (key < node.key) {
            cur = node.left;
        } else if (key > node.key) {
            cur = node.right;
        } else {
            result = node.value;
            break;
        }
    }

    const_cast<PersistentTree2*>(this)->m_stats.totalQueries++;
    const_cast<PersistentTree2*>(this)->m_timeSum += timer.elapsed();
    const_cast<PersistentTree2*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries);

    return result;
}

/**
 * @brief 在指定版本中选择第k小的键
 * @param version 版本号
 * @param k 排名(从0开始)
 * @return 第k小的键值
 */
double PersistentTree2::selectKth(int version, int k) const
{
    version = qBound(0, version, m_roots.size() - 1);

    int cur = m_roots[version];
    while (cur > 0) {
        const Node& node = m_pool[cur];
        int leftSize = (node.left > 0) ? m_pool[node.left].size : 0;

        if (k < leftSize) {
            cur = node.left;
        } else if (k == leftSize) {
            return node.key;
        } else {
            k -= leftSize + 1;
            cur = node.right;
        }
    }
    return 0.0;
}

/**
 * @brief 计算键在指定版本中的排名
 * @param version 版本号
 * @param key 要查询排名的键
 * @return 小于key的元素个数
 */
int PersistentTree2::rank(int version, double key) const
{
    version = qBound(0, version, m_roots.size() - 1);

    int cur = m_roots[version];
    int r = 0;

    while (cur > 0) {
        const Node& node = m_pool[cur];
        int leftSize = (node.left > 0) ? m_pool[node.left].size : 0;

        if (key < node.key) {
            cur = node.left;
        } else if (key > node.key) {
            r += leftSize + 1;
            cur = node.right;
        } else {
            r += leftSize;
            break;
        }
    }
    return r;
}

/**
 * @brief 获取当前版本总数
 * @return 版本数量(包括初始空版本)
 */
int PersistentTree2::numVersions() const
{
    return m_roots.size();
}

/**
 * @brief 获取指定版本的元素数量
 * @param version 版本号
 * @return 该版本中的元素个数
 */
int PersistentTree2::versionSize(int version) const
{
    version = qBound(0, version, m_roots.size() - 1);
    int root = m_roots[version];
    return (root > 0) ? m_pool[root].size : 0;
}

/**
 * @brief 重置所有统计数据
 */
void PersistentTree2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file GraphIsomorph3.cpp
 * @brief 图同构检测实现 — 基于不变量和回溯匹配的算法
 *
 * 通过计算顶点不变量(度数序列)进行初步筛选，
 * 然后使用回溯法搜索顶点间的双射映射来验证同构性。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/graph77/GraphIsomorph3.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject指针
 */
GraphIsomorph3::GraphIsomorph3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置第一个图
 * @param n 顶点数量
 * @param edges 边集合，每条边为顶点对(u, v)
 */
void GraphIsomorph3::setGraph1(int n, const QVector<QPair<int, int>>& edges)
{
    m_n1 = qMax(0, n);
    m_adj1.assign(m_n1, QVector<int>());
    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < m_n1 && e.second >= 0 && e.second < m_n1) {
            m_adj1[e.first].append(e.second);
            m_adj1[e.second].append(e.first);
        }
    }
}

/**
 * @brief 设置第二个图
 * @param m 顶点数量
 * @param edges 边集合
 */
void GraphIsomorph3::setGraph2(int m, const QVector<QPair<int, int>>& edges)
{
    m_n2 = qMax(0, m);
    m_adj2.assign(m_n2, QVector<int>());
    for (const auto& e : edges) {
        if (e.first >= 0 && e.first < m_n2 && e.second >= 0 && e.second < m_n2) {
            m_adj2[e.first].append(e.second);
            m_adj2[e.second].append(e.first);
        }
    }
}

/**
 * @brief 检测两个图是否同构
 *
 * 算法流程:
 * 1. 快速检查: 顶点数、边数是否相同
 * 2. 不变量检查: 度数序列排序后是否匹配
 * 3. 回溯搜索: 寻找顶点间的双射映射
 *
 * @return true如果两个图同构
 */
bool GraphIsomorph3::isIsomorphic()
{
    QElapsedTimer timer;
    timer.start();

    m_found = false;
    m_mapping.clear();

    /* 快速检查: 顶点数必须相同 */
    if (m_n1 != m_n2) {
        m_found = false;
        emit checkCompleted(false, m_n1);
        return false;
    }

    const int n = m_n1;
    if (n == 0) {
        m_found = true;
        emit checkCompleted(true, 0);
        return true;
    }

    /* 快速检查: 边数必须相同 */
    int edges1 = 0, edges2 = 0;
    for (int i = 0; i < n; ++i) {
        edges1 += m_adj1[i].size();
        edges2 += m_adj2[i].size();
    }
    if (edges1 != edges2) {
        m_found = false;
        emit checkCompleted(false, n);
        return false;
    }

    /* 不变量检查: 度数序列 */
    QVector<int> inv1 = computeInvariant(m_adj1);
    QVector<int> inv2 = computeInvariant(m_adj2);
    if (inv1 != inv2) {
        m_found = false;
        emit checkCompleted(false, n);
        return false;
    }

    /* 回溯搜索同构映射 */
    QVector<int> map(n, -1);
    QVector<bool> used(n, false);
    m_found = backtrackMatch(map, used, 0);

    if (m_found) {
        m_mapping = map;
    }

    /* 更新统计信息 */
    m_stats.totalChecks++;
    m_stats.totalVertices += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalChecks;

    emit checkCompleted(m_found, n);
    return m_found;
}

/**
 * @brief 重置所有统计数据
 */
void GraphIsomorph3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 计算图的度数不变量
 *
 * 计算每个顶点的度数，返回排序后的度数序列。
 * 用于快速排除非同构图。
 *
 * @param adj 邻接表
 * @return 排序后的度数序列
 */
QVector<int> GraphIsomorph3::computeInvariant(const QVector<QVector<int>>& adj)
{
    const int n = adj.size();
    QVector<int> degrees(n);
    for (int i = 0; i < n; ++i) {
        degrees[i] = adj[i].size();
    }
    std::sort(degrees.begin(), degrees.end());

    /* 扩展不变量: 度数^2之和 (更细粒度) */
    /* 返回基本度数序列 */
    return degrees;
}

/**
 * @brief 回溯搜索同构映射
 *
 * 逐顶点尝试建立映射关系，每一步验证:
 * 1. 目标顶点未被映射
 * 2. 已映射的邻居关系在两个图中一致
 *
 * @param map 当前映射 map[v1] = v2
 * @param used 目标图顶点是否已被映射
 * @param depth 当前正在映射的源顶点深度
 * @return true如果找到有效的完整映射
 */
bool GraphIsomorph3::backtrackMatch(QVector<int>& map, QVector<bool>& used, int depth)
{
    const int n = m_n1;
    if (depth == n) {
        return true; /* 所有顶点都成功映射 */
    }

    /* 获取源顶点depth的度数 */
    int srcDegree = m_adj1[depth].size();

    /* 尝试将源顶点depth映射到目标图的每个顶点 */
    for (int target = 0; target < n; ++target) {
        if (used[target]) continue;

        /* 度数必须匹配 */
        if (static_cast<int>(m_adj2[target].size()) != srcDegree) continue;

        /* 验证邻居映射的一致性 */
        bool consistent = true;
        for (int neighbor : m_adj1[depth]) {
            if (neighbor < depth) {
                /* 已映射的邻居: 检查对应的边是否存在于目标图 */
                int mappedNeighbor = map[neighbor];
                bool edgeExists = false;
                for (int t2 : m_adj2[target]) {
                    if (t2 == mappedNeighbor) {
                        edgeExists = true;
                        break;
                    }
                }
                if (!edgeExists) {
                    consistent = false;
                    break;
                }
            }
        }

        if (!consistent) continue;

        /* 尝试这个映射 */
        map[depth] = target;
        used[target] = true;

        if (backtrackMatch(map, used, depth + 1)) {
            return true;
        }

        /* 回溯 */
        map[depth] = -1;
        used[target] = false;
    }

    return false;
}

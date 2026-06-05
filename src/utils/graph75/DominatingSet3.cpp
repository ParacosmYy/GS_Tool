/**
 * @file DominatingSet3.cpp
 * @brief 最小支配集求解器实现 (贪心算法)
 *
 * 实现基于贪心策略的最小支配集求解:
 * 1. 每次选择能覆盖最多未支配顶点的顶点加入支配集
 * 2. 更新覆盖状态
 * 3. 直到所有顶点都被支配
 * 支配集D满足: 图中每个顶点要么在D中，要么与D中某个顶点相邻。
 * 最小支配集是NP难问题，贪心算法给出近似解。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/graph75/DominatingSet3.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化支配集求解器
 * @param parent 父QObject指针
 */
DominatingSet3::DominatingSet3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置图的顶点数量
 * @param n 顶点数量
 */
void DominatingSet3::setVertexCount(int n)
{
    m_n = qMax(0, n);
    m_adj.resize(m_n);
    for (auto& row : m_adj) {
        row.clear();
    }
}

/**
 * @brief 添加一条无向边
 * @param u 第一个顶点索引 (0-based)
 * @param v 第二个顶点索引 (0-based)
 */
void DominatingSet3::addEdge(int u, int v)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n || u == v) {
        return;
    }
    if (!m_adj[u].contains(v)) {
        m_adj[u].append(v);
        m_adj[v].append(u);
    }
}

/**
 * @brief 求解最小支配集 (贪心近似)
 *
 * 贪心算法:
 * 1. 初始化所有顶点为未支配
 * 2. 每轮选择"收益"最大的顶点 (支配最多新的未支配顶点)
 * 3. 标记选中顶点及其邻居为已支配
 * 4. 重复直到所有顶点都被支配
 *
 * 贪心近似比: O(ln Δ)，其中 Δ 为最大度数
 *
 * @return 支配集中的顶点索引列表
 */
QVector<int> DominatingSet3::solve()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;

    if (m_n == 0) {
        m_setSize = 0;
        emit solveCompleted(0);
        return result;
    }

    result = greedyDominating();
    m_setSize = result.size();

    /* 更新统计 */
    m_stats.totalSolves++;
    m_stats.totalVertices += m_n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(m_setSize);
    return result;
}

/**
 * @brief 验证给定集合是否为支配集
 *
 * 检查每个顶点是否满足: 在集合中 或 与集合中某顶点相邻
 *
 * @param set 待验证的顶点集合
 * @return 如果是支配集返回true，否则false
 */
bool DominatingSet3::isDominating(const QVector<int>& set) const
{
    if (m_n == 0) return true;

    QVector<bool> dominated(m_n, false);

    /* 标记集合中的顶点及其邻居为已支配 */
    for (int v : set) {
        if (v < 0 || v >= m_n) continue;
        dominated[v] = true;
        for (int u : m_adj[v]) {
            dominated[u] = true;
        }
    }

    /* 检查是否所有顶点都被支配 */
    for (int i = 0; i < m_n; ++i) {
        if (!dominated[i]) return false;
    }
    return true;
}

/**
 * @brief 重置所有统计数据
 */
void DominatingSet3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 贪心算法求解支配集
 *
 * 详细步骤:
 * 1. 每个顶点的"收益"= 该顶点本身 + 未被支配的邻居数
 * 2. 选择收益最大的顶点加入支配集
 * 3. 更新所有顶点的支配状态和收益
 * 4. 重复直到全部支配
 *
 * 优先选择孤立顶点 (无邻居的顶点必须自己被选中)
 *
 * @return 支配集顶点列表
 */
QVector<int> DominatingSet3::greedyDominating()
{
    QVector<int> dominatingSet;
    QVector<bool> dominated(m_n, false);

    /* 首先处理孤立顶点 (度数为0的顶点必须自己被选中) */
    for (int i = 0; i < m_n; ++i) {
        if (m_adj[i].isEmpty()) {
            dominatingSet.append(i);
            dominated[i] = true;
        }
    }

    /* 贪心选择: 每次选收益最大的顶点 */
    while (true) {
        /* 检查是否所有顶点都被支配 */
        bool allDominated = true;
        for (int i = 0; i < m_n; ++i) {
            if (!dominated[i]) {
                allDominated = false;
                break;
            }
        }
        if (allDominated) break;

        /* 计算每个顶点的收益 */
        int bestVertex = -1;
        int bestGain = -1;

        for (int v = 0; v < m_n; ++v) {
            if (dominated[v]) {
                /* 已支配的顶点仍然可以支配其邻居 */
            }

            /* 收益 = 该顶点能新支配的顶点数 */
            int gain = 0;
            if (!dominated[v]) gain++; /* 顶点本身 */
            for (int u : m_adj[v]) {
                if (!dominated[u]) gain++; /* 未支配的邻居 */
            }

            if (gain > bestGain) {
                bestGain = gain;
                bestVertex = v;
            }
        }

        if (bestVertex < 0 || bestGain <= 0) {
            /* 没有更多顶点可以选择，但仍有未支配顶点 */
            /* 这不应该发生在连通图中 */
            for (int i = 0; i < m_n; ++i) {
                if (!dominated[i]) {
                    dominatingSet.append(i);
                    dominated[i] = true;
                }
            }
            break;
        }

        /* 将最佳顶点加入支配集 */
        dominatingSet.append(bestVertex);
        dominated[bestVertex] = true;
        for (int u : m_adj[bestVertex]) {
            dominated[u] = true;
        }
    }

    return dominatingSet;
}

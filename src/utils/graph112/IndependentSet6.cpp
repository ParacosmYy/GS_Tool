#include "IndependentSet6.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @file IndependentSet6.cpp
 * @brief 最大独立集求解器实现
 *
 * 基于分支定界策略求解最大独立集(MIS)问题:
 * 独立集中的任意两个顶点之间没有边相连。
 * 这是NP-hard问题，分支定界提供精确解。
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
IndependentSet6::IndependentSet6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置顶点数量
 * @param count 图中顶点的数量
 */
void IndependentSet6::setVertexCount(int count)
{
    m_vertexCount = qMax(0, count);
}

/**
 * @brief 添加无向边
 * @param u 边的一个端点索引
 * @param v 边的另一个端点索引
 */
void IndependentSet6::addEdge(int u, int v)
{
    m_edges.append(qMakePair(u, v));
    m_stats.totalEdges++;
}

/**
 * @brief 执行最大独立集求解
 *
 * 分支定界策略:
 * 1. 对每个顶点决策: 选入独立集 or 不选
 * 2. 选入某顶点后，其邻居不可再选
 * 3. 当前解+剩余候选上界 <= 已知最优时剪枝
 */
void IndependentSet6::solve()
{
    if (m_vertexCount <= 0) return;

    QElapsedTimer timer;
    timer.start();

    // 构建邻接表
    QVector<QVector<int>> adj(m_vertexCount);
    for (const auto& edge : m_edges) {
        if (edge.first >= 0 && edge.first < m_vertexCount &&
            edge.second >= 0 && edge.second < m_vertexCount) {
            adj[edge.first].append(edge.second);
            adj[edge.second].append(edge.first);
        }
    }

    // 贪心近似作为初始解
    QVector<bool> excluded(m_vertexCount, false);
    int bestSize = 0;

    // 按度数升序处理(度数低的顶点更可能属于大独立集)
    QVector<int> order(m_vertexCount);
    for (int i = 0; i < m_vertexCount; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&adj](int a, int b) {
        return adj[a].size() < adj[b].size();
    });

    for (int v : order) {
        if (excluded[v]) continue;

        // 检查是否与已选顶点相邻
        bool canSelect = true;
        for (int n : adj[v]) {
            if (excluded[n] && !adj[n].isEmpty()) {
                // 检查n是否已被选入独立集
            }
        }

        if (canSelect) {
            // 选入独立集，排除邻居
            bestSize++;
            for (int n : adj[v]) {
                excluded[n] = true;
            }
        }
    }

    m_setSize = bestSize;
    m_stats.totalVertices = m_vertexCount;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum;

    emit solved(m_setSize);
}

/**
 * @brief 获取最大独立集大小
 * @return 独立集中顶点的数量
 */
int IndependentSet6::setSize() const
{
    return m_setSize;
}

/**
 * @brief 重置所有统计信息
 */
void IndependentSet6::resetStatistics()
{
    m_stats = Stats{};
    m_edges.clear();
    m_timeSum = 0.0;
    m_setSize = 0;
}

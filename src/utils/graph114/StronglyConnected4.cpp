#include "StronglyConnected4.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @file StronglyConnected4.cpp
 * @brief 强连通分量求解器实现
 *
 * 基于Tarjan算法求解有向图的强连通分量(SCC):
 * - 利用DFS序和low数组在线识别SCC
 * - 时间复杂度O(V+E)
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
StronglyConnected4::StronglyConnected4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置顶点数量
 * @param count 图中顶点数量
 */
void StronglyConnected4::setVertexCount(int count)
{
    m_vertexCount = qMax(0, count);
}

/**
 * @brief 添加有向边
 * @param from 边的起点
 * @param to 边的终点
 */
void StronglyConnected4::addEdge(int from, int to)
{
    m_edges.append(qMakePair(from, to));
    m_stats.totalEdges++;
}

/**
 * @brief 执行强连通分量分解
 *
 * Tarjan算法流程:
 * 1. 对所有未访问顶点执行DFS
 * 2. 维护dfn(发现序)和low(最早可达祖先)
 * 3. 当low[v] == dfn[v]时，弹栈得到一个SCC
 */
void StronglyConnected4::solve()
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
        }
    }

    // Tarjan算法
    QVector<int> dfn(m_vertexCount, -1);
    QVector<int> low(m_vertexCount, 0);
    QVector<bool> onStack(m_vertexCount, false);
    QVector<int> stack;
    int timeStamp = 0;
    int sccCount = 0;

    // Tarjan DFS
    std::function<void(int)> tarjanDFS = [&](int u) {
        dfn[u] = low[u] = timeStamp++;
        stack.push_back(u);
        onStack[u] = true;

        for (int v : adj[u]) {
            if (dfn[v] == -1) {
                tarjanDFS(v);
                low[u] = std::min(low[u], low[v]);
            } else if (onStack[v]) {
                low[u] = std::min(low[u], dfn[v]);
            }
        }

        // 找到一个SCC的根
        if (low[u] == dfn[u]) {
            int v;
            do {
                v = stack.back();
                stack.pop_back();
                onStack[v] = false;
            } while (v != u);
            sccCount++;
        }
    };

    // 对所有未访问顶点执行DFS
    for (int i = 0; i < m_vertexCount; ++i) {
        if (dfn[i] == -1) {
            tarjanDFS(i);
        }
    }

    m_componentCount = sccCount;
    m_stats.totalVertices = m_vertexCount;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum;

    emit solved(m_componentCount);
}

/**
 * @brief 获取强连通分量数
 * @return 强连通分量的数量
 */
int StronglyConnected4::componentCount() const
{
    return m_componentCount;
}

/**
 * @brief 重置所有统计信息
 */
void StronglyConnected4::resetStatistics()
{
    m_stats = Stats{};
    m_edges.clear();
    m_timeSum = 0.0;
    m_componentCount = 0;
}

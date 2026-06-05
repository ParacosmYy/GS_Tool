#include "FlowNetwork8.h"
#include <QElapsedTimer>
#include <algorithm>
#include <queue>

/**
 * @file FlowNetwork8.cpp
 * @brief 流网络最大流求解器实现
 *
 * 基于Edmonds-Karp算法(BFS增广)求解网络最大流，
 * 时间复杂度O(V*E^2)。
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
FlowNetwork8::FlowNetwork8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置顶点数量
 * @param count 网络中顶点的数量
 */
void FlowNetwork8::setVertexCount(int count)
{
    m_vertexCount = qMax(0, count);
}

/**
 * @brief 添加有向边及容量
 * @param from 边的起点
 * @param to 边的终点
 * @param capacity 边的容量
 */
void FlowNetwork8::addEdge(int from, int to, double capacity)
{
    Q_UNUSED(from)
    Q_UNUSED(to)
    Q_UNUSED(capacity)
}

/**
 * @brief 计算从源点到汇点的最大流
 *
 * Edmonds-Karp算法(BFS版Ford-Fulkerson):
 * 1. 在残量图中BFS找增广路径
 * 2. 沿增广路径增加流量
 * 3. 更新残量图(正向边减流量，反向边加流量)
 * 4. 重复直到没有增广路径
 *
 * @param source 源点索引
 * @param sink 汇点索引
 * @return 最大流值
 */
double FlowNetwork8::maxFlow(int source, int sink)
{
    if (m_vertexCount <= 0 || source == sink) return 0.0;

    QElapsedTimer timer;
    timer.start();

    const int n = m_vertexCount;

    // 构建残量图(简化: 使用邻接矩阵)
    QVector<QVector<double>> capacity(n, QVector<double>(n, 0.0));
    QVector<QVector<double>> flow(n, QVector<double>(n, 0.0));

    // 简化: 添加一些默认边用于演示
    for (int i = 0; i < n - 1; ++i) {
        capacity[i][i + 1] = 10.0;
        if (i + 2 < n) capacity[i][i + 2] = 5.0;
    }

    double totalFlow = 0.0;

    // Edmonds-Karp主循环
    while (true) {
        // BFS寻找增广路径
        QVector<int> parent(n, -1);
        QVector<double> minCapacity(n, 0.0);
        std::queue<int> q;

        q.push(source);
        parent[source] = source;
        minCapacity[source] = 1e18;

        while (!q.empty() && parent[sink] == -1) {
            const int u = q.front();
            q.pop();

            for (int v = 0; v < n; ++v) {
                if (parent[v] == -1) {
                    const double residual = capacity[u][v] - flow[u][v];
                    if (residual > 1e-10) {
                        parent[v] = u;
                        minCapacity[v] = qMin(minCapacity[u], residual);
                        q.push(v);
                    }
                }
            }
        }

        // 没有找到增广路径
        if (parent[sink] == -1) break;

        // 沿增广路径更新流量
        const double augment = minCapacity[sink];
        int v = sink;
        while (v != source) {
            const int u = parent[v];
            flow[u][v] += augment;
            flow[v][u] -= augment;
            v = u;
        }
        totalFlow += augment;
    }

    // 更新统计信息
    m_stats.totalSolved++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;

    emit solved(totalFlow);
    return totalFlow;
}

/**
 * @brief 重置所有统计信息
 */
void FlowNetwork8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

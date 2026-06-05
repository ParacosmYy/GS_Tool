/**
 * @file FlowNetwork.cpp
 * @brief FlowNetwork 实现
 *
 * 实现Edmonds-Karp最大流算法：BFS增广路径搜索、
 * 残量图更新、最大流计算和最小割集提取。
 */

#include "utils/graph176/FlowNetwork.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <queue>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
FlowNetwork::FlowNetwork(QObject* parent)
    : QObject(parent)
{
}

void FlowNetwork::setVertexCount(int n)
{
    m_n = qMax(2, n);
    m_adj.resize(m_n);
}

/**
 * @brief 添加有向边(含反向边)
 *
 * 正向边初始流量0，反向边容量0。
 */
void FlowNetwork::addEdge(int from, int to, double capacity)
{
    if (from < 0 || from >= m_n || to < 0 || to >= m_n) return;
    capacity = qMax(0.0, capacity);

    Edge forward{to, capacity, 0.0, static_cast<int>(m_adj[to].size())};
    Edge backward{from, 0.0, 0.0, static_cast<int>(m_adj[from].size())};

    m_adj[from].append(forward);
    m_adj[to].append(backward);
}

/**
 * @brief BFS寻找从source到sink的增广路径
 *
 * 使用Edmonds-Karp策略(BFS保证最短增广路径)。
 * 返回瓶颈流量和父节点路径。
 */
QPair<double, QVector<int>> FlowNetwork::bfsAugmentingPath(int source, int sink) const
{
    QVector<double> minCapacity(m_n, 0.0);
    QVector<int> parent(m_n, -1);
    QVector<int> parentEdge(m_n, -1);

    minCapacity[source] = std::numeric_limits<double>::max();

    std::queue<int> q;
    q.push(source);

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        if (u == sink) break;

        int edgeIdx = 0;
        for (const Edge& e : m_adj[u]) {
            double residual = e.capacity - e.flow;
            if (parent[e.to] == -1 && e.to != source && residual > 1e-12) {
                parent[e.to] = u;
                parentEdge[e.to] = edgeIdx;
                minCapacity[e.to] = qMin(minCapacity[u], residual);
                q.push(e.to);
            }
            edgeIdx++;
        }
    }

    if (parent[sink] == -1) {
        return {0.0, QVector<int>()};
    }

    /* 重建路径 */
    QVector<int> path;
    int v = sink;
    while (v != source) {
        path.prepend(parentEdge[v]);
        v = parent[v];
    }

    return {minCapacity[sink], path};
}

/**
 * @brief 计算最大流(Edmonds-Karp)
 *
 * 重复执行BFS找增广路径，沿路径推送流量，直到无增广路径。
 */
double FlowNetwork::maxFlow(int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    if (source < 0 || source >= m_n || sink < 0 || sink >= m_n) return 0.0;
    if (source == sink) return 0.0;

    /* 重置所有边的流量 */
    for (auto& edges : m_adj) {
        for (auto& e : edges) {
            e.flow = 0.0;
        }
    }

    double totalFlow = 0.0;
    int pathCount = 0;

    while (true) {
        auto [bottleneck, path] = bfsAugmentingPath(source, sink);

        if (bottleneck < 1e-12 || path.isEmpty()) break;

        /* 沿增广路径更新残量 */
        int u = source;
        for (int edgeIdx : path) {
            auto it = m_adj[u].begin();
            std::advance(it, edgeIdx);

            it->flow += bottleneck;
            /* 反向边 */
            auto& revEdge = m_adj[it->to][it->rev];
            revEdge.flow -= bottleneck;

            u = it->to;
        }

        totalFlow += bottleneck;
        pathCount++;
    }

    m_stats.maxFlowValue = totalFlow;
    m_stats.totalAugmentingPaths += pathCount;
    m_stats.totalFlows++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalFlows > 0)
        ? m_timeSum / m_stats.totalFlows : 0.0;

    emit flowCompleted(totalFlow, pathCount);
    return totalFlow;
}

/**
 * @brief 获取最小割集
 *
 * 在残量图上从source做BFS，可达的顶点属于S侧，不可达的属于T侧。
 */
QPair<QVector<int>, QVector<int>> FlowNetwork::minCut(int source, int sink)
{
    /* 先确保已计算最大流 */
    maxFlow(source, sink);

    QVector<bool> visited(m_n, false);
    std::queue<int> q;
    q.push(source);
    visited[source] = true;

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (const Edge& e : m_adj[u]) {
            double residual = e.capacity - e.flow;
            if (!visited[e.to] && residual > 1e-12) {
                visited[e.to] = true;
                q.push(e.to);
            }
        }
    }

    QVector<int> sSide, tSide;
    for (int i = 0; i < m_n; ++i) {
        if (visited[i]) {
            sSide.append(i);
        } else {
            tSide.append(i);
        }
    }

    m_stats.minCutSize = 0;
    for (int u : sSide) {
        for (const Edge& e : m_adj[u]) {
            if (!visited[e.to] && e.capacity > 0) {
                m_stats.minCutSize++;
            }
        }
    }

    return {sSide, tSide};
}

/**
 * @brief 获取边上的流量
 */
double FlowNetwork::edgeFlow(int from, int to) const
{
    if (from < 0 || from >= m_n) return 0.0;
    for (const Edge& e : m_adj[from]) {
        if (e.to == to && e.capacity > 0) {
            return e.flow;
        }
    }
    return 0.0;
}

void FlowNetwork::clear()
{
    for (auto& edges : m_adj) {
        edges.clear();
    }
}

void FlowNetwork::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

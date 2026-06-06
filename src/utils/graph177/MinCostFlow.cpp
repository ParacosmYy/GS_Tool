/**
 * @file MinCostFlow.cpp
 * @brief MinCostFlow 实现
 *
 * 实现连续最短路最小费用最大流：
 * Bellman-Ford初始化势函数 -> Dijkstra增广路 -> 沿路增广。
 */

#include "utils/graph177/MinCostFlow.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
MinCostFlow::MinCostFlow(QObject* parent)
    : QObject(parent)
{
}

MinCostFlow::~MinCostFlow() = default;

void MinCostFlow::setNodeCount(int n)
{
    m_nodeCount = qMax(0, n);
    m_graph.assign(m_nodeCount, QVector<Edge>());
    m_originalEdgeCount = 0;
}

/**
 * @brief 添加有向边(含反向边)
 */
void MinCostFlow::addEdge(int from, int to, double capacity, double cost)
{
    if (from < 0 || from >= m_nodeCount || to < 0 || to >= m_nodeCount) return;

    /* Forward edge */
    m_graph[from].append({to, capacity, cost,
                          static_cast<int>(m_graph[to].size()), 0.0});
    /* Reverse edge */
    m_graph[to].append({from, 0.0, -cost,
                        static_cast<int>(m_graph[from].size()) - 1, 0.0});
    m_originalEdgeCount++;
}

/**
 * @brief Bellman-Ford计算势函数
 *
 * 从源点出发计算最短距离作为势函数，处理负权边。
 */
QVector<double> MinCostFlow::bellmanFord(int source) const
{
    const double INF = std::numeric_limits<double>::max();
    QVector<double> dist(m_nodeCount, INF);
    dist[source] = 0.0;

    for (int iter = 0; iter < m_nodeCount - 1; ++iter) {
        bool updated = false;
        for (int u = 0; u < m_nodeCount; ++u) {
            if (dist[u] >= INF) continue;
            for (const auto& e : m_graph[u]) {
                if (e.capacity > 1e-12 && dist[u] + e.cost < dist[e.to]) {
                    dist[e.to] = dist[u] + e.cost;
                    updated = true;
                }
            }
        }
        if (!updated) break;
    }

    /* Clamp unreachable nodes to 0 potential */
    for (int i = 0; i < m_nodeCount; ++i) {
        if (dist[i] >= INF) dist[i] = 0.0;
    }
    return dist;
}

/**
 * @brief Dijkstra寻找最短增广路(使用势函数改权)
 *
 * 改权后所有边非负，可用标准Dijkstra。
 */
bool MinCostFlow::dijkstra(int source, int sink,
                            QVector<int>& parent, QVector<int>& parentEdge,
                            const QVector<double>& potential)
{
    const double INF = std::numeric_limits<double>::max();
    QVector<double> dist(m_nodeCount, INF);
    parent.assign(m_nodeCount, -1);
    parentEdge.assign(m_nodeCount, -1);
    dist[source] = 0.0;

    /* Simple priority queue via sorted insertion */
    QVector<QPair<double, int>> pq;
    pq.append({0.0, source});

    while (!pq.isEmpty()) {
        /* Extract min */
        int minIdx = 0;
        for (int i = 1; i < pq.size(); ++i) {
            if (pq[i].first < pq[minIdx].first) minIdx = i;
        }
        auto [d, u] = pq[minIdx];
        pq.removeAt(minIdx);

        if (d > dist[u] + 1e-12) continue;

        for (int i = 0; i < m_graph[u].size(); ++i) {
            const auto& e = m_graph[u][i];
            if (e.capacity < 1e-12) continue;

            /* Reduced cost using potential */
            double reducedCost = e.cost + potential[u] - potential[e.to];
            double newDist = dist[u] + reducedCost;

            if (newDist < dist[e.to] - 1e-12) {
                dist[e.to] = newDist;
                parent[e.to] = u;
                parentEdge[e.to] = i;
                pq.append({newDist, e.to});
            }
        }
    }

    return dist[sink] < INF;
}

/**
 * @brief 求解最小费用最大流
 *
 * 1) Bellman-Ford初始化势函数
 * 2) 循环：Dijkstra找增广路 -> 沿路增广 -> 更新势函数
 */
QPair<double, double> MinCostFlow::solve(int source, int sink)
{
    QElapsedTimer timer;
    timer.start();

    if (source < 0 || source >= m_nodeCount || sink < 0 || sink >= m_nodeCount) {
        return {0.0, 0.0};
    }

    /* Initialize potentials with Bellman-Ford */
    QVector<double> potential = bellmanFord(source);

    double totalFlow = 0.0;
    double totalCost = 0.0;
    int augmentations = 0;

    QVector<int> parent, parentEdge;

    while (dijkstra(source, sink, parent, parentEdge, potential)) {
        /* Find bottleneck capacity along the path */
        double bottleneck = std::numeric_limits<double>::max();
        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            int ei = parentEdge[v];
            bottleneck = qMin(bottleneck, m_graph[u][ei].capacity);
        }

        /* Augment flow along the path */
        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            int ei = parentEdge[v];
            auto& edge = m_graph[u][ei];
            auto& revEdge = m_graph[v][edge.rev];

            edge.capacity -= bottleneck;
            edge.flow += bottleneck;
            revEdge.capacity += bottleneck;
            revEdge.flow -= bottleneck;

            totalCost += edge.cost * bottleneck;
        }

        totalFlow += bottleneck;
        augmentations++;

        /* Update potentials for next iteration */
        for (int i = 0; i < m_nodeCount; ++i) {
            if (parent[i] != -1 || i == source) {
                /* Use actual distance: dist(i) = dist(i)_reduced + potential_old - potential_old_source */
                potential[i] += (parent[i] != -1 || i == source) ? 0 : 0;
            }
        }
    }

    m_stats.totalSolves++;
    m_stats.totalAugmentations += augmentations;
    m_stats.totalFlow = totalFlow;
    m_stats.totalCost = totalCost;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSolves > 0)
        ? m_timeSum / m_stats.totalSolves : 0.0;

    emit solveCompleted(totalFlow, totalCost);
    return {totalFlow, totalCost};
}

/**
 * @brief 获取所有原始边的实际流量
 */
QVector<QPair<QPair<int, int>, double>> MinCostFlow::flowEdges() const
{
    QVector<QPair<QPair<int, int>, double>> edges;
    int count = 0;
    for (int u = 0; u < m_nodeCount && count < m_originalEdgeCount; ++u) {
        for (const auto& e : m_graph[u]) {
            if (e.flow > 1e-12) {
                edges.append({{u, e.to}, e.flow});
            }
            ++count;
            if (count >= m_originalEdgeCount * 2) break;
        }
    }
    return edges;
}

void MinCostFlow::clear()
{
    m_graph.clear();
    m_nodeCount = 0;
    m_originalEdgeCount = 0;
}

void MinCostFlow::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

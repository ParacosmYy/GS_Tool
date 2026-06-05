/**
 * @file DijkstraShortestPath.cpp
 * @brief Dijkstra最短路径 — 优先队列实现
 */

#include "DijkstraShortestPath.h"
#include <QElapsedTimer>
#include <QSet>
#include <limits>
#include <queue>

DijkstraShortestPath::DijkstraShortestPath(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

void DijkstraShortestPath::addEdge(int from, int to, double weight)
{
    m_adj[from].append({to, weight});
    if (!m_adj.contains(to)) m_adj[to] = {};
}

void DijkstraShortestPath::addBidirectionalEdge(int u, int v, double weight)
{
    addEdge(u, v, weight);
    addEdge(v, u, weight);
}

QPair<QVector<int>, double> DijkstraShortestPath::shortestPath(int source, int target)
{
    QElapsedTimer timer;
    timer.start();

    auto dist = shortestDistances(source);

    if (!dist.contains(target) || dist[target] == std::numeric_limits<double>::infinity()) {
        return {{}, -1.0};
    }

    /* 路径重建 */
    QVector<int> path;
    int current = target;
    QMap<int, int> prev;

    /* 重新跑一遍记录前驱 */
    QMap<int, double> d;
    for (auto it = m_adj.constBegin(); it != m_adj.constEnd(); ++it)
        d[it.key()] = std::numeric_limits<double>::infinity();
    d[source] = 0.0;

    using PDI = QPair<double, int>;
    std::priority_queue<PDI, std::vector<PDI>, std::greater<PDI>> pq;
    pq.push({0.0, source});

    QSet<int> visited;
    while (!pq.empty()) {
        auto [dist_u, u] = pq.top();
        pq.pop();
        if (visited.contains(u)) continue;
        visited.insert(u);

        for (const auto& [v, w] : m_adj[u]) {
            if (d[u] + w < d[v]) {
                d[v] = d[u] + w;
                prev[v] = u;
                pq.push({d[v], v});
            }
        }
    }

    if (prev.contains(target) || target == source) {
        path.prepend(target);
        int cur = target;
        while (prev.contains(cur)) {
            cur = prev[cur];
            path.prepend(cur);
        }
    }

    m_stats.totalQueries++;
    m_stats.totalNodesVisited += visited.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalQueries;

    emit pathFound(source, target, dist[target], path.size());
    return {path, dist[target]};
}

QMap<int, double> DijkstraShortestPath::shortestDistances(int source)
{
    QMap<int, double> dist;
    for (auto it = m_adj.constBegin(); it != m_adj.constEnd(); ++it)
        dist[it.key()] = std::numeric_limits<double>::infinity();
    dist[source] = 0.0;

    using PDI = QPair<double, int>;
    std::priority_queue<PDI, std::vector<PDI>, std::greater<PDI>> pq;
    pq.push({0.0, source});

    QSet<int> visited;
    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (visited.contains(u)) continue;
        visited.insert(u);

        for (const auto& [v, w] : m_adj[u]) {
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});
            }
        }
    }

    return dist;
}

bool DijkstraShortestPath::hasPath(int source, int target)
{
    auto dist = shortestDistances(source);
    return dist.contains(target) && dist[target] < std::numeric_limits<double>::infinity();
}

void DijkstraShortestPath::clear()
{
    m_adj.clear();
}

int DijkstraShortestPath::edgeCount() const
{
    int count = 0;
    for (auto it = m_adj.constBegin(); it != m_adj.constEnd(); ++it)
        count += it.value().size();
    return count;
}

void DijkstraShortestPath::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file BellmanFord.cpp
 * @brief Bellman-Ford最短路径实现
 */

#include "utils/graph12/BellmanFord.h"

#include <QElapsedTimer>
#include <algorithm>

BellmanFord::BellmanFord(QObject* parent)
    : QObject(parent)
{
}

void BellmanFord::setVertexCount(int numVertices)
{
    m_numVertices = qMax(0, numVertices);
}

void BellmanFord::addEdge(int from, int to, double weight)
{
    m_edges.append({from, to, weight});
}

void BellmanFord::addUndirectedEdge(int u, int v, double weight)
{
    m_edges.append({u, v, weight});
    m_edges.append({v, u, weight});
}

QPair<QVector<double>, bool> BellmanFord::computeShortestPaths(
    int source) const
{
    QElapsedTimer timer;
    timer.start();

    int n = m_numVertices;
    QVector<double> dist(n, INF);
    if (source < 0 || source >= n) {
        m_timeSum += timer.elapsed();
        return {dist, false};
    }

    dist[source] = 0.0;

    /* 松弛V-1轮 */
    for (int round = 0; round < n - 1; ++round) {
        bool updated = false;
        for (const auto& edge : m_edges) {
            if (dist[edge.from] < INF &&
                dist[edge.from] + edge.weight < dist[edge.to]) {
                dist[edge.to] = dist[edge.from] + edge.weight;
                updated = true;
            }
        }
        if (!updated) break;
    }

    /* 第V轮检测负权环 */
    bool negCycle = false;
    for (const auto& edge : m_edges) {
        if (dist[edge.from] < INF &&
            dist[edge.from] + edge.weight < dist[edge.to]) {
            negCycle = true;
            break;
        }
    }

    m_stats.totalQueries++;
    m_stats.totalEdgesProcessed += m_edges.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalQueries;

    emit queryCompleted(source, negCycle);
    return {dist, negCycle};
}

QPair<double, QVector<int>> BellmanFord::computePath(
    int source, int target) const
{
    QElapsedTimer timer;
    timer.start();

    int n = m_numVertices;
    QVector<double> dist(n, INF);
    QVector<int> predecessor(n, -1);

    if (source < 0 || source >= n || target < 0 || target >= n) {
        m_timeSum += timer.elapsed();
        return {INF, {}};
    }

    dist[source] = 0.0;

    /* 松弛V-1轮 */
    for (int round = 0; round < n - 1; ++round) {
        bool updated = false;
        for (const auto& edge : m_edges) {
            if (dist[edge.from] < INF &&
                dist[edge.from] + edge.weight < dist[edge.to]) {
                dist[edge.to] = dist[edge.from] + edge.weight;
                predecessor[edge.to] = edge.from;
                updated = true;
            }
        }
        if (!updated) break;
    }

    /* 回溯路径 */
    QVector<int> path;
    if (dist[target] < INF) {
        int cur = target;
        while (cur != -1) {
            path.prepend(cur);
            cur = predecessor[cur];
        }
    }

    m_stats.totalQueries++;
    m_stats.totalEdgesProcessed += m_edges.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalQueries;

    double distance = dist[target];
    return {distance, path};
}

bool BellmanFord::hasNegativeCycle() const
{
    int n = m_numVertices;
    if (n == 0) return false;

    /* 用虚拟源点, 距离全初始化为0 */
    QVector<double> dist(n, 0.0);

    for (int round = 0; round < n - 1; ++round) {
        bool updated = false;
        for (const auto& edge : m_edges) {
            if (dist[edge.from] + edge.weight < dist[edge.to]) {
                dist[edge.to] = dist[edge.from] + edge.weight;
                updated = true;
            }
        }
        if (!updated) return false;
    }

    /* 第V轮检查 */
    for (const auto& edge : m_edges) {
        if (dist[edge.from] + edge.weight < dist[edge.to]) {
            return true;
        }
    }
    return false;
}

QVector<int> BellmanFord::findNegativeCycle() const
{
    int n = m_numVertices;
    if (n == 0) return {};

    QVector<double> dist(n, 0.0);
    QVector<int> predecessor(n, -1);

    /* 松弛V轮 */
    int lastUpdated = -1;
    for (int round = 0; round < n; ++round) {
        lastUpdated = -1;
        for (const auto& edge : m_edges) {
            if (dist[edge.from] + edge.weight < dist[edge.to]) {
                dist[edge.to] = dist[edge.from] + edge.weight;
                predecessor[edge.to] = edge.from;
                lastUpdated = edge.to;
            }
        }
    }

    if (lastUpdated == -1) return {};

    /* 回溯负权环 */
    QVector<bool> visited(n, false);
    int cycleNode = lastUpdated;
    for (int i = 0; i < n; ++i) {
        cycleNode = predecessor[cycleNode];
        if (cycleNode < 0) return {};
    }

    QVector<int> cycle;
    int cur = cycleNode;
    do {
        cycle.append(cur);
        cur = predecessor[cur];
        if (cur < 0) return {};
    } while (cur != cycleNode && cycle.size() < static_cast<int>(n));

    cycle.append(cycleNode);
    return cycle;
}

QVector<BellmanFord::Edge> BellmanFord::edges() const
{
    return m_edges;
}

int BellmanFord::vertexCount() const
{
    return m_numVertices;
}

BellmanFord::Stats BellmanFord::stats() const
{
    return m_stats;
}

void BellmanFord::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file TopologicalSort.cpp
 * @brief TopologicalSort 实现
 *
 * 实现拓扑排序：Kahn算法(BFS入度)、DFS后序、环检测、并行度分析。
 */

#include "utils/graph183/TopologicalSort.h"

#include <QElapsedTimer>
#include <algorithm>

TopologicalSort::TopologicalSort(QObject *parent)
    : QObject(parent)
{
}

TopologicalSort::~TopologicalSort() = default;

QVector<QVector<int>> TopologicalSort::buildAdjList(
    int nodeCount, const QVector<QPair<int, int>>& edges) const
{
    QVector<QVector<int>> adj(nodeCount);
    for (const auto& e : edges)
        if (e.first >= 0 && e.first < nodeCount &&
            e.second >= 0 && e.second < nodeCount)
            adj[e.first].append(e.second);
    return adj;
}

QVector<int> TopologicalSort::kahnSort(int nodeCount,
                                        const QVector<QPair<int, int>>& edges)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<int>> adj = buildAdjList(nodeCount, edges);
    QVector<int> inDeg(nodeCount, 0);

    for (const auto& e : edges)
        if (e.second >= 0 && e.second < nodeCount)
            inDeg[e.second]++;

    /* Queue: collect all zero-indegree nodes */
    QVector<int> queue;
    for (int i = 0; i < nodeCount; ++i)
        if (inDeg[i] == 0) queue.append(i);

    QVector<int> order;
    order.reserve(nodeCount);
    int qi = 0;

    while (qi < queue.size()) {
        int u = queue[qi++];
        order.append(u);
        for (int v : adj[u]) {
            if (--inDeg[v] == 0) queue.append(v);
        }
    }

    /* Cycle detection */
    m_detectedCycle.clear();
    bool hasCycle = (order.size() < nodeCount);
    if (hasCycle) {
        /* Find cycle nodes (remaining nodes with in-degree > 0) */
        QVector<bool> inOrder(nodeCount, false);
        for (int u : order) inOrder[u] = true;
        for (int i = 0; i < nodeCount; ++i)
            if (!inOrder[i]) m_detectedCycle.append(i);
    }

    m_stats.totalSorts++;
    m_stats.lastNodeCount = nodeCount;
    m_stats.lastEdgeCount = edges.size();
    m_stats.lastHadCycle = hasCycle;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSorts > 0)
        ? m_timeSum / m_stats.totalSorts : 0.0;

    emit sortCompleted(nodeCount, hasCycle);
    return hasCycle ? QVector<int>() : order;
}

bool TopologicalSort::dfsVisit(int node, const QVector<QVector<int>>& adj,
                                QVector<int>& color, QVector<int>& order,
                                QVector<int>& cycle)
{
    color[node] = 1; /* GRAY: visiting */
    for (int v : adj[node]) {
        if (color[v] == 1) {
            /* Back edge: cycle detected */
            cycle.append(v);
            cycle.append(node);
            return true;
        }
        if (color[v] == 0) {
            if (dfsVisit(v, adj, color, order, cycle)) return true;
        }
    }
    color[node] = 2; /* BLACK: done */
    order.append(node);
    return false;
}

QVector<int> TopologicalSort::dfsSort(int nodeCount,
                                       const QVector<QPair<int, int>>& edges)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<int>> adj = buildAdjList(nodeCount, edges);
    QVector<int> color(nodeCount, 0); /* 0=WHITE, 1=GRAY, 2=BLACK */
    QVector<int> order;
    order.reserve(nodeCount);
    m_detectedCycle.clear();
    bool hasCycle = false;

    for (int i = 0; i < nodeCount; ++i) {
        if (color[i] == 0) {
            QVector<int> cycle;
            if (dfsVisit(i, adj, color, order, cycle)) {
                hasCycle = true;
                m_detectedCycle = cycle;
                break;
            }
        }
    }

    if (!hasCycle) std::reverse(order.begin(), order.end());

    m_stats.totalSorts++;
    m_stats.lastNodeCount = nodeCount;
    m_stats.lastEdgeCount = edges.size();
    m_stats.lastHadCycle = hasCycle;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSorts > 0)
        ? m_timeSum / m_stats.totalSorts : 0.0;

    emit sortCompleted(nodeCount, hasCycle);
    return hasCycle ? QVector<int>() : order;
}

bool TopologicalSort::hasCycle(int nodeCount,
                                const QVector<QPair<int, int>>& edges) const
{
    QVector<QVector<int>> adj = buildAdjList(nodeCount, edges);
    QVector<int> color(nodeCount, 0);
    /* DFS cycle detection without full sort */
    for (int i = 0; i < nodeCount; ++i) {
        if (color[i] != 0) continue;
        QVector<int> stack;
        stack.append(i);
        while (!stack.isEmpty()) {
            int u = stack.last();
            if (color[u] == 0) {
                color[u] = 1;
                for (int v : adj[u]) {
                    if (color[v] == 1) return true;
                    if (color[v] == 0) stack.append(v);
                }
            } else {
                color[u] = 2;
                stack.removeLast();
            }
        }
    }
    return false;
}

QVector<int> TopologicalSort::detectedCycle() const { return m_detectedCycle; }

QVector<QVector<int>> TopologicalSort::parallelLevels(
    int nodeCount, const QVector<QPair<int, int>>& edges)
{
    QVector<QVector<int>> adj = buildAdjList(nodeCount, edges);
    QVector<int> inDeg(nodeCount, 0);
    for (const auto& e : edges)
        if (e.second >= 0 && e.second < nodeCount)
            inDeg[e.second]++;

    QVector<QVector<int>> levels;
    QVector<int> current;
    for (int i = 0; i < nodeCount; ++i)
        if (inDeg[i] == 0) current.append(i);

    while (!current.isEmpty()) {
        levels.append(current);
        QVector<int> next;
        for (int u : current) {
            for (int v : adj[u]) {
                if (--inDeg[v] == 0) next.append(v);
            }
        }
        current = next;
    }
    return levels;
}

QVector<int> TopologicalSort::inDegrees(
    int nodeCount, const QVector<QPair<int, int>>& edges) const
{
    QVector<int> deg(nodeCount, 0);
    for (const auto& e : edges)
        if (e.second >= 0 && e.second < nodeCount)
            deg[e.second]++;
    return deg;
}

void TopologicalSort::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

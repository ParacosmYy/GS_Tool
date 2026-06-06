/**
 * @file StrongConnectivity.cpp
 * @brief StrongConnectivity 实现
 *
 * 实现强连通分量检测：Tarjan单次遍历算法、Kosaraju双遍DFS、分量DAG构建。
 */

#include "utils/graph182/StrongConnectivity.h"

#include <QElapsedTimer>
#include <algorithm>

StrongConnectivity::StrongConnectivity(QObject *parent)
    : QObject(parent)
{
}

StrongConnectivity::~StrongConnectivity() = default;

void StrongConnectivity::tarjanDFS(int u, const QVector<QVector<int>>& adj,
                                    QVector<int>& disc, QVector<int>& low,
                                    QVector<bool>& onStack, QVector<int>& stack,
                                    int& index, SCCResult& result)
{
    disc[u] = low[u] = index++;
    stack.append(u);
    onStack[u] = true;

    for (int v : adj[u]) {
        if (v < 0 || v >= adj.size()) continue;
        if (disc[v] == -1) {
            /* Unvisited: recurse */
            tarjanDFS(v, adj, disc, low, onStack, stack, index, result);
            low[u] = qMin(low[u], low[v]);
        } else if (onStack[v]) {
            /* Back edge to ancestor on stack */
            low[u] = qMin(low[u], disc[v]);
        }
    }

    /* Root of SCC found */
    if (low[u] == disc[u]) {
        QVector<int> comp;
        int w;
        do {
            w = stack.takeLast();
            onStack[w] = false;
            comp.append(w);
            result.componentId[w] = result.componentCount;
        } while (w != u);

        result.components.append(comp);
        result.componentCount++;
    }
}

StrongConnectivity::SCCResult StrongConnectivity::tarjan(
    const QVector<QVector<int>>& adj)
{
    QElapsedTimer timer;
    timer.start();

    int n = adj.size();
    SCCResult result;
    result.componentId.resize(n);
    result.componentId.fill(-1);
    result.componentCount = 0;

    QVector<int> disc(n, -1), low(n, -1);
    QVector<bool> onStack(n, false);
    QVector<int> stack;
    int index = 0;

    for (int i = 0; i < n; ++i) {
        if (disc[i] == -1)
            tarjanDFS(i, adj, disc, low, onStack, stack, index, result);
    }

    /* Build DAG edges */
    for (int u = 0; u < n; ++u) {
        for (int v : adj[u]) {
            if (v < 0 || v >= n) continue;
            int cu = result.componentId[u];
            int cv = result.componentId[v];
            if (cu != cv)
                result.dagEdges.append({cu, cv});
        }
    }

    m_stats.totalRuns++;
    m_stats.lastNodeCount = n;
    m_stats.lastEdgeCount = 0;
    for (const auto& list : adj) m_stats.lastEdgeCount += list.size();
    m_stats.lastComponentCount = result.componentCount;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalRuns > 0) ? m_timeSum / m_stats.totalRuns : 0.0;

    emit connectivityCompleted(result.componentCount);
    return result;
}

void StrongConnectivity::kosarajuDFS1(int u, const QVector<QVector<int>>& adj,
                                       QVector<bool>& visited, QVector<int>& order)
{
    visited[u] = true;
    for (int v : adj[u]) {
        if (v < 0 || v >= adj.size()) continue;
        if (!visited[v])
            kosarajuDFS1(v, adj, visited, order);
    }
    order.append(u);
}

void StrongConnectivity::kosarajuDFS2(int u, const QVector<QVector<int>>& radj,
                                       QVector<bool>& visited, QVector<int>& comp,
                                       int compId)
{
    visited[u] = true;
    comp[u] = compId;
    for (int v : radj[u]) {
        if (v < 0 || v >= radj.size()) continue;
        if (!visited[v])
            kosarajuDFS2(v, radj, visited, comp, compId);
    }
}

StrongConnectivity::SCCResult StrongConnectivity::kosaraju(
    const QVector<QVector<int>>& adj)
{
    QElapsedTimer timer;
    timer.start();

    int n = adj.size();
    SCCResult result;
    result.componentId.resize(n);
    result.componentId.fill(-1);
    result.componentCount = 0;

    /* Pass 1: forward DFS, record finish order */
    QVector<bool> visited(n, false);
    QVector<int> order;
    for (int i = 0; i < n; ++i)
        if (!visited[i])
            kosarajuDFS1(i, adj, visited, order);

    /* Build reverse graph */
    QVector<QVector<int>> radj(n);
    for (int u = 0; u < n; ++u)
        for (int v : adj[u])
            if (v >= 0 && v < n)
                radj[v].append(u);

    /* Pass 2: reverse DFS in decreasing finish order */
    visited.fill(false);
    for (int i = n - 1; i >= 0; --i) {
        int u = order[i];
        if (!visited[u]) {
            QVector<int> compNodes;
            kosarajuDFS2(u, radj, visited, result.componentId,
                         result.componentCount);
            result.componentCount++;
        }
    }

    /* Collect component members */
    result.components.resize(result.componentCount);
    for (int i = 0; i < n; ++i)
        result.components[result.componentId[i]].append(i);

    /* Build DAG edges */
    for (int u = 0; u < n; ++u) {
        for (int v : adj[u]) {
            if (v < 0 || v >= n) continue;
            int cu = result.componentId[u];
            int cv = result.componentId[v];
            if (cu != cv)
                result.dagEdges.append({cu, cv});
        }
    }

    m_stats.totalRuns++;
    m_stats.lastNodeCount = n;
    m_stats.lastComponentCount = result.componentCount;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalRuns > 0) ? m_timeSum / m_stats.totalRuns : 0.0;

    emit connectivityCompleted(result.componentCount);
    return result;
}

QVector<QVector<int>> StrongConnectivity::buildComponentDAG(
    const SCCResult& result, int nodeCount)
{
    int nc = result.componentCount;
    QVector<QVector<int>> dag(nc);
    for (const auto& edge : result.dagEdges) {
        int u = edge.first, v = edge.second;
        if (u >= 0 && u < nc && v >= 0 && v < nc)
            dag[u].append(v);
    }
    return dag;
}

QVector<int> StrongConnectivity::topologicalOrder(const QVector<QVector<int>>& dag)
{
    int n = dag.size();
    QVector<int> inDeg(n, 0);
    for (int u = 0; u < n; ++u)
        for (int v : dag[u])
            if (v >= 0 && v < n) inDeg[v]++;

    QVector<int> order;
    for (int i = 0; i < n; ++i)
        if (inDeg[i] == 0) order.append(i);

    for (int idx = 0; idx < order.size(); ++idx) {
        int u = order[idx];
        for (int v : dag[u]) {
            if (v >= 0 && v < n) {
                inDeg[v]--;
                if (inDeg[v] == 0) order.append(v);
            }
        }
    }
    return order;
}

void StrongConnectivity::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

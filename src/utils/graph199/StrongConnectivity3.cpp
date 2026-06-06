/**
 * @file StrongConnectivity3.cpp
 * @brief StrongConnectivity3 实现
 *
 * 实现Kosaraju算法：两遍DFS、反向图构建、缩点DAG生成。
 */

#include "utils/graph199/StrongConnectivity3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

StrongConnectivity3::StrongConnectivity3(QObject *parent) : QObject(parent) {}
StrongConnectivity3::~StrongConnectivity3() = default;

/* ---- First DFS pass: finish order ---- */

void StrongConnectivity3::dfsOrder(int v, const QVector<QVector<int>>& adj,
                                    QVector<bool>& visited,
                                    QVector<int>& order) const
{
    visited[v] = true;
    for (int u : adj[v]) {
        if (!visited[u])
            dfsOrder(u, adj, visited, order);
    }
    order.append(v);
}

/* ---- Second DFS pass: assign component IDs ---- */

void StrongConnectivity3::dfsAssign(int v, const QVector<QVector<int>>& revAdj,
                                     QVector<bool>& visited,
                                     QVector<int>& compId, int id) const
{
    visited[v] = true;
    compId[v] = id;
    for (int u : revAdj[v]) {
        if (!visited[u])
            dfsAssign(u, revAdj, visited, compId, id);
    }
}

/* ---- Build reverse graph ---- */

QVector<QVector<int>> StrongConnectivity3::buildReverseGraph(
    int n, const QVector<QVector<int>>& adj) const
{
    QVector<QVector<int>> revAdj(n);
    for (int v = 0; v < n; ++v)
        for (int u : adj[v])
            revAdj[u].append(v);
    return revAdj;
}

/* ---- Build condensation DAG ---- */

QVector<QPair<int, int>> StrongConnectivity3::buildCondensationDAG(
    const QVector<QPair<int, int>>& edges,
    const QVector<int>& componentId,
    int numComponents) const
{
    QVector<QPair<int, int>> dagEdges;
    for (auto& e : edges) {
        int cu = componentId[e.first];
        int cv = componentId[e.second];
        if (cu != cv)
            dagEdges.append({cu, cv});
    }

    // Remove duplicate edges
    std::sort(dagEdges.begin(), dagEdges.end(),
              [](const QPair<int,int>& a, const QPair<int,int>& b) {
                  return (a.first != b.first) ? (a.first < b.first) : (a.second < b.second);
              });
    dagEdges.erase(std::unique(dagEdges.begin(), dagEdges.end()), dagEdges.end());
    return dagEdges;
}

/* ---- Main compute ---- */

StrongConnectivity3::SCCResult StrongConnectivity3::compute(
    int numVertices, const QVector<QPair<int, int>>& edges)
{
    QElapsedTimer timer;
    timer.start();

    SCCResult result;
    int n = numVertices;
    if (n <= 0) return result;

    // Build adjacency list
    QVector<QVector<int>> adj(n);
    for (auto& e : edges) {
        if (e.first >= 0 && e.first < n && e.second >= 0 && e.second < n)
            adj[e.first].append(e.second);
    }

    // Build reverse graph
    auto revAdj = buildReverseGraph(n, adj);

    // Pass 1: DFS on original graph, record finish order
    QVector<bool> visited(n, false);
    QVector<int> order;
    for (int v = 0; v < n; ++v) {
        if (!visited[v])
            dfsOrder(v, adj, visited, order);
    }

    // Pass 2: DFS on reverse graph in reverse finish order
    QVector<int> compId(n, -1);
    std::fill(visited.begin(), visited.end(), false);
    int numComp = 0;

    for (int i = order.size() - 1; i >= 0; --i) {
        int v = order[i];
        if (!visited[v]) {
            dfsAssign(v, revAdj, visited, compId, numComp);
            numComp++;
        }
    }

    // Build component lists
    result.componentId = compId;
    result.numComponents = numComp;
    result.components.resize(numComp);
    for (int v = 0; v < n; ++v)
        result.components[compId[v]].append(v);

    // Build condensation DAG
    result.dagEdges = buildCondensationDAG(edges, compId, numComp);

    // Find largest component
    int largestSize = 0;
    for (auto& comp : result.components)
        largestSize = qMax(largestSize, comp.size());

    m_stats.totalRuns++;
    m_stats.numVertices = n;
    m_stats.numEdges = edges.size();
    m_stats.numComponents = numComp;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalRuns;

    emit computationCompleted(numComp, largestSize);
    return result;
}

/* ---- Reset ---- */

void StrongConnectivity3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

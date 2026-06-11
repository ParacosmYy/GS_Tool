/**
 * @file TransitiveClosure9.cpp
 * @brief TransitiveClosure9 实现
 *
 * 实现传递闭包：Floyd-Warshall位集优化与Warshall可达性实现紧凑图连通性计算。
 */

#include "utils/graph310/TransitiveClosure9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

TransitiveClosure9::TransitiveClosure9(QObject *parent)
    : QObject(parent) {}

TransitiveClosure9::~TransitiveClosure9() = default;

/* ---- Floyd-Warshall with boolean matrix optimization ---- */

void TransitiveClosure9::floydWarshallBitset(
    QVector<QVector<bool>>& dist, int n) const
{
    // Classic Warshall algorithm with bitset-friendly row operations:
    // If dist[i][k] is true, then dist[i][j] |= dist[k][j] for all j
    // This is the bitset-optimized version where each row can be ORed
    for (int k = 0; k < n; ++k) {
        for (int i = 0; i < n; ++i) {
            if (dist[i][k]) {
                // Row i can reach everything row k can reach
                for (int j = 0; j < n; ++j) {
                    dist[i][j] = dist[i][j] || dist[k][j];
                }
            }
        }
    }
}

/* ---- Count connected components ---- */

int TransitiveClosure9::countComponents(
    const QVector<QVector<bool>>& reachable, int n) const
{
    if (n == 0) return 0;
    QVector<bool> visited(n, false);
    int components = 0;

    for (int start = 0; start < n; ++start) {
        if (visited[start]) continue;
        components++;
        // BFS from start using reachability
        QVector<int> queue;
        queue.append(start);
        visited[start] = true;
        while (!queue.isEmpty()) {
            int u = queue.takeFirst();
            for (int v = 0; v < n; ++v) {
                if (!visited[v] && reachable[u][v]) {
                    visited[v] = true;
                    queue.append(v);
                }
            }
        }
    }
    return components;
}

/* ---- Compute from adjacency matrix ---- */

TransitiveClosure9::ClosureResult TransitiveClosure9::compute(
    const QVector<QVector<bool>>& adjMatrix)
{
    QElapsedTimer timer;
    timer.start();

    ClosureResult result;
    int n = adjMatrix.size();
    result.numNodes = n;

    // Initialize distance matrix with self-loops and adjacency
    QVector<QVector<bool>> dist(n, QVector<bool>(n, false));
    for (int i = 0; i < n; ++i) {
        dist[i][i] = true;  // Every node reaches itself
        for (int j = 0; j < n; ++j) {
            if (i < adjMatrix.size() && j < adjMatrix[i].size())
                dist[i][j] = dist[i][j] || adjMatrix[i][j];
        }
    }

    // Run Floyd-Warshall bitset-optimized Warshall algorithm
    floydWarshallBitset(dist, n);

    // Count reachable pairs
    int pairs = 0;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (dist[i][j]) pairs++;

    result.reachable = dist;
    result.numReachablePairs = pairs;
    result.numComponents = countComponents(dist, n);

    // Cache for queries
    m_reachable = dist;

    double elapsed = timer.elapsed();
    m_stats.graphSize = n;
    m_stats.totalComputations++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit closureDone(n, pairs, result.numComponents, elapsed);
    return result;
}

/* ---- Compute from edge list ---- */

TransitiveClosure9::ClosureResult TransitiveClosure9::computeFromEdges(
    const QVector<QPair<int, int>>& edges, int numNodes)
{
    // Build adjacency matrix from edge list
    QVector<QVector<bool>> adj(numNodes, QVector<bool>(numNodes, false));
    for (const auto& edge : edges) {
        if (edge.first >= 0 && edge.first < numNodes &&
            edge.second >= 0 && edge.second < numNodes) {
            adj[edge.first][edge.second] = true;
        }
    }
    return compute(adj);
}

/* ---- Query reachability ---- */

bool TransitiveClosure9::isReachable(int u, int v) const
{
    if (u < 0 || u >= m_reachable.size()) return false;
    if (v < 0 || v >= m_reachable[u].size()) return false;
    return m_reachable[u][v];
}

/* ---- Get all nodes reachable from u ---- */

QVector<int> TransitiveClosure9::reachableFrom(int u) const
{
    QVector<int> result;
    if (u < 0 || u >= m_reachable.size()) return result;
    for (int v = 0; v < m_reachable[u].size(); ++v) {
        if (m_reachable[u][v])
            result.append(v);
    }
    return result;
}

/* ---- Reset ---- */

void TransitiveClosure9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_reachable.clear();
}

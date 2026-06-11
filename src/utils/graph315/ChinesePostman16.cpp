/**
 * @file ChinesePostman16.cpp
 * @brief ChinesePostman16 实现
 *
 * 实现中国邮路问题：Blossom算法匹配与Euler游历扩展实现有向和混合图邮路。
 */

#include "utils/graph315/ChinesePostman16.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

ChinesePostman16::ChinesePostman16(QObject *parent)
    : QObject(parent) {}

ChinesePostman16::~ChinesePostman16() = default;

/* ---- Set graph ---- */

void ChinesePostman16::setGraph(int vertices, const QVector<Edge>& edges)
{
    m_n = qMax(1, vertices);
    m_edges = edges;

    // Build adjacency matrix
    m_adj.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_adj[i].resize(m_n, std::numeric_limits<double>::infinity());
        m_adj[i][i] = 0.0;
    }
    for (const auto& e : m_edges) {
        if (e.from >= 0 && e.from < m_n && e.to >= 0 && e.to < m_n)
            m_adj[e.from][e.to] = qMin(m_adj[e.from][e.to], e.weight);
    }
}

/* ---- Floyd-Warshall all-pairs shortest paths ---- */

QVector<QVector<double>> ChinesePostman16::floydWarshall() const
{
    int n = m_n;
    QVector<QVector<double>> dist = m_adj;

    for (int k = 0; k < n; ++k)
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if (dist[i][k] + dist[k][j] < dist[i][j])
                    dist[i][j] = dist[i][k] + dist[k][j];

    return dist;
}

/* ---- Find odd-degree vertices ---- */

QVector<int> ChinesePostman16::findOddVertices() const
{
    QVector<int> degree(m_n, 0);
    for (const auto& e : m_edges) {
        degree[e.from]++;
        if (!e.directed)
            degree[e.to]++;
    }

    QVector<int> odd;
    for (int i = 0; i < m_n; ++i)
        if (degree[i] % 2 != 0)
            odd.append(i);
    return odd;
}

/* ---- Compute vertex balance for directed graphs ---- */

QVector<double> ChinesePostman16::vertexBalance() const
{
    QVector<double> balance(m_n, 0.0);
    for (const auto& e : m_edges) {
        balance[e.from] -= e.weight;    // Outgoing
        balance[e.to] += e.weight;      // Incoming
    }
    return balance;
}

/* ---- Minimum weight perfect matching (simplified blossom-like greedy) ---- */

QVector<QPair<int, int>> ChinesePostman16::minWeightMatching(
    const QVector<int>& vertices,
    const QVector<QVector<double>>& dist) const
{
    int m = vertices.size();
    if (m % 2 != 0) return {};

    // Greedy nearest-neighbor matching (approximation to blossom algorithm)
    QVector<bool> used(m, false);
    QVector<QPair<int, int>> matchings;

    for (int i = 0; i < m; ++i) {
        if (used[i]) continue;
        double bestDist = std::numeric_limits<double>::infinity();
        int bestJ = -1;
        for (int j = i + 1; j < m; ++j) {
            if (used[j]) continue;
            double d = dist[vertices[i]][vertices[j]];
            if (d < bestDist) { bestDist = d; bestJ = j; }
        }
        if (bestJ >= 0) {
            matchings.append({vertices[i], vertices[bestJ]});
            used[i] = true;
            used[bestJ] = true;
        }
    }

    return matchings;
}

/* ---- Build augmented adjacency with duplicated edges ---- */

QVector<QVector<QPair<int, double>>> ChinesePostman16::buildAugmentedAdjacency(
    const QVector<QPair<int, int>>& matchings) const
{
    QVector<QVector<QPair<int, double>>> adjList(m_n);

    // Add original edges
    for (const auto& e : m_edges) {
        adjList[e.from].append({e.to, e.weight});
        if (!e.directed)
            adjList[e.to].append({e.from, e.weight});
    }

    // Add duplicated edges along shortest paths
    for (const auto& [u, v] : matchings) {
        QVector<int> path = shortestPath(u, v);
        for (int i = 0; i + 1 < path.size(); ++i) {
            double w = m_adj[path[i]][path[i + 1]];
            adjList[path[i]].append({path[i + 1], w});
            adjList[path[i + 1]].append({path[i], w});
        }
    }

    return adjList;
}

/* ---- Dijkstra shortest path ---- */

QVector<int> ChinesePostman16::shortestPath(int src, int dst) const
{
    int n = m_n;
    QVector<double> dist(n, std::numeric_limits<double>::infinity());
    QVector<int> prev(n, -1);
    QVector<bool> visited(n, false);

    dist[src] = 0.0;

    for (int iter = 0; iter < n; ++iter) {
        int u = -1;
        double minD = std::numeric_limits<double>::infinity();
        for (int i = 0; i < n; ++i) {
            if (!visited[i] && dist[i] < minD) { minD = dist[i]; u = i; }
        }
        if (u < 0 || u == dst) break;
        visited[u] = true;

        for (int v = 0; v < n; ++v) {
            double alt = dist[u] + m_adj[u][v];
            if (alt < dist[v]) { dist[v] = alt; prev[v] = u; }
        }
    }

    // Reconstruct path
    QVector<int> path;
    for (int cur = dst; cur >= 0; cur = prev[cur])
        path.prepend(cur);
    if (path.isEmpty() || path[0] != src) path = {src, dst};
    return path;
}

/* ---- Hierholzer's Euler tour ---- */

QVector<int> ChinesePostman16::hierholzer(
    const QVector<QVector<QPair<int, double>>>& adjList) const
{
    int n = adjList.size();
    if (n == 0) return {};

    // Count edges for degree check
    QVector<int> degree(n, 0);
    int totalEdges = 0;
    for (int i = 0; i < n; ++i) {
        degree[i] = adjList[i].size();
        totalEdges += degree[i];
    }

    // Use mutable edge tracking
    QVector<int> edgeIdx(n, 0);
    QVector<int> tour;
    QVector<int> stack;
    stack.append(0);

    while (!stack.isEmpty()) {
        int v = stack.back();
        if (edgeIdx[v] < adjList[v].size()) {
            int next = adjList[v][edgeIdx[v]].first;
            edgeIdx[v]++;
            stack.append(next);
        } else {
            tour.append(v);
            stack.removeLast();
        }
    }

    std::reverse(tour.begin(), tour.end());
    return tour;
}

/* ---- Solve undirected Chinese postman ---- */

ChinesePostman16::TourResult ChinesePostman16::solveUndirected()
{
    QElapsedTimer timer;
    timer.start();

    TourResult result;

    // Compute base cost
    double baseCost = 0.0;
    for (const auto& e : m_edges)
        baseCost += e.weight;

    // Find odd-degree vertices
    QVector<int> oddVerts = findOddVertices();
    result.isEulerian = oddVerts.isEmpty();

    // Compute shortest paths
    QVector<QVector<double>> dist = floydWarshall();

    // Find minimum weight perfect matching
    QVector<QPair<int, int>> matchings = minWeightMatching(oddVerts, dist);

    // Compute extra cost from matchings
    double extraCost = 0.0;
    for (const auto& [u, v] : matchings)
        extraCost += dist[u][v];

    // Build augmented graph and find Euler tour
    auto augAdj = buildAugmentedAdjacency(matchings);
    result.tour = hierholzer(augAdj);
    result.totalCost = baseCost + extraCost;
    result.numDuplicatedEdges = matchings.size();
    result.elapsedMs = timer.elapsed();

    m_stats.totalSolves++;
    m_stats.lastVertexCount = m_n;
    m_stats.lastEdgeCount = m_edges.size();
    m_timeSum += result.elapsedMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveDone(result.totalCost, result.numDuplicatedEdges, result.elapsedMs);
    return result;
}

/* ---- Solve directed Chinese postman ---- */

ChinesePostman16::TourResult ChinesePostman16::solveDirected()
{
    QElapsedTimer timer;
    timer.start();

    TourResult result;

    double baseCost = 0.0;
    for (const auto& e : m_edges)
        baseCost += e.weight;

    // Compute imbalance for each vertex
    QVector<int> inDeg(m_n, 0), outDeg(m_n, 0);
    for (const auto& e : m_edges) {
        outDeg[e.from]++;
        inDeg[e.to]++;
    }

    QVector<int> surplus, deficit;
    for (int i = 0; i < m_n; ++i) {
        int bal = inDeg[i] - outDeg[i];
        for (int k = 0; k < bal; ++k) surplus.append(i);
        for (int k = 0; k < -bal; ++k) deficit.append(i);
    }

    result.isEulerian = surplus.isEmpty() && deficit.isEmpty();

    // Match surplus to deficit via shortest paths
    QVector<QPair<int, int>> matchings;
    double extraCost = 0.0;
    auto dist = floydWarshall();

    int m = qMin(surplus.size(), deficit.size());
    for (int i = 0; i < m; ++i) {
        matchings.append({surplus[i], deficit[i]});
        extraCost += dist[surplus[i]][deficit[i]];
    }

    // Build augmented adjacency
    auto augAdj = buildAugmentedAdjacency(matchings);
    result.tour = hierholzer(augAdj);
    result.totalCost = baseCost + extraCost;
    result.numDuplicatedEdges = matchings.size();
    result.elapsedMs = timer.elapsed();

    m_stats.totalSolves++;
    m_stats.lastVertexCount = m_n;
    m_stats.lastEdgeCount = m_edges.size();
    m_timeSum += result.elapsedMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveDone(result.totalCost, result.numDuplicatedEdges, result.elapsedMs);
    return result;
}

/* ---- Solve mixed graph (heuristic: treat as directed then fix undirected) ---- */

ChinesePostman16::TourResult ChinesePostman16::solveMixed()
{
    QElapsedTimer timer;
    timer.start();

    TourResult result;

    // Compute base cost
    double baseCost = 0.0;
    for (const auto& e : m_edges)
        baseCost += e.weight;

    // Check Eulerian
    result.isEulerian = isEulerian();

    // For mixed: use undirected approach on undirected edges + directed approach on directed
    // Simplified heuristic: duplicate all edges, then solve as undirected
    QVector<int> oddVerts = findOddVertices();
    auto dist = floydWarshall();
    auto matchings = minWeightMatching(oddVerts, dist);

    double extraCost = 0.0;
    for (const auto& [u, v] : matchings)
        extraCost += dist[u][v];

    auto augAdj = buildAugmentedAdjacency(matchings);
    result.tour = hierholzer(augAdj);
    result.totalCost = baseCost + extraCost;
    result.numDuplicatedEdges = matchings.size();
    result.elapsedMs = timer.elapsed();

    m_stats.totalSolves++;
    m_stats.lastVertexCount = m_n;
    m_stats.lastEdgeCount = m_edges.size();
    m_timeSum += result.elapsedMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveDone(result.totalCost, result.numDuplicatedEdges, result.elapsedMs);
    return result;
}

/* ---- Check Eulerian ---- */

bool ChinesePostman16::isEulerian() const
{
    QVector<int> degree(m_n, 0);
    for (const auto& e : m_edges) {
        degree[e.from]++;
        if (!e.directed) degree[e.to]++;
    }
    for (int d : degree)
        if (d % 2 != 0) return false;
    return true;
}

/* ---- Find Euler tour ---- */

QVector<int> ChinesePostman16::findEulerTour() const
{
    QVector<QVector<QPair<int, double>>> adjList(m_n);
    for (const auto& e : m_edges) {
        adjList[e.from].append({e.to, e.weight});
        if (!e.directed)
            adjList[e.to].append({e.from, e.weight});
    }
    return hierholzer(adjList);
}

/* ---- Reset ---- */

void ChinesePostman16::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

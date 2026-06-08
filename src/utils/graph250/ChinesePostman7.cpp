/**
 * @file ChinesePostman7.cpp
 * @brief ChinesePostman7 实现
 *
 * 实现中国邮路问题：Hierholzer回路构造与最小权奇度顶点匹配。
 */

#include "utils/graph250/ChinesePostman7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

ChinesePostman7::ChinesePostman7(QObject *parent) : QObject(parent) {}
ChinesePostman7::~ChinesePostman7() = default;

/* ---- Configuration ---- */

void ChinesePostman7::setDirected(bool directed) { m_directed = directed; }

void ChinesePostman7::addEdge(int from, int to, double weight)
{
    Edge e;
    e.from = from;
    e.to = to;
    e.weight = weight;
    e.index = m_edges.size();
    e.directed = m_directed;
    m_edges.append(e);

    // Expand vertex count if needed
    m_numVertices = qMax(m_numVertices, qMax(from, to) + 1);
}

void ChinesePostman7::buildGraph(int numVertices, const QVector<Edge>& edges)
{
    m_numVertices = numVertices;
    m_edges = edges;
    for (int i = 0; i < m_edges.size(); ++i) {
        m_edges[i].index = i;
        m_edges[i].directed = m_directed;
    }
}

/* ---- Build adjacency list ---- */

// Rebuild adjacency from m_edges (called internally before algorithms)

/* ---- Dijkstra from source ---- */

QVector<double> ChinesePostman7::dijkstra(int source) const
{
    int n = m_numVertices;
    QVector<double> dist(n, std::numeric_limits<double>::max());
    dist[source] = 0.0;

    // Build adjacency for dijkstra
    QVector<QVector<QPair<int, double>>> adj(n);
    for (const auto& e : m_edges) {
        adj[e.from].append({e.to, e.weight});
        if (!m_directed)
            adj[e.to].append({e.from, e.weight});
    }

    QVector<bool> visited(n, false);
    for (int iter = 0; iter < n; ++iter) {
        // Find unvisited with min dist
        int u = -1;
        double minD = std::numeric_limits<double>::max();
        for (int i = 0; i < n; ++i) {
            if (!visited[i] && dist[i] < minD) { minD = dist[i]; u = i; }
        }
        if (u < 0) break;
        visited[u] = true;

        for (const auto& [v, w] : adj[u]) {
            if (dist[u] + w < dist[v])
                dist[v] = dist[u] + w;
        }
    }
    return dist;
}

/* ---- Shortest paths between sources ---- */

QVector<QVector<double>> ChinesePostman7::shortestPaths(const QVector<int>& sources) const
{
    int k = sources.size();
    QVector<QVector<double>> allD(k);
    for (int i = 0; i < k; ++i)
        allD[i] = dijkstra(sources[i]);

    // Extract k x k submatrix
    QVector<QVector<double>> result(k, QVector<double>(k, 0.0));
    for (int i = 0; i < k; ++i)
        for (int j = 0; j < k; ++j)
            result[i][j] = allD[i][sources[j]];
    return result;
}

/* ---- Minimum-weight perfect matching (greedy for small sets) ---- */

QVector<QPair<int, int>> ChinesePostman7::minWeightMatching(
    const QVector<int>& oddVerts,
    const QVector<QVector<double>>& dist) const
{
    int k = oddVerts.size();
    QVector<QPair<int, int>> pairs;
    QVector<bool> used(k, false);

    // Greedy: repeatedly pick the shortest available edge
    while (pairs.size() < k / 2) {
        double bestW = std::numeric_limits<double>::max();
        int bestI = -1, bestJ = -1;
        for (int i = 0; i < k; ++i) {
            if (used[i]) continue;
            for (int j = i + 1; j < k; ++j) {
                if (used[j]) continue;
                if (dist[i][j] < bestW) {
                    bestW = dist[i][j];
                    bestI = i;
                    bestJ = j;
                }
            }
        }
        if (bestI < 0) break;
        used[bestI] = true;
        used[bestJ] = true;
        pairs.append({bestI, bestJ});
    }
    return pairs;
}

/* ---- Reconstruct shortest path ---- */

QVector<int> ChinesePostman7::reconstructPath(int from, int to) const
{
    // BFS-based shortest path reconstruction using edge weights
    int n = m_numVertices;
    QVector<double> dist(n, std::numeric_limits<double>::max());
    QVector<int> prev(n, -1);
    dist[from] = 0.0;

    // Build adjacency with edge indices
    QVector<QVector<QPair<int, int>>> adj(n);  // (neighbor, edgeIdx)
    for (int i = 0; i < m_edges.size(); ++i) {
        adj[m_edges[i].from].append({m_edges[i].to, i});
        if (!m_directed)
            adj[m_edges[i].to].append({m_edges[i].from, i});
    }

    QVector<bool> visited(n, false);
    for (int iter = 0; iter < n; ++iter) {
        int u = -1;
        double minD = std::numeric_limits<double>::max();
        for (int i = 0; i < n; ++i) {
            if (!visited[i] && dist[i] < minD) { minD = dist[i]; u = i; }
        }
        if (u < 0 || u == to) break;
        visited[u] = true;
        for (const auto& [v, eidx] : adj[u]) {
            double w = m_edges[eidx].weight;
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                prev[v] = u;
            }
        }
    }

    QVector<int> path;
    for (int cur = to; cur >= 0; cur = prev[cur])
        path.prepend(cur);
    return path;
}

/* ---- Duplicate edges along shortest paths ---- */

void ChinesePostman7::duplicateEdges(const QVector<QPair<int, int>>& pairs,
                                       const QVector<int>& oddVerts,
                                       const QVector<QVector<double>>& dist)
{
    for (const auto& [i, j] : pairs) {
        int from = oddVerts[i];
        int to = oddVerts[j];
        QVector<int> path = reconstructPath(from, to);

        // Duplicate edges along path
        for (int p = 0; p < path.size() - 1; ++p) {
            int u = path[p], v = path[p + 1];
            // Find the edge
            for (const auto& e : m_edges) {
                if ((e.from == u && e.to == v) || (!m_directed && e.from == v && e.to == u)) {
                    Edge dup = e;
                    dup.index = m_edges.size();
                    m_edges.append(dup);
                    break;
                }
            }
        }
    }
}

/* ---- Hierholzer for Eulerian circuit ---- */

QVector<int> ChinesePostman7::hierholzer()
{
    // Rebuild adjacency
    int n = m_numVertices;
    QVector<QVector<int>> adj(n);
    for (int i = 0; i < m_edges.size(); ++i)
        adj[m_edges[i].from].append(i);

    QVector<bool> usedEdge(m_edges.size(), false);
    QVector<int> edgePath;

    // Start from first edge's source
    int start = m_edges.isEmpty() ? 0 : m_edges[0].from;
    QVector<int> stack;
    stack.append(start);

    while (!stack.isEmpty()) {
        int v = stack.back();
        bool found = false;
        while (!adj[v].isEmpty()) {
            int eidx = adj[v].back();
            adj[v].pop_back();
            if (usedEdge[eidx]) continue;
            usedEdge[eidx] = true;
            int next = m_edges[eidx].to;
            stack.append(next);
            found = true;
            break;
        }
        if (!found) {
            if (!stack.isEmpty()) {
                int cur = stack.back();
                stack.pop_back();
                if (!stack.isEmpty())
                    edgePath.append(cur);
            }
        }
    }

    return edgePath;
}

/* ---- Odd-degree vertices ---- */

QVector<int> ChinesePostman7::oddDegreeVertices() const
{
    QVector<int> degree(m_numVertices, 0);
    for (const auto& e : m_edges) {
        degree[e.from]++;
        if (!m_directed) degree[e.to]++;
    }
    QVector<int> odd;
    for (int i = 0; i < m_numVertices; ++i)
        if (degree[i] % 2 != 0) odd.append(i);
    return odd;
}

/* ---- Is Eulerian ---- */

bool ChinesePostman7::isEulerian() const { return oddDegreeVertices().isEmpty(); }

/* ---- Solve ---- */

ChinesePostman7::Tour ChinesePostman7::solve()
{
    QElapsedTimer timer;
    timer.start();

    Tour result;

    // Find odd-degree vertices
    QVector<int> oddVerts = oddDegreeVertices();
    m_stats.numOddVertices = oddVerts.size();

    if (!oddVerts.isEmpty()) {
        // Compute shortest paths
        QVector<QVector<double>> dist = shortestPaths(oddVerts);

        // Min-weight matching
        auto pairs = minWeightMatching(oddVerts, dist);

        double matchCost = 0.0;
        for (const auto& [i, j] : pairs)
            matchCost += dist[i][j];

        // Duplicate edges
        duplicateEdges(pairs, oddVerts, dist);

        emit matchingCompleted(pairs.size(), matchCost);
    }

    // Hierholzer circuit
    QVector<int> vertexPath = hierholzer();

    // Compute total cost
    double cost = 0.0;
    for (const auto& e : m_edges)
        cost += e.weight;
    // Subtract duplicated costs for original
    int origEdges = 0;
    for (const auto& e : m_edges) {
        if (e.index < m_stats.numEdges) origEdges++;
    }

    result.vertexSequence = vertexPath;
    result.totalCost = cost;
    result.numDuplicatedEdges = m_edges.size() - m_stats.numEdges;

    m_stats.numVertices = m_numVertices;
    m_stats.numEdges = m_edges.size();
    m_stats.tourCost = cost;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit tourCompleted(cost, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void ChinesePostman7::resetStatistics()
{
    m_edges.clear();
    m_adj.clear();
    m_numVertices = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

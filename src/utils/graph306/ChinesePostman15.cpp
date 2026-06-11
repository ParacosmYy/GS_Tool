/**
 * @file ChinesePostman15.cpp
 * @brief ChinesePostman15 实现
 *
 * 实现中国邮递员问题：Christofides匹配启发式与捷径消除实现稀疏图近优邮路。
 */

#include "utils/graph306/ChinesePostman15.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ChinesePostman15::ChinesePostman15(QObject *parent)
    : QObject(parent) {}

ChinesePostman15::~ChinesePostman15() = default;

/* ---- Configuration ---- */

void ChinesePostman15::setDirected(bool directed) { m_directed = directed; }

/* ---- Find odd-degree vertices ---- */

QVector<int> ChinesePostman15::findOddDegreeVertices(
    int n, const QVector<Edge>& edges) const
{
    QVector<int> degree(n, 0);
    for (const auto& e : edges) {
        degree[e.from]++;
        if (!m_directed) degree[e.to]++;
    }
    QVector<int> odd;
    for (int i = 0; i < n; ++i)
        if (degree[i] % 2 != 0)
            odd.append(i);
    return odd;
}

/* ---- Floyd-Warshall all-pairs shortest path ---- */

void ChinesePostman15::floydWarshall(
    int n, const QVector<QVector<double>>& adj,
    QVector<QVector<double>>& dist,
    QVector<QVector<int>>& next) const
{
    const double INF = 1e18;
    dist.assign(n, QVector<double>(n, INF));
    next.assign(n, QVector<int>(n, -1));

    for (int i = 0; i < n; ++i) dist[i][i] = 0.0;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (adj[i][j] < INF) {
                dist[i][j] = adj[i][j];
                next[i][j] = j;
            }

    for (int k = 0; k < n; ++k)
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if (dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    next[i][j] = next[i][k];
                }
}

/* ---- Minimum weight perfect matching (greedy heuristic) ---- */

QVector<QPair<int, int>> ChinesePostman15::minWeightMatching(
    const QVector<int>& oddVertices,
    const QVector<QVector<double>>& dist) const
{
    int m = oddVertices.size();
    QVector<QPair<int, int>> matches;
    if (m % 2 != 0) return matches;

    QVector<bool> used(m, false);

    // Greedy: repeatedly pair closest unmatched vertices
    for (int i = 0; i < m; ++i) {
        if (used[i]) continue;
        double bestD = 1e18;
        int bestJ = -1;
        for (int j = i + 1; j < m; ++j) {
            if (used[j]) continue;
            double d = dist[oddVertices[i]][oddVertices[j]];
            if (d < bestD) { bestD = d; bestJ = j; }
        }
        if (bestJ >= 0) {
            used[i] = true;
            used[bestJ] = true;
            matches.append({oddVertices[i], oddVertices[bestJ]});
        }
    }
    return matches;
}

/* ---- Euler tour (Hierholzer's algorithm) ---- */

QVector<int> ChinesePostman15::eulerTour(int n, QVector<QVector<int>>& adjList) const
{
    QVector<int> tour;
    if (n == 0) return tour;

    // Find start vertex with non-zero degree
    int start = 0;
    for (int i = 0; i < n; ++i)
        if (!adjList[i].isEmpty()) { start = i; break; }

    QVector<int> stack;
    stack.append(start);

    while (!stack.isEmpty()) {
        int v = stack.back();
        if (adjList[v].isEmpty()) {
            tour.append(v);
            stack.removeLast();
        } else {
            int u = adjList[v].back();
            adjList[v].removeLast();
            // Remove reverse edge too
            for (int i = 0; i < adjList[u].size(); ++i) {
                if (adjList[u][i] == v) {
                    adjList[u].removeAt(i);
                    break;
                }
            }
            stack.append(u);
        }
    }

    std::reverse(tour.begin(), tour.end());
    return tour;
}

/* ---- Shortcut elimination ---- */

QVector<int> ChinesePostman15::shortcutElimination(const QVector<int>& tour, int n) const
{
    QVector<bool> visited(n, false);
    QVector<int> result;
    for (int v : tour) {
        if (!visited[v]) {
            visited[v] = true;
            result.append(v);
        }
    }
    if (!result.isEmpty())
        result.append(result[0]); // Close the cycle
    return result;
}

/* ---- Main solve ---- */

ChinesePostman15::TourResult ChinesePostman15::solve(
    int numVertices, const QVector<Edge>& edges)
{
    QElapsedTimer timer;
    timer.start();

    TourResult result;
    int n = numVertices;
    if (n == 0 || edges.isEmpty()) return result;

    // Build adjacency matrix
    const double INF = 1e18;
    QVector<QVector<double>> adj(n, QVector<double>(n, INF));
    for (const auto& e : edges) {
        adj[e.from][e.to] = qMin(adj[e.from][e.to], e.weight);
        if (!m_directed)
            adj[e.to][e.from] = qMin(adj[e.to][e.from], e.weight);
    }

    // Compute all-pairs shortest paths
    QVector<QVector<double>> dist;
    QVector<QVector<int>> next;
    floydWarshall(n, adj, dist, next);

    // Find odd-degree vertices
    auto oddVertices = findOddDegreeVertices(n, edges);

    // Minimum weight perfect matching on odd vertices
    auto matches = minWeightMatching(oddVertices, dist);

    // Build multigraph by duplicating matched edges
    QVector<QVector<int>> adjList(n);
    double totalCost = 0.0;
    int dupEdges = 0;

    // Add original edges
    for (const auto& e : edges) {
        adjList[e.from].append(e.to);
        if (!m_directed) adjList[e.to].append(e.from);
        totalCost += e.weight;
    }

    // Add shortest paths for matched pairs
    for (const auto& [u, v] : matches) {
        // Trace shortest path and add edges
        int cur = u;
        while (cur != v && next[cur][v] != -1) {
            int nxt = next[cur][v];
            adjList[cur].append(nxt);
            adjList[nxt].append(cur);
            totalCost += dist[cur][nxt];
            dupEdges++;
            cur = nxt;
        }
    }

    // Find Euler tour
    auto adjCopy = adjList;
    result.tour = eulerTour(n, adjCopy);

    // Shortcut elimination for near-optimal tour
    result.tour = shortcutElimination(result.tour, n);

    result.totalCost = totalCost;
    result.numDuplicateEdges = dupEdges;
    result.isOptimal = (oddVertices.size() <= 2);

    double elapsed = timer.elapsed();
    m_stats.numVertices = n;
    m_stats.numEdges = edges.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveDone(result.tour.size(), result.totalCost, elapsed);
    return result;
}

/* ---- Reset ---- */

void ChinesePostman15::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

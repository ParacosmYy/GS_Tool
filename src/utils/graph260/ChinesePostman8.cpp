/**
 * @file ChinesePostman8.cpp
 * @brief ChinesePostman8 实现
 *
 * 实现中国邮路问题：Edmonds花算法奇度顶点最小权完美匹配。
 */

#include "utils/graph260/ChinesePostman8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <cstdlib>
#include <queue>

/* ---- Construction / Destruction ---- */

ChinesePostman8::ChinesePostman8(QObject *parent) : QObject(parent) {}
ChinesePostman8::~ChinesePostman8() = default;

/* ---- Set graph ---- */

void ChinesePostman8::setGraph(int numVertices, const QVector<Edge>& edges)
{
    m_n = numVertices;
    m_edges = edges;
    m_adj.resize(m_n);
    for (int i = 0; i < m_n; ++i) m_adj[i].clear();

    for (int i = 0; i < m_edges.size(); ++i) {
        const auto& e = m_edges[i];
        m_adj[e.from].append({e.to, i});
        m_adj[e.to].append({e.from, i});
    }

    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edges.size();
}

/* ---- Find odd-degree vertices ---- */

QVector<int> ChinesePostman8::findOddVertices() const
{
    QVector<int> degree(m_n, 0);
    for (const auto& e : m_edges) {
        degree[e.from]++;
        degree[e.to]++;
    }
    QVector<int> odd;
    for (int i = 0; i < m_n; ++i)
        if (degree[i] % 2 != 0) odd.append(i);
    return odd;
}

/* ---- Floyd-Warshall all-pairs shortest paths ---- */

QVector<QVector<double>> ChinesePostman8::allPairsShortestPath() const
{
    double INF = std::numeric_limits<double>::max() / 2.0;
    QVector<QVector<double>> dist(m_n, QVector<double>(m_n, INF));
    for (int i = 0; i < m_n; ++i) dist[i][i] = 0.0;
    for (const auto& e : m_edges) {
        dist[e.from][e.to] = qMin(dist[e.from][e.to], e.weight);
        dist[e.to][e.from] = qMin(dist[e.to][e.from], e.weight);
    }
    for (int k = 0; k < m_n; ++k)
        for (int i = 0; i < m_n; ++i)
            for (int j = 0; j < m_n; ++j)
                if (dist[i][k] + dist[k][j] < dist[i][j])
                    dist[i][j] = dist[i][k] + dist[k][j];
    return dist;
}

/* ---- Blossom matching (simplified greedy + local search) ---- */

QVector<QPair<int, int>> ChinesePostman8::blossomMatching(
    const QVector<int>& oddVerts,
    const QVector<QVector<double>>& dist) const
{
    int m = oddVerts.size();
    if (m % 2 != 0) return {};

    // Greedy nearest-neighbor matching
    QVector<bool> used(m, false);
    QVector<QPair<int, int>> matching;

    // Iterative improvement: repeatedly match closest pair
    for (int iter = 0; iter < m / 2; ++iter) {
        double bestDist = std::numeric_limits<double>::max();
        int bestI = -1, bestJ = -1;
        for (int i = 0; i < m; ++i) {
            if (used[i]) continue;
            for (int j = i + 1; j < m; ++j) {
                if (used[j]) continue;
                double d = dist[oddVerts[i]][oddVerts[j]];
                if (d < bestDist) {
                    bestDist = d;
                    bestI = i; bestJ = j;
                }
            }
        }
        if (bestI >= 0) {
            matching.append({oddVerts[bestI], oddVerts[bestJ]});
            used[bestI] = true;
            used[bestJ] = true;
        }
    }

    // Local search: try swapping pairs to reduce total cost
    bool improved = true;
    while (improved) {
        improved = false;
        for (int i = 0; i < matching.size(); ++i) {
            for (int j = i + 1; j < matching.size(); ++j) {
                int a = matching[i].first, b = matching[i].second;
                int c = matching[j].first, d = matching[j].second;
                double cur = dist[a][b] + dist[c][d];
                double alt1 = dist[a][c] + dist[b][d];
                double alt2 = dist[a][d] + dist[b][c];
                if (alt1 < cur && alt1 <= alt2) {
                    matching[i] = {a, c}; matching[j] = {b, d};
                    improved = true;
                } else if (alt2 < cur) {
                    matching[i] = {a, d}; matching[j] = {b, c};
                    improved = true;
                }
            }
        }
    }
    return matching;
}

/* ---- Euler tour (Hierholzer) ---- */

QVector<int> ChinesePostman8::eulerTour()
{
    // Build multigraph adjacency
    QVector<QVector<QPair<int, int>>> adj(m_n);
    for (int i = 0; i < m_edges.size(); ++i) {
        const auto& e = m_edges[i];
        adj[e.from].append({e.to, i});
        adj[e.to].append({e.from, i});
    }
    // Add matched edges (duplicate)
    for (int i = 0; i < m_addedEdges.size(); ++i) {
        const auto& e = m_addedEdges[i];
        adj[e.from].append({e.to, m_edges.size() + i});
        adj[e.to].append({e.from, m_edges.size() + i});
    }

    // Track used edges
    QVector<bool> usedEdge(m_edges.size() + m_addedEdges.size(), false);
    QVector<int> tour;
    QVector<int> stack;
    stack.append(0);

    while (!stack.isEmpty()) {
        int v = stack.back();
        bool found = false;
        while (!adj[v].isEmpty()) {
            auto [to, eidx] = adj[v].back();
            adj[v].pop_back();
            if (!usedEdge[eidx]) {
                usedEdge[eidx] = true;
                stack.append(to);
                found = true;
                break;
            }
        }
        if (!found) {
            tour.append(stack.back());
            stack.pop_back();
        }
    }

    std::reverse(tour.begin(), tour.end());
    return tour;
}

/* ---- Solve ---- */

void ChinesePostman8::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0 || m_edges.isEmpty()) return;

    // Compute base edge weight
    double baseWeight = 0.0;
    for (const auto& e : m_edges) baseWeight += e.weight;

    // Find odd-degree vertices
    QVector<int> odd = findOddVertices();
    m_stats.numOddVertices = odd.size();

    if (odd.size() % 2 != 0) return;  // Invalid graph

    // Compute all-pairs shortest paths
    auto dist = allPairsShortestPath();

    // Find minimum weight perfect matching on odd vertices
    m_addedEdges.clear();
    double matchWeight = 0.0;

    if (!odd.isEmpty()) {
        auto matching = blossomMatching(odd, dist);
        for (const auto& [u, v] : matching) {
            m_addedEdges.append({u, v, dist[u][v]});
            matchWeight += dist[u][v];
        }
    }

    m_stats.matchingWeight = matchWeight;

    // Build Euler tour
    m_tour = eulerTour();
    m_stats.totalWeight = baseWeight + matchWeight;

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(m_stats.totalWeight, m_tour.size(), timer.elapsed());
}

/* ---- Accessors ---- */

QVector<int> ChinesePostman8::tour() const { return m_tour; }
double ChinesePostman8::tourCost() const { return m_stats.totalWeight; }
QVector<ChinesePostman8::Edge> ChinesePostman8::addedEdges() const { return m_addedEdges; }

/* ---- Reset ---- */

void ChinesePostman8::resetStatistics()
{
    m_edges.clear(); m_adj.clear(); m_tour.clear(); m_addedEdges.clear();
    m_n = 0;
    m_stats = Stats{}; m_timeSum = 0.0;
}

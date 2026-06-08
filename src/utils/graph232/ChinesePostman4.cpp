/**
 * @file ChinesePostman4.cpp
 * @brief ChinesePostman4 实现
 *
 * 实现中国邮路问题：Floyd-Warshall最短路径、Edmonds-Johnson最小权完美匹配、Hierholzer欧拉回路。
 */

#include "utils/graph232/ChinesePostman4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

ChinesePostman4::ChinesePostman4(QObject *parent) : QObject(parent) {}
ChinesePostman4::~ChinesePostman4() = default;

/* ---- Set graph ---- */

void ChinesePostman4::setGraph(int vertices, const QVector<Edge>& edges)
{
    m_n = vertices;
    m_edges = edges;

    // Build adjacency list
    m_adj.resize(m_n);
    for (auto& list : m_adj) list.clear();
    for (const auto& e : edges) {
        if (e.from >= 0 && e.from < m_n && e.to >= 0 && e.to < m_n) {
            m_adj[e.from].append({e.to, e.weight});
            m_adj[e.to].append({e.from, e.weight});
        }
    }

    m_stats.numVertices = m_n;
    m_stats.numEdges = edges.size();
}

/* ---- Find odd-degree vertices ---- */

QVector<int> ChinesePostman4::findOddVertices() const
{
    QVector<int> odd;
    for (int v = 0; v < m_n; ++v) {
        if (m_adj[v].size() % 2 != 0) odd.append(v);
    }
    return odd;
}

/* ---- Floyd-Warshall all-pairs shortest paths ---- */

QVector<QVector<double>> ChinesePostman4::floydWarshall() const
{
    double inf = std::numeric_limits<double>::max() / 2.0;
    QVector<QVector<double>> dist(m_n, QVector<double>(m_n, inf));

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

/* ---- Edmonds-Johnson minimum weight perfect matching (greedy) ---- */

QVector<QPair<int, int>> ChinesePostman4::minWeightMatching(
    const QVector<int>& oddVerts,
    const QVector<QVector<double>>& dist)
{
    int m = oddVerts.size();
    QVector<QPair<int, int>> matching;
    QVector<bool> used(m, false);

    // Greedy: pick cheapest edge repeatedly
    while (true) {
        double bestCost = std::numeric_limits<double>::max();
        int bestI = -1, bestJ = -1;
        for (int i = 0; i < m; ++i) {
            if (used[i]) continue;
            for (int j = i + 1; j < m; ++j) {
                if (used[j]) continue;
                double d = dist[oddVerts[i]][oddVerts[j]];
                if (d < bestCost) {
                    bestCost = d;
                    bestI = i;
                    bestJ = j;
                }
            }
        }
        if (bestI < 0) break;
        used[bestI] = true;
        used[bestJ] = true;
        matching.append({oddVerts[bestI], oddVerts[bestJ]});
    }

    return matching;
}

/* ---- Hierholzer Euler tour ---- */

QVector<int> ChinesePostman4::eulerTour(
    const QVector<QVector<QPair<int, double>>>& augmentedAdj) const
{
    QVector<int> tour;
    QVector<QVector<bool>> used(m_n, QVector<bool>(m_n, false));

    // Stack-based Hierholzer
    QVector<int> stack;
    stack.append(0);

    while (!stack.isEmpty()) {
        int v = stack.back();
        bool found = false;
        for (auto& edge : augmentedAdj[v]) {
            int u = edge.first;
            if (!used[v][u]) {
                used[v][u] = true;
                used[u][v] = true;
                stack.append(u);
                found = true;
                break;
            }
        }
        if (!found) {
            tour.append(v);
            stack.removeLast();
        }
    }

    return tour;
}

/* ---- Solve ---- */

QVector<int> ChinesePostman4::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) return {};

    // Find odd vertices
    QVector<int> odd = findOddVertices();
    m_stats.numOddVertices = odd.size();

    // All-pairs shortest paths
    QVector<QVector<double>> dist = floydWarshall();

    // If already Eulerian, just find tour
    if (odd.isEmpty()) {
        m_matchingCost = 0.0;
        QVector<int> tour = eulerTour(m_adj);
        m_stats.totalTourCost = 0.0;
        for (const auto& e : m_edges) m_stats.totalTourCost += e.weight;
        m_stats.totalOps++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
        emit solved(m_stats.totalTourCost, 0.0, timer.elapsed());
        return tour;
    }

    // Min-weight perfect matching on odd vertices
    auto matching = minWeightMatching(odd, dist);
    m_matchingCost = 0.0;
    for (const auto& p : matching)
        m_matchingCost += dist[p.first][p.second];

    // Duplicate edges along shortest paths
    QVector<QVector<QPair<int, double>>> augmentedAdj = m_adj;
    for (const auto& p : matching) {
        // Add edge (duplicate) with matching weight
        augmentedAdj[p.first].append({p.second, dist[p.first][p.second]});
        augmentedAdj[p.second].append({p.first, dist[p.first][p.second]});
    }

    QVector<int> tour = eulerTour(augmentedAdj);

    m_stats.totalTourCost = 0.0;
    for (const auto& e : m_edges) m_stats.totalTourCost += e.weight;
    m_stats.totalTourCost += m_matchingCost;
    m_stats.matchingCost = m_matchingCost;

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solved(m_stats.totalTourCost, m_matchingCost, timer.elapsed());
    return tour;
}

/* ---- Odd shortest paths ---- */

QVector<QVector<double>> ChinesePostman4::oddShortestPaths() const
{
    return floydWarshall();
}

/* ---- Matching cost ---- */

double ChinesePostman4::matchingCost() const { return m_matchingCost; }

/* ---- Is Eulerian ---- */

bool ChinesePostman4::isEulerian() const { return findOddVertices().isEmpty(); }

/* ---- Reset ---- */

void ChinesePostman4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_matchingCost = 0.0;
    m_adj.clear();
    m_edges.clear();
    m_n = 0;
}

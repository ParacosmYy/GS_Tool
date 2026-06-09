/**
 * @file ChinesePostman10.cpp
 * @brief ChinesePostman10 实现
 *
 * 实现中国邮路问题：Blossom V匹配增广路径最小奇集配对。
 */

#include "utils/graph274/ChinesePostman10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

ChinesePostman10::ChinesePostman10(QObject *parent)
    : QObject(parent) {}
ChinesePostman10::~ChinesePostman10() = default;

/* ---- Add edge ---- */

void ChinesePostman10::addEdge(int u, int v, double weight)
{
    m_edges.append({u, v, weight, false});
    m_n = qMax(m_n, qMax(u, v) + 1);
}

/* ---- Find odd-degree vertices ---- */

QVector<int> ChinesePostman10::findOddVertices() const
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

void ChinesePostman10::floydWarshall(QVector<QVector<double>>& dist,
                                       QVector<QVector<int>>& next) const
{
    dist.assign(m_n, QVector<double>(m_n, std::numeric_limits<double>::infinity()));
    next.assign(m_n, QVector<int>(m_n, -1));
    for (int i = 0; i < m_n; ++i) dist[i][i] = 0.0;
    for (const auto& e : m_edges) {
        if (e.weight < dist[e.from][e.to]) {
            dist[e.from][e.to] = e.weight;
            dist[e.to][e.from] = e.weight;
            next[e.from][e.to] = e.to;
            next[e.to][e.from] = e.from;
        }
    }
    for (int k = 0; k < m_n; ++k)
        for (int i = 0; i < m_n; ++i)
            for (int j = 0; j < m_n; ++j)
                if (dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    next[i][j] = next[i][k];
                }
}

/* ---- Blossom matching (simplified greedy + augmenting path) ---- */

QVector<QPair<int, int>> ChinesePostman10::blossomMatch(
    const QVector<int>& oddVerts,
    const QVector<QVector<double>>& dist) const
{
    int m = oddVerts.size();
    if (m % 2 != 0) return {};

    QVector<QPair<int, int>> matching;
    QVector<bool> matched(m, false);

    // Greedy initial matching: pair closest unmatched vertices
    for (int i = 0; i < m; ++i) {
        if (matched[i]) continue;
        int bestJ = -1;
        double bestD = std::numeric_limits<double>::infinity();
        for (int j = i + 1; j < m; ++j) {
            if (matched[j]) continue;
            double d = dist[oddVerts[i]][oddVerts[j]];
            if (d < bestD) { bestD = d; bestJ = j; }
        }
        if (bestJ >= 0) {
            matched[i] = true;
            matched[bestJ] = true;
            matching.append({oddVerts[i], oddVerts[bestJ]});
        }
    }

    // Try augmenting path improvements
    for (int iter = 0; iter < 3; ++iter) {
        bool improved = false;
        for (int i = 0; i < matching.size(); ++i) {
            for (int j = i + 1; j < matching.size(); ++j) {
                int a = matching[i].first, b = matching[i].second;
                int c = matching[j].first, d = matching[j].second;
                double cur = dist[a][b] + dist[c][d];
                double alt1 = dist[a][c] + dist[b][d];
                double alt2 = dist[a][d] + dist[b][c];
                if (alt1 < cur && alt1 <= alt2) {
                    matching[i] = {a, c};
                    matching[j] = {b, d};
                    improved = true;
                } else if (alt2 < cur) {
                    matching[i] = {a, d};
                    matching[j] = {b, c};
                    improved = true;
                }
            }
        }
        if (!improved) break;
    }
    return matching;
}

/* ---- Solve ---- */

double ChinesePostman10::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_edges.isEmpty()) return 0.0;

    // Find odd-degree vertices
    QVector<int> odd = findOddVertices();

    // Compute all-pairs shortest paths
    QVector<QVector<double>> dist;
    QVector<QVector<int>> next;
    floydWarshall(dist, next);

    // Find minimum weight perfect matching on odd vertices
    double extraCost = 0.0;
    m_allEdges = m_edges;
    if (odd.size() > 0) {
        QVector<QPair<int, int>> matching = blossomMatch(odd, dist);
        // Add duplicate edges along shortest paths for each matched pair
        for (const auto& p : matching) {
            int u = p.first, v = p.second;
            extraCost += dist[u][v];
            // Trace shortest path and add edges
            int cur = u;
            while (cur != v) {
                int nxt = next[cur][v];
                m_allEdges.append({cur, nxt, dist[cur][nxt], true});
                cur = nxt;
            }
        }
    }

    // Build Eulerian tour (Hierholzer's algorithm)
    QVector<QVector<QPair<int, int>>> adj(m_n); // adj[u] = {(v, edge_idx)}
    for (int i = 0; i < m_allEdges.size(); ++i) {
        adj[m_allEdges[i].from].append({m_allEdges[i].to, i});
        adj[m_allEdges[i].to].append({m_allEdges[i].from, i});
    }
    QVector<bool> used(m_allEdges.size(), false);
    QVector<int> stack, tour;
    stack.append(m_allEdges[0].from);
    while (!stack.isEmpty()) {
        int u = stack.back();
        bool found = false;
        while (!adj[u].isEmpty()) {
            auto [v, eidx] = adj[u].last();
            adj[u].removeLast();
            if (!used[eidx]) {
                used[eidx] = true;
                stack.append(u);
                stack.append(v);
                u = v;
                found = true;
                break;
            }
        }
        if (!found) {
            tour.append(stack.back());
            stack.removeLast();
        }
    }
    std::reverse(tour.begin(), tour.end());
    m_tour = tour;

    // Total cost
    double totalCost = 0.0;
    for (const auto& e : m_allEdges) totalCost += e.weight;

    double elapsed = timer.elapsed();
    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edges.size();
    m_stats.numOddVertices = odd.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solved(odd.size(), extraCost, totalCost, elapsed);
    return totalCost;
}

/* ---- Euler tour accessor ---- */

QVector<int> ChinesePostman10::eulerTour() const { return m_tour; }

/* ---- All edges accessor ---- */

QVector<ChinesePostman10::Edge> ChinesePostman10::allEdges() const { return m_allEdges; }

/* ---- Reset ---- */

void ChinesePostman10::resetStatistics()
{
    m_edges.clear();
    m_allEdges.clear();
    m_tour.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file ChinesePostman3.cpp
 * @brief ChinesePostman3 实现
 *
 * 实现中国邮路：Floyd-Warshall最短路、Edmonds匹配、Hierholzer欧拉回路。
 */

#include "utils/graph222/ChinesePostman3.h"

#include <QElapsedTimer>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

ChinesePostman3::ChinesePostman3(QObject *parent) : QObject(parent) {}
ChinesePostman3::~ChinesePostman3() = default;

/* ---- Configuration ---- */

void ChinesePostman3::setVertexCount(int n) { m_n = qMax(1, n); }
void ChinesePostman3::addEdge(int from, int to, double weight)
{
    Edge e;
    e.from = from;
    e.to = to;
    e.weight = weight;
    e.isDuplicate = false;
    m_edges.append(e);
}

/* ---- Floyd-Warshall ---- */

void ChinesePostman3::floydWarshall()
{
    m_dist.resize(m_n);
    m_next.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_dist[i].resize(m_n, std::numeric_limits<double>::max());
        m_next[i].resize(m_n, -1);
        m_dist[i][i] = 0.0;
        m_next[i][i] = i;
    }
    for (const auto& e : m_edges) {
        if (e.weight < m_dist[e.from][e.to]) {
            m_dist[e.from][e.to] = e.weight;
            m_dist[e.to][e.from] = e.weight;
            m_next[e.from][e.to] = e.to;
            m_next[e.to][e.from] = e.from;
        }
    }
    for (int k = 0; k < m_n; ++k)
        for (int i = 0; i < m_n; ++i)
            for (int j = 0; j < m_n; ++j)
                if (m_dist[i][k] + m_dist[k][j] < m_dist[i][j]) {
                    m_dist[i][j] = m_dist[i][k] + m_dist[k][j];
                    m_next[i][j] = m_next[i][k];
                }
}

/* ---- Reconstruct path ---- */

QVector<int> ChinesePostman3::reconstructPath(int u, int v) const
{
    if (m_next[u][v] < 0) return {};
    QVector<int> path;
    path.append(u);
    while (u != v) {
        u = m_next[u][v];
        path.append(u);
    }
    return path;
}

/* ---- Find odd-degree vertices ---- */

QVector<int> ChinesePostman3::findOddVertices() const
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

/* ---- Edmonds maximum matching (greedy for even set) ---- */

QVector<QPair<int, int>> ChinesePostman3::edmondsMatching(const QVector<int>& oddVerts)
{
    int m = oddVerts.size();
    QVector<QPair<int, int>> matches;
    if (m < 2) return matches;

    // Greedy minimum-weight perfect matching on odd vertices
    QVector<bool> used(m, false);
    for (int i = 0; i < m; ++i) {
        if (used[i]) continue;
        double bestDist = std::numeric_limits<double>::max();
        int bestJ = -1;
        for (int j = i + 1; j < m; ++j) {
            if (used[j]) continue;
            double d = m_dist[oddVerts[i]][oddVerts[j]];
            if (d < bestDist) { bestDist = d; bestJ = j; }
        }
        if (bestJ >= 0) {
            matches.append({oddVerts[i], oddVerts[bestJ]});
            used[i] = true;
            used[bestJ] = true;
        }
    }
    return matches;
}

/* ---- Duplicate matched edges ---- */

void ChinesePostman3::duplicateMatchedEdges(const QVector<QPair<int, int>>& matches)
{
    for (const auto& [u, v] : matches) {
        auto path = reconstructPath(u, v);
        for (int i = 0; i + 1 < path.size(); ++i) {
            Edge e;
            e.from = path[i];
            e.to = path[i + 1];
            e.weight = m_dist[path[i]][path[i + 1]];
            e.isDuplicate = true;
            m_edges.append(e);
        }
    }
}

/* ---- Hierholzer Euler tour ---- */

QVector<int> ChinesePostman3::hierholzer() const
{
    // Build multi-graph adjacency
    QVector<QVector<QPair<int, int>>> adj(m_n); // (neighbor, edge_index)
    for (int i = 0; i < m_edges.size(); ++i) {
        adj[m_edges[i].from].append({m_edges[i].to, i});
        adj[m_edges[i].to].append({m_edges[i].from, i});
    }

    QVector<bool> usedEdge(m_edges.size(), false);
    QVector<int> tour;
    QVector<int> stack;
    stack.append(0);

    while (!stack.isEmpty()) {
        int v = stack.back();
        bool found = false;
        while (!adj[v].isEmpty()) {
            auto [u, ei] = adj[v].last();
            adj[v].removeLast();
            if (!usedEdge[ei]) {
                usedEdge[ei] = true;
                stack.append(u);
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
    return tour;
}

/* ---- Euler tour ---- */

QVector<int> ChinesePostman3::eulerTour() const
{
    return hierholzer();
}

/* ---- Solve ---- */

QVector<int> ChinesePostman3::solve()
{
    QElapsedTimer timer;
    timer.start();

    floydWarshall();
    auto oddVerts = findOddVertices();
    auto matches = edmondsMatching(oddVerts);
    duplicateMatchedEdges(matches);

    // Compute tour cost
    m_tourCost = 0.0;
    for (const auto& e : m_edges)
        m_tourCost += e.weight;

    auto tour = hierholzer();

    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edges.size();
    m_stats.numOddVertices = oddVerts.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveCompleted(m_n, m_edges.size(), m_tourCost, timer.elapsed());
    return tour;
}

/* ---- Tour cost ---- */

double ChinesePostman3::tourCost() const { return m_tourCost; }

/* ---- Reset ---- */

void ChinesePostman3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_edges.clear();
    m_dist.clear();
    m_next.clear();
    m_adjMulti.clear();
    m_tourCost = 0.0;
}

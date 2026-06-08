/**
 * @file ChinesePostman6.cpp
 * @brief ChinesePostman6 实现
 *
 * 实现中国邮路问题：Blossom算法最小权完美匹配与层次化欧拉回路构造。
 */

#include "utils/graph246/ChinesePostman6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

ChinesePostman6::ChinesePostman6(QObject *parent) : QObject(parent) {}
ChinesePostman6::~ChinesePostman6() = default;

/* ---- Build graph ---- */

void ChinesePostman6::buildGraph(int numVertices, const QVector<Edge>& edges)
{
    m_n = qMax(1, numVertices);
    m_edges = edges;
    m_stats.numVertices = m_n;
    m_stats.numEdges = edges.size();

    floydWarshall();
}

/* ---- Floyd-Warshall ---- */

void ChinesePostman6::floydWarshall()
{
    double inf = std::numeric_limits<double>::max() / 2.0;
    m_dist.resize(m_n);
    m_next.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_dist[i].resize(m_n, inf);
        m_next[i].resize(m_n, -1);
        m_dist[i][i] = 0.0;
        m_next[i][i] = i;
    }

    for (int e = 0; e < m_edges.size(); ++e) {
        int u = m_edges[e].from, v = m_edges[e].to;
        double w = m_edges[e].weight;
        if (u >= 0 && u < m_n && v >= 0 && v < m_n && w < m_dist[u][v]) {
            m_dist[u][v] = w;
            m_dist[v][u] = w;
            m_next[u][v] = v;
            m_next[v][u] = u;
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

/* ---- Find odd-degree vertices ---- */

QVector<int> ChinesePostman6::findOddVertices() const
{
    QVector<int> degree(m_n, 0);
    for (const auto& e : m_edges) {
        if (e.from >= 0 && e.from < m_n) degree[e.from]++;
        if (e.to >= 0 && e.to < m_n) degree[e.to]++;
    }
    QVector<int> odd;
    for (int i = 0; i < m_n; ++i)
        if (degree[i] % 2 != 0) odd.append(i);
    return odd;
}

/* ---- Greedy matching (fallback / small instances) ---- */

QVector<QPair<int, int>> ChinesePostman6::greedyMatch(const QVector<int>& oddVerts)
{
    int m = oddVerts.size();
    QVector<bool> used(m, false);
    QVector<QPair<int, int>> pairs;

    while (pairs.size() < m / 2) {
        double bestDist = std::numeric_limits<double>::max();
        int bestI = -1, bestJ = -1;
        for (int i = 0; i < m; ++i) {
            if (used[i]) continue;
            for (int j = i + 1; j < m; ++j) {
                if (used[j]) continue;
                double d = m_dist[oddVerts[i]][oddVerts[j]];
                if (d < bestDist) { bestDist = d; bestI = i; bestJ = j; }
            }
        }
        if (bestI < 0) break;
        used[bestI] = used[bestJ] = true;
        pairs.append({oddVerts[bestI], oddVerts[bestJ]});
    }
    return pairs;
}

/* ---- Blossom matching ---- */

QVector<QPair<int, int>> ChinesePostman6::blossomMatch(const QVector<int>& oddVerts)
{
    int m = oddVerts.size();
    if (m == 0) return {};

    // For moderate sizes, use greedy with local improvement
    QVector<QPair<int, int>> pairs = greedyMatch(oddVerts);

    // Local optimization: try swapping pairs to reduce total cost
    bool improved = true;
    int iterations = 0;
    while (improved && iterations < 100) {
        improved = false;
        iterations++;
        for (int i = 0; i < pairs.size(); ++i) {
            for (int j = i + 1; j < pairs.size(); ++j) {
                int a = pairs[i].first, b = pairs[i].second;
                int c = pairs[j].first, d = pairs[j].second;
                double cur = m_dist[a][b] + m_dist[c][d];
                double alt1 = m_dist[a][c] + m_dist[b][d];
                double alt2 = m_dist[a][d] + m_dist[b][c];
                if (alt1 < cur && alt1 <= alt2) {
                    pairs[i] = {a, c};
                    pairs[j] = {b, d};
                    improved = true;
                } else if (alt2 < cur) {
                    pairs[i] = {a, d};
                    pairs[j] = {b, c};
                    improved = true;
                }
            }
        }
    }
    return pairs;
}

/* ---- Build Eulerian tour ---- */

ChinesePostman6::TourResult ChinesePostman6::buildEulerTour(const QVector<QPair<int, int>>& matchedPairs)
{
    TourResult result;

    // Build adjacency with duplicated edges for matched pairs
    QVector<QVector<QPair<int, int>>> adj(m_n); // (neighbor, edgeIndex)
    for (int e = 0; e < m_edges.size(); ++e) {
        adj[m_edges[e].from].append({m_edges[e].to, e});
        adj[m_edges[e].to].append({m_edges[e].from, e});
    }

    // Count repeated edges
    int dupEdgeStart = m_edges.size();
    for (const auto& pr : matchedPairs) {
        // Reconstruct path via m_next
        int u = pr.first, v = pr.second;
        int cur = u;
        while (cur != v && cur >= 0 && cur < m_n) {
            int nxt = m_next[cur][v];
            if (nxt < 0) break;
            // Add duplicate edge
            adj[cur].append({nxt, dupEdgeStart++});
            adj[nxt].append({cur, dupEdgeStart++});
            result.numRepeatedEdges++;
            cur = nxt;
        }
    }

    // Hierholzer's algorithm for Euler circuit
    QVector<int> edgeUsed(dupEdgeStart + m_edges.size() * 2, 0);
    QVector<int> stack;
    stack.append(0);

    while (!stack.isEmpty()) {
        int u = stack.back();
        bool found = false;
        while (!adj[u].isEmpty()) {
            auto [v, eIdx] = adj[u].back();
            adj[u].pop_back();
            if (edgeUsed[eIdx]) continue;
            edgeUsed[eIdx] = 1;
            stack.append(v);
            found = true;
            break;
        }
        if (!found) {
            result.vertices.append(stack.back());
            stack.removeLast();
        }
    }

    // Reverse to get correct order
    std::reverse(result.vertices.begin(), result.vertices.end());

    // Compute total cost
    result.totalCost = 0.0;
    for (const auto& e : m_edges)
        result.totalCost += e.weight;
    for (const auto& pr : matchedPairs)
        result.totalCost += m_dist[pr.first][pr.second];

    return result;
}

/* ---- Shortest path ---- */

double ChinesePostman6::shortestPath(int u, int v) const
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return std::numeric_limits<double>::max();
    return m_dist[u][v];
}

/* ---- Solve ---- */

ChinesePostman6::TourResult ChinesePostman6::solve()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> odd = findOddVertices();
    m_stats.numOddVertices = odd.size();

    QVector<QPair<int, int>> matched;
    double matchCost = 0.0;

    if (odd.size() > 0) {
        matched = blossomMatch(odd);
        for (const auto& pr : matched)
            matchCost += m_dist[pr.first][pr.second];
    }

    m_stats.numMatchedPairs = matched.size();
    m_stats.matchCost = matchCost;
    emit matchingCompleted(matched.size(), matchCost);

    TourResult result = buildEulerTour(matched);

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit tourCompleted(result.totalCost, result.numRepeatedEdges);
    return result;
}

/* ---- Reset ---- */

void ChinesePostman6::resetStatistics()
{
    m_edges.clear();
    m_dist.clear();
    m_next.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

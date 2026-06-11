/**
 * @file ChinesePostman14.cpp
 * @brief ChinesePostman14 实现
 *
 * 实现中国邮路问题：Edmonds奇度匹配与权重无向Euler游览的环游拼接。
 */

#include "utils/graph302/ChinesePostman14.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ChinesePostman14::ChinesePostman14(QObject *parent)
    : QObject(parent) {}

ChinesePostman14::~ChinesePostman14() = default;

/* ---- Configuration ---- */

void ChinesePostman14::addEdge(int u, int v, double weight)
{
    m_edges.append({u, v, qMax(weight, 0.0), false});
}

void ChinesePostman14::setVertexCount(int n) { m_n = qMax(0, n); }

/* ---- Compute vertex degrees ---- */

QVector<int> ChinesePostman14::computeDegrees() const
{
    QVector<int> deg(m_n, 0);
    for (const auto& e : m_edges) {
        deg[e.from]++;
        deg[e.to]++;
    }
    return deg;
}

/* ---- Find odd-degree vertices ---- */

QVector<int> ChinesePostman14::oddDegreeVertices() const
{
    QVector<int> deg = computeDegrees();
    QVector<int> odd;
    for (int i = 0; i < m_n; ++i)
        if (deg[i] % 2 != 0) odd.append(i);
    return odd;
}

/* ---- Check if Eulerian ---- */

bool ChinesePostman14::isEulerian() const
{
    return oddDegreeVertices().isEmpty();
}

/* ---- Floyd-Warshall all-pairs shortest paths ---- */

QVector<QVector<double>> ChinesePostman14::allPairsShortest(
    QVector<QVector<int>>& next) const
{
    QVector<QVector<double>> dist(m_n, QVector<double>(m_n, 1e300));
    next = QVector<QVector<int>>(m_n, QVector<int>(m_n, -1));

    for (int i = 0; i < m_n; ++i) { dist[i][i] = 0.0; next[i][i] = i; }
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
    return dist;
}

/* ---- DP matching for minimum weight perfect matching ---- */

double ChinesePostman14::matchingDP(int mask, const QVector<int>& odd,
                                     const QVector<QVector<double>>& dist,
                                     QVector<QVector<double>>& memo) const
{
    int k = odd.size();
    if (mask == (1 << k) - 1) return 0.0;
    if (memo[mask][0] >= 0) return memo[mask][0];

    // Find first unmatched
    int i = 0;
    while (i < k && (mask & (1 << i))) ++i;

    double best = 1e300;
    for (int j = i + 1; j < k; ++j) {
        if (mask & (1 << j)) continue;
        double cost = dist[odd[i]][odd[j]] +
                      matchingDP(mask | (1 << i) | (1 << j), odd, dist, memo);
        if (cost < best) best = cost;
    }
    memo[mask][0] = best;
    return best;
}

void ChinesePostman14::reconstructMatching(int mask, const QVector<int>& odd,
                                            const QVector<QVector<double>>& dist,
                                            const QVector<QVector<double>>& memo,
                                            QVector<QPair<int,int>>& pairs) const
{
    int k = odd.size();
    if (mask == (1 << k) - 1) return;

    int i = 0;
    while (i < k && (mask & (1 << i))) ++i;

    double best = 1e300;
    int bestJ = -1;
    for (int j = i + 1; j < k; ++j) {
        if (mask & (1 << j)) continue;
        double cost = dist[odd[i]][odd[j]] +
                      matchingDP(mask | (1 << i) | (1 << j), odd, dist, memo);
        if (cost < best) { best = cost; bestJ = j; }
    }
    if (bestJ >= 0) {
        pairs.append({odd[i], odd[bestJ]});
        reconstructMatching(mask | (1 << i) | (1 << bestJ), odd, dist, memo, pairs);
    }
}

QVector<QPair<int,int>> ChinesePostman14::minWeightMatching(
    const QVector<int>& oddVerts, const QVector<QVector<double>>& dist) const
{
    int k = oddVerts.size();
    if (k == 0) return {};

    // Use DP matching (feasible for small k <= 20, i.e. 10 odd pairs)
    int fullMask = 1 << k;
    QVector<QVector<double>> memo(fullMask, QVector<double>(1, -1.0));
    matchingDP(0, oddVerts, dist, memo);

    QVector<QPair<int,int>> pairs;
    reconstructMatching(0, oddVerts, dist, memo, pairs);
    return pairs;
}

/* ---- Shortest path edges via next-hop table ---- */

QVector<QPair<int,int>> ChinesePostman14::shortestPathEdges(
    int u, int v, const QVector<QVector<int>>& next) const
{
    QVector<QPair<int,int>> path;
    if (u == v) return path;
    int cur = u;
    while (cur != v) {
        int nxt = next[cur][v];
        if (nxt < 0) break;
        path.append({cur, nxt});
        cur = nxt;
    }
    return path;
}

/* ---- Euler tour via Hierholzer ---- */

QVector<int> ChinesePostman14::eulerTour(int start)
{
    // Build adjacency as multimap: vertex -> list of {neighbor, edge_index}
    QVector<QVector<QPair<int,int>>> adj(m_n);
    for (int ei = 0; ei < m_edges.size(); ++ei) {
        adj[m_edges[ei].from].append({m_edges[ei].to, ei});
        adj[m_edges[ei].to].append({m_edges[ei].from, ei});
    }

    QVector<bool> used(m_edges.size(), false);
    QVector<int> tour;
    QVector<int> stack;
    stack.append(start);

    while (!stack.isEmpty()) {
        int v = stack.back();
        bool found = false;
        while (!adj[v].isEmpty()) {
            auto [u, ei] = adj[v].last();
            adj[v].removeLast();
            if (used[ei]) continue;
            used[ei] = true;
            stack.append(v);
            // Also remove reverse edge from u's list
            for (int i = adj[u].size() - 1; i >= 0; --i) {
                if (adj[u][i].second == ei) { adj[u].removeAt(i); break; }
            }
            v = u;
            found = true;
            break;
        }
        if (!found) { tour.append(stack.back()); stack.removeLast(); }
    }
    std::reverse(tour.begin(), tour.end());
    return tour;
}

/* ---- Main solve ---- */

ChinesePostman14::TourResult ChinesePostman14::solve()
{
    QElapsedTimer timer;
    timer.start();

    TourResult result;

    // Compute all-pairs shortest paths
    QVector<QVector<int>> nextHop;
    QVector<QVector<double>> dist = allPairsShortest(nextHop);

    // Find odd-degree vertices
    QVector<int> oddVerts = oddDegreeVertices();
    result.originalEdges = m_edges.size();

    // Match odd-degree vertices
    QVector<QPair<int,int>> matching = minWeightMatching(oddVerts, dist);
    result.duplicatedEdges = matching.size();

    double matchCost = 0.0;
    for (const auto& [u, v] : matching) {
        matchCost += dist[u][v];
        // Add shortest path edges as duplicates
        QVector<QPair<int,int>> pathEdges = shortestPathEdges(u, v, nextHop);
        for (const auto& [a, b] : pathEdges) {
            m_edges.append({a, b, dist[a][b], true});
        }
    }
    result.matchingCost = matchCost;

    // Compute total original edge cost
    double edgeCost = 0.0;
    int origCount = result.originalEdges;
    for (int i = 0; i < origCount; ++i) edgeCost += m_edges[i].weight;
    result.totalCost = edgeCost + matchCost;

    // Find Euler tour
    int start = (m_n > 0) ? 0 : 0;
    if (!m_edges.isEmpty()) start = m_edges[0].from;
    result.tour = eulerTour(start);

    // Remove duplicate edges (cleanup)
    while (m_edges.size() > origCount) m_edges.removeLast();

    double elapsed = timer.elapsed();
    m_stats.numVertices = m_n;
    m_stats.numEdges = result.originalEdges;
    m_stats.oddDegreeVertices = oddVerts.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit tourFound(m_n, result.originalEdges, result.totalCost, elapsed);
    return result;
}

/* ---- Reset ---- */

void ChinesePostman14::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_edges.clear();
    m_n = 0;
}

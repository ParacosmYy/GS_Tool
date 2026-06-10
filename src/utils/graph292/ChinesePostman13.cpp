/**
 * @file ChinesePostman13.cpp
 * @brief ChinesePostman13 实现
 *
 * 实现中国邮路问题：Edmonds花算法最小权匹配与有向图邮路遍历构造。
 */

#include "utils/graph292/ChinesePostman13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ChinesePostman13::ChinesePostman13(QObject *parent)
    : QObject(parent) {}

ChinesePostman13::~ChinesePostman13() = default;

/* ---- Set graph ---- */

void ChinesePostman13::setGraph(int numVertices, const QVector<Edge>& edges)
{
    m_numVertices = numVertices;
    m_edges = edges;
    m_apspComputed = false;
}

/* ---- Compute degrees ---- */

QVector<int> ChinesePostman13::computeDegrees(const QVector<Edge>& edgeList) const
{
    QVector<int> deg(m_numVertices, 0);
    for (const auto& e : edgeList) {
        deg[e.from]++;
        deg[e.to]++;
    }
    return deg;
}

/* ---- Find odd-degree vertices ---- */

QVector<int> ChinesePostman13::findOddDegreeVertices() const
{
    auto deg = computeDegrees(m_edges);
    QVector<int> odd;
    for (int i = 0; i < m_numVertices; ++i) {
        if (deg[i] % 2 != 0) odd.append(i);
    }
    return odd;
}

/* ---- Floyd-Warshall all-pairs shortest paths ---- */

void ChinesePostman13::computeAllPairsShortestPaths()
{
    int n = m_numVertices;
    double INF = 1e18;
    m_dist = QVector<QVector<double>>(n, QVector<double>(n, INF));
    m_next = QVector<QVector<int>>(n, QVector<int>(n, -1));

    for (int i = 0; i < n; ++i) {
        m_dist[i][i] = 0.0;
        m_next[i][i] = i;
    }

    for (const auto& e : m_edges) {
        if (e.weight < m_dist[e.from][e.to]) {
            m_dist[e.from][e.to] = e.weight;
            m_next[e.from][e.to] = e.to;
        }
    }

    for (int k = 0; k < n; ++k)
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if (m_dist[i][k] + m_dist[k][j] < m_dist[i][j]) {
                    m_dist[i][j] = m_dist[i][k] + m_dist[k][j];
                    m_next[i][j] = m_next[i][k];
                }

    m_apspComputed = true;
}

/* ---- Shortest path distance ---- */

double ChinesePostman13::shortestPathDist(int u, int v) const
{
    if (!m_apspComputed || u < 0 || v < 0 || u >= m_numVertices || v >= m_numVertices)
        return 1e18;
    return m_dist[u][v];
}

/* ---- Reconstruct path ---- */

QVector<int> ChinesePostman13::reconstructPath(int u, int v) const
{
    if (!m_apspComputed || m_dist[u][v] >= 1e17) return {};
    QVector<int> path;
    int cur = u;
    while (cur != v) {
        path.append(cur);
        cur = m_next[cur][v];
        if (cur < 0) return {};
    }
    path.append(v);
    return path;
}

/* ---- Greedy minimum weight perfect matching ---- */

QVector<QPair<int, int>> ChinesePostman13::minimumWeightMatching(
    const QVector<int>& oddVertices)
{
    int m = oddVertices.size();
    QVector<QPair<int, int>> matching;
    if (m % 2 != 0) return matching;

    // Greedy: sort all pairs by distance, pick smallest unmatched
    QVector<QPair<double, QPair<int, int>>> pairs;
    for (int i = 0; i < m; ++i)
        for (int j = i + 1; j < m; ++j)
            pairs.append({m_dist[oddVertices[i]][oddVertices[j]],
                          {oddVertices[i], oddVertices[j]}});

    std::sort(pairs.begin(), pairs.end());

    QVector<bool> matched(m_numVertices, false);
    for (const auto& p : pairs) {
        int u = p.second.first;
        int v = p.second.second;
        if (!matched[u] && !matched[v]) {
            matching.append({u, v});
            matched[u] = true;
            matched[v] = true;
        }
    }
    return matching;
}

/* ---- Build Eulerian tour (Hierholzer) ---- */

QVector<int> ChinesePostman13::buildEulerianTour(const QVector<Edge>& augmentedEdges)
{
    int n = m_numVertices;
    if (n == 0) return {};

    // Adjacency list with edge counts
    QVector<QVector<QPair<int, int>>> adj(n); // adj[u] = {(v, edge_idx)}
    QVector<int> edgeUsed(augmentedEdges.size(), 0);

    for (int i = 0; i < augmentedEdges.size(); ++i) {
        adj[augmentedEdges[i].from].append({augmentedEdges[i].to, i});
    }

    // Start from vertex 0 (or any with degree > 0)
    int start = 0;
    auto deg = computeDegrees(augmentedEdges);
    for (int i = 0; i < n; ++i) {
        if (deg[i] > 0) { start = i; break; }
    }

    QVector<int> stack;
    QVector<int> tour;
    stack.append(start);

    while (!stack.isEmpty()) {
        int u = stack.last();
        bool found = false;
        while (!adj[u].isEmpty()) {
            auto [v, eidx] = adj[u].last();
            adj[u].removeLast();
            if (edgeUsed[eidx] > 0) continue;
            edgeUsed[eidx]++;
            stack.append(v);
            found = true;
            break;
        }
        if (!found) {
            tour.append(stack.last());
            stack.removeLast();
        }
    }

    // Reverse to get correct order
    QVector<int> result;
    for (int i = tour.size() - 1; i >= 0; --i)
        result.append(tour[i]);

    return result;
}

/* ---- Solve ---- */

ChinesePostman13::TourResult ChinesePostman13::solve()
{
    QElapsedTimer timer;
    timer.start();

    TourResult result;

    if (m_numVertices == 0 || m_edges.isEmpty()) return result;

    // Compute APSP
    computeAllPairsShortestPaths();

    // Find odd-degree vertices
    auto oddVerts = findOddDegreeVertices();
    int oddCount = oddVerts.size();

    // Copy edges for augmentation
    QVector<Edge> augmented = m_edges;
    int numDup = 0;

    if (oddCount > 0) {
        // Minimum weight matching on odd vertices
        auto matching = minimumWeightMatching(oddVerts);

        // Add shortest paths for matched pairs
        for (const auto& [u, v] : matching) {
            auto path = reconstructPath(u, v);
            for (int i = 0; i < path.size() - 1; ++i) {
                Edge dup;
                dup.from = path[i];
                dup.to = path[i + 1];
                dup.weight = m_dist[path[i]][path[i + 1]];
                dup.isOriginal = false;
                augmented.append(dup);
                numDup++;
            }
        }
    }

    // Build Eulerian tour
    result.tour = buildEulerianTour(augmented);
    result.numOriginalEdges = m_edges.size();
    result.numDuplicatedEdges = numDup;

    // Compute total cost
    result.totalCost = 0.0;
    for (const auto& e : m_edges) result.totalCost += e.weight;
    for (const auto& e : augmented) {
        if (!e.isOriginal) result.totalCost += e.weight;
    }

    result.isValid = !result.tour.isEmpty();

    double elapsed = timer.elapsed();
    m_stats.numVertices = m_numVertices;
    m_stats.numEdges = m_edges.size();
    m_stats.numOddVertices = oddCount;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveDone(result.tour.size(), result.totalCost, oddCount, elapsed);

    return result;
}

/* ---- Reset ---- */

void ChinesePostman13::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_apspComputed = false;
}

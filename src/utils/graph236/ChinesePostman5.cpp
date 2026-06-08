/**
 * @file ChinesePostman5.cpp
 * @brief ChinesePostman5 实现
 *
 * 实现中国邮路问题：Floyd-Warshall最短路、Blossom V简化匹配、Hierholzer欧拉回路。
 */

#include "utils/graph236/ChinesePostman5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

ChinesePostman5::ChinesePostman5(QObject *parent) : QObject(parent) {}
ChinesePostman5::~ChinesePostman5() = default;

/* ---- Build graph ---- */

void ChinesePostman5::buildGraph(int vertices, const QVector<Edge>& edges)
{
    m_n = vertices;
    m_edges = edges;
    m_stats.numVertices = vertices;
    m_stats.numEdges = edges.size();

    // Build adjacency matrix
    m_adjMatrix.resize(vertices);
    for (int i = 0; i < vertices; ++i)
        m_adjMatrix[i].resize(vertices, std::numeric_limits<double>::infinity());

    for (const auto& e : edges) {
        if (e.from >= 0 && e.from < vertices && e.to >= 0 && e.to < vertices) {
            m_adjMatrix[e.from][e.to] = qMin(m_adjMatrix[e.from][e.to], e.weight);
            m_adjMatrix[e.to][e.from] = qMin(m_adjMatrix[e.to][e.from], e.weight);
        }
    }
}

/* ---- Find odd-degree vertices ---- */

QVector<int> ChinesePostman5::findOddVertices() const
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

/* ---- Floyd-Warshall all-pairs shortest path ---- */

void ChinesePostman5::floydWarshall(QVector<QVector<double>>& dist) const
{
    int n = m_n;
    dist = m_adjMatrix;
    for (int i = 0; i < n; ++i) dist[i][i] = 0.0;

    for (int k = 0; k < n; ++k)
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if (dist[i][k] + dist[k][j] < dist[i][j])
                    dist[i][j] = dist[i][k] + dist[k][j];
}

/* ---- Build odd subgraph ---- */

QVector<QVector<double>> ChinesePostman5::buildOddSubgraph(
    const QVector<int>& oddVerts, const QVector<QVector<double>>& dist) const
{
    int m = oddVerts.size();
    QVector<QVector<double>> W(m, QVector<double>(m, 0.0));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < m; ++j)
            W[i][j] = dist[oddVerts[i]][oddVerts[j]];
    return W;
}

/* ---- Greedy minimum-weight perfect matching (Blossom V simplified) ---- */

QVector<QPair<int,int>> ChinesePostman5::minWeightPerfectMatch(
    const QVector<int>& oddVerts, const QVector<QVector<double>>& weights) const
{
    int m = oddVerts.size();
    QVector<bool> used(m, false);
    QVector<QPair<int,int>> matching;

    // Greedy: repeatedly pair closest unmatched vertices
    for (int iter = 0; iter < m / 2; ++iter) {
        double bestW = std::numeric_limits<double>::max();
        int bi = -1, bj = -1;
        for (int i = 0; i < m; ++i) {
            if (used[i]) continue;
            for (int j = i + 1; j < m; ++j) {
                if (used[j]) continue;
                if (weights[i][j] < bestW) {
                    bestW = weights[i][j];
                    bi = i; bj = j;
                }
            }
        }
        if (bi >= 0 && bj >= 0) {
            used[bi] = true;
            used[bj] = true;
            matching.append({bi, bj});
        }
    }
    return matching;
}

/* ---- Blossom V matching (public API) ---- */

QVector<QPair<int,int>> ChinesePostman5::blossomVMatch(
    const QVector<int>& oddVerts) const
{
    QVector<QVector<double>> dist;
    floydWarshall(dist);
    QVector<QVector<double>> weights = buildOddSubgraph(oddVerts, dist);
    return minWeightPerfectMatch(oddVerts, weights);
}

/* ---- Build multigraph with duplicated edges ---- */

void ChinesePostman5::buildMultigraph(
    const QVector<QPair<int,int>>& matched,
    const QVector<int>& oddVerts,
    const QVector<QVector<double>>& dist,
    QVector<QVector<QPair<int,double>>>& mg) const
{
    mg.resize(m_n);
    for (int i = 0; i < m_n; ++i) mg[i].clear();

    // Add original edges
    for (const auto& e : m_edges) {
        mg[e.from].append({e.to, e.weight});
        mg[e.to].append({e.from, e.weight});
    }

    // Add duplicated shortest paths for matched odd pairs
    for (const auto& pair : matched) {
        int u = oddVerts[pair.first];
        int v = oddVerts[pair.second];
        mg[u].append({v, dist[u][v]});
        mg[v].append({u, dist[u][v]});
    }
}

/* ---- Hierholzer Euler tour ---- */

QVector<int> ChinesePostman5::hierholzerEuler(
    const QVector<QVector<QPair<int,double>>>& multigraph) const
{
    QVector<QVector<QPair<int,double>>> mg = multigraph;
    QVector<int> path;
    QVector<int> circuit;

    path.append(0);

    while (!path.isEmpty()) {
        int v = path.back();
        if (!mg[v].isEmpty()) {
            auto edge = mg[v].back();
            mg[v].pop_back();
            // Remove reverse edge
            for (int i = mg[edge.first].size() - 1; i >= 0; --i) {
                if (mg[edge.first][i].first == v) {
                    mg[edge.first].removeAt(i);
                    break;
                }
            }
            path.append(edge.first);
        } else {
            circuit.append(v);
            path.pop_back();
        }
    }
    std::reverse(circuit.begin(), circuit.end());
    return circuit;
}

/* ---- Solve ---- */

ChinesePostman5::TourResult ChinesePostman5::solve()
{
    QElapsedTimer timer;
    timer.start();

    TourResult result;

    // Find odd-degree vertices
    QVector<int> oddVerts = findOddVertices();
    m_stats.numOddVertices = oddVerts.size();

    if (oddVerts.isEmpty()) {
        // Already Eulerian
        QVector<QVector<QPair<int,double>>> mg;
        for (int i = 0; i < m_n; ++i) mg.append(QVector<QPair<int,double>>());
        for (const auto& e : m_edges) {
            mg[e.from].append({e.to, e.weight});
            mg[e.to].append({e.from, e.weight});
        }
        result.tour = hierholzerEuler(mg);
        result.isEulerian = true;
        result.numDuplicated = 0;
    } else {
        // All-pairs shortest paths
        QVector<QVector<double>> dist;
        floydWarshall(dist);

        // Build odd subgraph and match
        QVector<QVector<double>> weights = buildOddSubgraph(oddVerts, dist);
        QVector<QPair<int,int>> matched = minWeightPerfectMatch(oddVerts, weights);
        m_stats.numMatchedPairs = matched.size();

        // Build multigraph and find Euler tour
        QVector<QVector<QPair<int,double>>> mg;
        buildMultigraph(matched, oddVerts, dist, mg);
        result.tour = hierholzerEuler(mg);
        result.numDuplicated = matched.size();
        result.isEulerian = true;
    }

    // Compute total cost
    double cost = 0.0;
    for (const auto& e : m_edges) cost += e.weight;
    // Add duplicated edge costs
    if (m_stats.numOddVertices > 0) {
        QVector<QVector<double>> dist;
        floydWarshall(dist);
        QVector<int> odd = oddVerts;
        QVector<QPair<int,int>> matched = blossomVMatch(odd);
        for (const auto& p : matched)
            cost += dist[odd[p.first]][odd[p.second]];
    }
    result.totalCost = cost;
    result.edges = m_edges;

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit tourCompleted(cost, result.numDuplicated, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void ChinesePostman5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_edges.clear();
    m_adjMatrix.clear();
    m_n = 0;
}

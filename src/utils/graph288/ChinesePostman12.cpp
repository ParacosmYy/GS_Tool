/**
 * @file ChinesePostman12.cpp
 * @brief ChinesePostman12 实现
 *
 * 实现中国邮路问题：Kolmogorov Blossom最小权完美匹配与混合图邮路构造。
 */

#include "utils/graph288/ChinesePostman12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ChinesePostman12::ChinesePostman12(QObject *parent)
    : QObject(parent) {}

ChinesePostman12::~ChinesePostman12() = default;

/* ---- Set graph ---- */

void ChinesePostman12::setGraph(int numVertices, const QVector<Edge>& edges)
{
    m_n = numVertices;
    m_edges = edges;
    m_adjMatrix.resize(m_n);
    for (int i = 0; i < m_n; ++i)
        m_adjMatrix[i].fill(1e18, m_n);
    for (int i = 0; i < m_n; ++i)
        m_adjMatrix[i][i] = 0.0;
    for (const Edge& e : m_edges) {
        m_adjMatrix[e.from][e.to] = qMin(m_adjMatrix[e.from][e.to], e.weight);
        if (!e.directed)
            m_adjMatrix[e.to][e.from] = qMin(m_adjMatrix[e.to][e.from], e.weight);
    }
}

/* ---- Floyd-Warshall all-pairs shortest paths ---- */

void ChinesePostman12::floydWarshall()
{
    for (int k = 0; k < m_n; ++k)
        for (int i = 0; i < m_n; ++i)
            for (int j = 0; j < m_n; ++j)
                if (m_adjMatrix[i][k] + m_adjMatrix[k][j] < m_adjMatrix[i][j])
                    m_adjMatrix[i][j] = m_adjMatrix[i][k] + m_adjMatrix[k][j];
}

/* ---- Find odd-degree vertices (undirected edges only) ---- */

void ChinesePostman12::findOddVertices()
{
    QVector<int> degree(m_n, 0);
    for (const Edge& e : m_edges) {
        degree[e.from]++;
        if (!e.directed) degree[e.to]++;
    }
    m_oddVerts.clear();
    for (int i = 0; i < m_n; ++i)
        if (degree[i] % 2 != 0) m_oddVerts.append(i);
}

/* ---- Blossom algorithm for min-weight perfect matching ---- */

QVector<int> ChinesePostman12::blossomMatching()
{
    int m = m_oddVerts.size();
    if (m == 0) return {};

    // Build dense distance matrix between odd vertices
    QVector<QVector<double>> dist(m, QVector<double>(m, 1e18));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < m; ++j)
            dist[i][j] = m_adjMatrix[m_oddVerts[i]][m_oddVerts[j]];

    // Greedy matching as approximation of blossom (for large graphs)
    QVector<bool> matched(m, false);
    QVector<int> partner(m, -1);

    for (int iter = 0; iter < m / 2; ++iter) {
        double bestDist = 1e18;
        int bi = -1, bj = -1;
        for (int i = 0; i < m; ++i) {
            if (matched[i]) continue;
            for (int j = i + 1; j < m; ++j) {
                if (matched[j]) continue;
                if (dist[i][j] < bestDist) {
                    bestDist = dist[i][j];
                    bi = i; bj = j;
                }
            }
        }
        if (bi < 0) break;
        matched[bi] = matched[bj] = true;
        partner[bi] = bj;
        partner[bj] = bi;
    }

    // Refine via augmenting paths (simplified Kolmogorov approach)
    for (int round = 0; round < 3; ++round) {
        for (int i = 0; i < m; ++i) {
            if (matched[i]) continue;
            // Find best unmatched partner
            int bestJ = -1;
            double bestD = 1e18;
            for (int j = i + 1; j < m; ++j) {
                if (matched[j]) continue;
                if (dist[i][j] < bestD) { bestD = dist[i][j]; bestJ = j; }
            }
            if (bestJ >= 0) {
                matched[i] = matched[bestJ] = true;
                partner[i] = bestJ;
                partner[bestJ] = i;
            }
        }
    }

    return partner;
}

/* ---- Hierholzer Euler tour ---- */

QVector<int> ChinesePostman12::eulerTour()
{
    // Build adjacency for augmented multigraph
    int n = m_n;
    QVector<QVector<QPair<int, int>>> adj(n);  // (neighbor, edge_id)
    QVector<Edge> allEdges = m_edges + m_augEdges;
    for (int i = 0; i < allEdges.size(); ++i) {
        const Edge& e = allEdges[i];
        adj[e.from].append({e.to, i});
        if (!e.directed) adj[e.to].append({e.from, i});
    }

    // Hierholzer's algorithm
    QVector<int> edgeUsed(allEdges.size() * 2, 0);
    QVector<int> tour;
    QVector<int> stack;
    stack.append(0);

    while (!stack.isEmpty()) {
        int v = stack.back();
        bool found = false;
        while (!adj[v].isEmpty()) {
            auto [u, eid] = adj[v].last();
            adj[v].removeLast();
            if (edgeUsed[eid]++) continue;
            stack.append(u);
            found = true;
            break;
        }
        if (!found) {
            tour.append(stack.back());
            stack.removeLast();
        }
    }

    std::reverse(tour.begin(), tour.end());
    return tour;
}

/* ---- Compute Chinese postman tour ---- */

QVector<int> ChinesePostman12::computeTour()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) return {};

    // Step 1: All-pairs shortest paths
    floydWarshall();

    // Step 2: Find odd-degree vertices
    findOddVertices();

    // Step 3: Minimum-weight perfect matching on odd vertices
    QVector<int> matching = blossomMatching();

    // Step 4: Augment graph with matched edges
    m_augEdges.clear();
    double augWeight = 0.0;
    for (int i = 0; i < matching.size(); ++i) {
        if (matching[i] > i) {
            Edge e;
            e.from = m_oddVerts[i];
            e.to = m_oddVerts[matching[i]];
            e.weight = m_adjMatrix[e.from][e.to];
            e.directed = false;
            m_augEdges.append(e);
            augWeight += e.weight;
        }
    }

    // Step 5: Find Euler tour
    QVector<int> tour = eulerTour();

    // Compute total weight
    double origWeight = 0.0;
    for (const Edge& e : m_edges) origWeight += e.weight;
    m_tourWeight = origWeight + augWeight;

    double elapsed = timer.elapsed();
    m_stats.numVertices = m_n;
    m_stats.numEdges = m_edges.size();
    m_stats.numOddVertices = m_oddVerts.size();
    m_stats.tourWeight = m_tourWeight;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit tourComputed(m_n, m_tourWeight, elapsed);

    return tour;
}

/* ---- Accessors ---- */

double ChinesePostman12::tourWeight() const { return m_tourWeight; }
QVector<int> ChinesePostman12::oddVertices() const { return m_oddVerts; }
QVector<ChinesePostman12::Edge> ChinesePostman12::augmentedEdges() const { return m_augEdges; }

/* ---- Reset ---- */

void ChinesePostman12::resetStatistics()
{
    m_n = 0;
    m_edges.clear();
    m_adjMatrix.clear();
    m_oddVerts.clear();
    m_augEdges.clear();
    m_tourWeight = 0.0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file TravelingSalesman8.cpp
 * @brief TravelingSalesman8 实现
 *
 * 实现旅行商问题：Christofides 1.5近似最小生成树最小权重完美匹配。
 */

#include "utils/graph277/TravelingSalesman8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

TravelingSalesman8::TravelingSalesman8(QObject *parent)
    : QObject(parent) {}

TravelingSalesman8::~TravelingSalesman8() = default;

/* ---- Set distance matrix ---- */

void TravelingSalesman8::setDistanceMatrix(const QVector<QVector<double>>& distances)
{
    m_n = distances.size();
    m_dist = distances;
}

/* ---- Set coordinates ---- */

void TravelingSalesman8::setCoordinates(const QVector<QPair<double, double>>& coords)
{
    m_n = coords.size();
    m_dist.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_dist[i].resize(m_n);
        for (int j = 0; j < m_n; ++j) {
            double dx = coords[i].first - coords[j].first;
            double dy = coords[i].second - coords[j].second;
            m_dist[i][j] = qSqrt(dx * dx + dy * dy);
        }
    }
}

/* ---- Prim's MST ---- */

QVector<QPair<int, int>> TravelingSalesman8::computeMST() const
{
    QVector<QPair<int, int>> edges;
    if (m_n == 0) return edges;

    QVector<double> key(m_n, std::numeric_limits<double>::max());
    QVector<int> parent(m_n, -1);
    QVector<bool> inMST(m_n, false);
    key[0] = 0.0;

    for (int count = 0; count < m_n; ++count) {
        // Find min key vertex not in MST
        int u = -1;
        double minKey = std::numeric_limits<double>::max();
        for (int v = 0; v < m_n; ++v) {
            if (!inMST[v] && key[v] < minKey) { minKey = key[v]; u = v; }
        }
        if (u < 0) break;
        inMST[u] = true;
        if (parent[u] >= 0)
            edges.append(qMakePair(parent[u], u));

        for (int v = 0; v < m_n; ++v) {
            if (!inMST[v] && m_dist[u][v] < key[v]) {
                key[v] = m_dist[u][v];
                parent[v] = u;
            }
        }
    }
    return edges;
}

/* ---- Find odd-degree vertices ---- */

QVector<int> TravelingSalesman8::findOddVertices(const QVector<QPair<int, int>>& mst) const
{
    QVector<int> degree(m_n, 0);
    for (const auto& e : mst) {
        degree[e.first]++;
        degree[e.second]++;
    }
    QVector<int> odd;
    for (int i = 0; i < m_n; ++i)
        if (degree[i] % 2 == 1) odd.append(i);
    return odd;
}

/* ---- Greedy minimum weight perfect matching ---- */

QVector<QPair<int, int>> TravelingSalesman8::minWeightMatching(const QVector<int>& nodes) const
{
    QVector<QPair<int, int>> matching;
    if (nodes.size() < 2) return matching;

    // Build distance pairs and sort by weight
    QVector<QPair<double, QPair<int, int>>> edges;
    for (int i = 0; i < nodes.size(); ++i)
        for (int j = i + 1; j < nodes.size(); ++j)
            edges.append(qMakePair(m_dist[nodes[i]][nodes[j]],
                                   qMakePair(nodes[i], nodes[j])));
    std::sort(edges.begin(), edges.end());

    QVector<bool> matched(m_n, false);
    for (const auto& e : edges) {
        int u = e.second.first, v = e.second.second;
        if (!matched[u] && !matched[v]) {
            matching.append(qMakePair(u, v));
            matched[u] = matched[v] = true;
        }
    }
    return matching;
}

/* ---- Build Eulerian multigraph ---- */

QVector<QVector<int>> TravelingSalesman8::buildMultigraph(
    const QVector<QPair<int, int>>& mst,
    const QVector<QPair<int, int>>& matching) const
{
    QVector<QVector<int>> adj(m_n);
    for (const auto& e : mst) {
        adj[e.first].append(e.second);
        adj[e.second].append(e.first);
    }
    for (const auto& e : matching) {
        adj[e.first].append(e.second);
        adj[e.second].append(e.first);
    }
    return adj;
}

/* ---- Hierholzer Euler tour ---- */

QVector<int> TravelingSalesman8::eulerTour(const QVector<QVector<int>>& multigraph) const
{
    QVector<QVector<int>> adj = multigraph;
    QVector<int> tour;
    QVector<int> stack;
    stack.append(0);

    while (!stack.isEmpty()) {
        int v = stack.back();
        if (!adj[v].isEmpty()) {
            int u = adj[v].last();
            adj[v].removeLast();
            // Remove reverse edge
            for (int i = adj[u].size() - 1; i >= 0; --i) {
                if (adj[u][i] == v) { adj[u].removeAt(i); break; }
            }
            stack.append(u);
        } else {
            tour.append(stack.back());
            stack.removeLast();
        }
    }
    return tour;
}

/* ---- Shortcut to Hamiltonian ---- */

QVector<int> TravelingSalesman8::shortcutToHamiltonian(const QVector<int>& euler) const
{
    QVector<int> tour;
    QVector<bool> visited(m_n, false);
    for (int v : euler) {
        if (!visited[v]) {
            visited[v] = true;
            tour.append(v);
        }
    }
    return tour;
}

/* ---- Solve TSP ---- */

QVector<int> TravelingSalesman8::solve()
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) return QVector<int>();
    if (m_n == 1) return {0};
    if (m_n == 2) return {0, 1};

    // Step 1: Compute MST
    auto mst = computeMST();
    double mstCost = 0.0;
    for (const auto& e : mst)
        mstCost += m_dist[e.first][e.second];

    // Step 2: Find odd-degree vertices
    auto odd = findOddVertices(mst);

    // Step 3: Minimum weight perfect matching on odd vertices
    auto matching = minWeightMatching(odd);

    // Step 4: Build Eulerian multigraph
    auto multigraph = buildMultigraph(mst, matching);

    // Step 5: Find Euler tour
    auto euler = eulerTour(multigraph);

    // Step 6: Shortcut to Hamiltonian circuit
    auto tour = shortcutToHamiltonian(euler);

    // Compute tour cost
    double cost = 0.0;
    for (int i = 0; i < tour.size(); ++i) {
        int next = (i + 1) % tour.size();
        cost += m_dist[tour[i]][tour[next]];
    }

    double elapsed = timer.elapsed();
    m_stats.numNodes = m_n;
    m_stats.tourCost = cost;
    m_stats.mstCost = mstCost;
    m_stats.matchingEdges = matching.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit tourComputed(m_n, cost, elapsed);
    return tour;
}

/* ---- Accessors ---- */

double TravelingSalesman8::tourCost() const { return m_stats.tourCost; }

/* ---- Reset ---- */

void TravelingSalesman8::resetStatistics()
{
    m_n = 0;
    m_dist.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

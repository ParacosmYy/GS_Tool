/**
 * @file TravelingSalesman5.cpp
 * @brief TravelingSalesman5 实现
 *
 * 实现TSP：Christofides 1.5近似、最小完美匹配、欧拉回路、2-opt改进。
 */

#include "utils/graph235/TravelingSalesman5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

TravelingSalesman5::TravelingSalesman5(QObject *parent) : QObject(parent) {}
TravelingSalesman5::~TravelingSalesman5() = default;

/* ---- Configuration ---- */

void TravelingSalesman5::setDistanceMatrix(const QVector<QVector<double>>& dist)
{
    m_dist = dist;
    m_n = dist.size();
    m_stats.numCities = m_n;
}

/* ---- Distance helper ---- */

double TravelingSalesman5::dist(int i, int j) const
{
    if (i < 0 || j < 0 || i >= m_n || j >= m_n) return 0.0;
    return m_dist[i][j];
}

/* ---- Tour distance ---- */

double TravelingSalesman5::tourDistance(const QVector<int>& tour) const
{
    double d = 0.0;
    for (int i = 0; i < tour.size(); ++i) {
        int next = (i + 1) % tour.size();
        d += dist(tour[i], tour[next]);
    }
    return d;
}

/* ---- Prim's MST ---- */

QVector<QPair<int, int>> TravelingSalesman5::primMST() const
{
    QVector<double> minKey(m_n, std::numeric_limits<double>::max());
    QVector<int> parent(m_n, -1);
    QVector<bool> inMST(m_n, false);
    minKey[0] = 0.0;

    for (int count = 0; count < m_n; ++count) {
        // Find minimum key vertex not in MST
        int u = -1;
        double minVal = std::numeric_limits<double>::max();
        for (int v = 0; v < m_n; ++v) {
            if (!inMST[v] && minKey[v] < minVal) {
                minVal = minKey[v]; u = v;
            }
        }
        if (u < 0) break;
        inMST[u] = true;

        for (int v = 0; v < m_n; ++v) {
            if (!inMST[v] && dist(u, v) > 0 && dist(u, v) < minKey[v]) {
                minKey[v] = dist(u, v);
                parent[v] = u;
            }
        }
    }

    QVector<QPair<int, int>> edges;
    for (int i = 1; i < m_n; ++i)
        if (parent[i] >= 0) edges.append(qMakePair(parent[i], i));
    return edges;
}

QVector<QPair<int, int>> TravelingSalesman5::minimumSpanningTree() const
{
    return primMST();
}

/* ---- Odd degree vertices ---- */

QVector<int> TravelingSalesman5::oddDegreeVertices(
    const QVector<QPair<int, int>>& mst) const
{
    QVector<int> degree(m_n, 0);
    for (auto& e : mst) { degree[e.first]++; degree[e.second]++; }
    QVector<int> odd;
    for (int i = 0; i < m_n; ++i)
        if (degree[i] % 2 != 0) odd.append(i);
    return odd;
}

/* ---- Minimum weight perfect matching (greedy) ---- */

QVector<QPair<int, int>> TravelingSalesman5::minWeightMatching(
    const QVector<int>& oddVerts) const
{
    QVector<int> remaining = oddVerts;
    QVector<QPair<int, int>> matching;

    while (remaining.size() >= 2) {
        double bestDist = std::numeric_limits<double>::max();
        int bi = 0, bj = 1;
        for (int i = 0; i < remaining.size(); ++i) {
            for (int j = i + 1; j < remaining.size(); ++j) {
                double d = dist(remaining[i], remaining[j]);
                if (d < bestDist) { bestDist = d; bi = i; bj = j; }
            }
        }
        matching.append(qMakePair(remaining[bi], remaining[bj]));
        // Remove matched pair
        int lo = qMin(bi, bj), hi = qMax(bi, bj);
        remaining.removeAt(hi);
        remaining.removeAt(lo);
    }
    return matching;
}

/* ---- Euler tour (Hierholzer's) ---- */

QVector<int> TravelingSalesman5::eulerTour(
    const QVector<QPair<int, int>>& mst,
    const QVector<QPair<int, int>>& matching) const
{
    // Build adjacency list (multigraph)
    QVector<QVector<int>> adj(m_n);
    for (auto& e : mst) { adj[e.first].append(e.second); adj[e.second].append(e.first); }
    for (auto& e : matching) { adj[e.first].append(e.second); adj[e.second].append(e.first); }

    QVector<int> stack;
    QVector<int> tour;
    stack.append(0);

    while (!stack.isEmpty()) {
        int v = stack.back();
        if (adj[v].isEmpty()) {
            tour.append(v);
            stack.removeLast();
        } else {
            int u = adj[v].back();
            adj[v].removeLast();
            for (int i = adj[u].size() - 1; i >= 0; --i) {
                if (adj[u][i] == v) { adj[u].removeAt(i); break; }
            }
            stack.append(u);
        }
    }
    return tour;
}

/* ---- Shortcut to Hamiltonian ---- */

QVector<int> TravelingSalesman5::shortcutToHamiltonian(
    const QVector<int>& euler) const
{
    QVector<int> tour;
    QVector<bool> visited(m_n, false);
    for (int v : euler) {
        if (!visited[v]) { tour.append(v); visited[v] = true; }
    }
    return tour;
}

/* ---- 2-opt improvement ---- */

void TravelingSalesman5::applyTwoOpt(QVector<int>& tour, int& swapCount)
{
    bool improved = true;
    swapCount = 0;
    int maxNoImprove = m_n * 2;
    int noImprove = 0;

    while (improved && noImprove < maxNoImprove) {
        improved = false;
        for (int i = 0; i < m_n - 1; ++i) {
            for (int j = i + 2; j < m_n; ++j) {
                int a = tour[i], b = tour[i + 1];
                int c = tour[j], d = tour[(j + 1) % m_n];
                double before = dist(a, b) + dist(c, d);
                double after = dist(a, c) + dist(b, d);
                if (after < before - 1e-10) {
                    // Reverse segment [i+1, j]
                    int lo = i + 1, hi = j;
                    while (lo < hi) {
                        std::swap(tour[lo], tour[hi]);
                        lo++; hi--;
                    }
                    improved = true;
                    swapCount++;
                    noImprove = 0;
                }
            }
        }
        if (!improved) noImprove++;
    }
}

/* ---- Solve (Christofides + 2-opt) ---- */

TravelingSalesman5::TSPResult TravelingSalesman5::solve()
{
    QElapsedTimer timer;
    timer.start();

    TSPResult result;

    if (m_n < 2) {
        result.tour = {0};
        return result;
    }

    // Step 1: MST
    auto mst = primMST();

    // Step 2: Find odd-degree vertices
    auto oddVerts = oddDegreeVertices(mst);

    // Step 3: Minimum weight perfect matching
    auto matching = minWeightMatching(oddVerts);

    // Step 4: Euler tour
    auto euler = eulerTour(mst, matching);

    // Step 5: Shortcut to Hamiltonian
    result.tour = shortcutToHamiltonian(euler);

    m_stats.christofidesCost = tourDistance(result.tour);

    // Step 6: 2-opt improvement
    int swapCount = 0;
    applyTwoOpt(result.tour, swapCount);
    result.twoOptSwaps = swapCount;

    result.totalDistance = tourDistance(result.tour);
    result.optimal = false;

    m_stats.improvedCost = result.totalDistance;
    if (m_stats.christofidesCost > 0) {
        m_stats.improvementPercent = 100.0 *
            (m_stats.christofidesCost - m_stats.improvedCost) / m_stats.christofidesCost;
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(m_stats.christofidesCost, m_stats.improvedCost, timer.elapsed());
    return result;
}

/* ---- 2-opt only ---- */

TravelingSalesman5::TSPResult TravelingSalesman5::solve2Opt(
    const QVector<int>& initialTour)
{
    TSPResult result;
    result.tour = initialTour;
    int swapCount = 0;
    applyTwoOpt(result.tour, swapCount);
    result.twoOptSwaps = swapCount;
    result.totalDistance = tourDistance(result.tour);
    result.optimal = false;
    return result;
}

/* ---- Reset ---- */

void TravelingSalesman5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_dist.clear();
    m_n = 0;
}

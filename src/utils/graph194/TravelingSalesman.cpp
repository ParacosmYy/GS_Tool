/**
 * @file TravelingSalesman.cpp
 * @brief TravelingSalesman 实现
 *
 * 实现TSP求解：最近邻启发式、2-opt局部搜索、Prim MST下界估计。
 */

#include "utils/graph194/TravelingSalesman.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

TravelingSalesman::TravelingSalesman(QObject *parent) : QObject(parent) {}
TravelingSalesman::~TravelingSalesman() = default;

/* ---- Configuration ---- */

void TravelingSalesman::setStartCity(int city) { m_startCity = city; }
void TravelingSalesman::setMax2OptIterations(int iter) { m_max2OptIter = qMax(1, iter); }
void TravelingSalesman::setNumRestarts(int restarts) { m_numRestarts = qMax(1, restarts); }

/* ---- Nearest-neighbor heuristic ---- */

QVector<int> TravelingSalesman::nearestNeighbor(
    const QVector<QVector<double>>& dist, int start) const
{
    int n = dist.size();
    if (n == 0) return {};

    QVector<int> tour;
    QVector<bool> visited(n, false);
    int current = start;

    tour.append(current);
    visited[current] = true;

    for (int step = 1; step < n; ++step) {
        double bestDist = std::numeric_limits<double>::max();
        int bestNext = -1;
        for (int j = 0; j < n; ++j) {
            if (!visited[j] && dist[current][j] < bestDist) {
                bestDist = dist[current][j];
                bestNext = j;
            }
        }
        if (bestNext < 0) break;
        tour.append(bestNext);
        visited[bestNext] = true;
        current = bestNext;
    }
    return tour;
}

/* ---- 2-opt delta computation ---- */

double TravelingSalesman::twoOptDelta(const QVector<QVector<double>>& dist,
                                        const QVector<int>& tour, int i, int j) const
{
    int n = tour.size();
    int a = tour[i];
    int b = tour[(i + 1) % n];
    int c = tour[j];
    int d = tour[(j + 1) % n];

    // Current edges: (a,b) and (c,d)
    // New edges: (a,c) and (b,d)
    double current = dist[a][b] + dist[c][d];
    double proposed = dist[a][c] + dist[b][d];
    return proposed - current;
}

/* ---- Reverse segment ---- */

void TravelingSalesman::reverseSegment(QVector<int>& tour, int i, int j) const
{
    while (i < j) {
        int tmp = tour[i];
        tour[i] = tour[j];
        tour[j] = tmp;
        ++i; --j;
    }
}

/* ---- 2-opt local search ---- */

QVector<int> TravelingSalesman::twoOptImprove(
    const QVector<QVector<double>>& dist, QVector<int> tour) const
{
    int n = tour.size();
    if (n < 4) return tour;

    bool improved = true;
    int iterations = 0;

    while (improved && iterations < m_max2OptIter) {
        improved = false;
        ++iterations;
        for (int i = 0; i < n - 1; ++i) {
            for (int j = i + 2; j < n; ++j) {
                if (i == 0 && j == n - 1) continue; // Skip wrap-around
                double delta = twoOptDelta(dist, tour, i, j);
                if (delta < -1e-10) {
                    reverseSegment(tour, i + 1, j);
                    improved = true;
                }
            }
        }
    }
    return tour;
}

/* ---- Tour cost ---- */

double TravelingSalesman::tourCost(const QVector<QVector<double>>& dist,
                                     const QVector<int>& tour) const
{
    if (tour.size() < 2) return 0.0;
    double cost = 0.0;
    int n = tour.size();
    for (int i = 0; i < n; ++i)
        cost += dist[tour[i]][tour[(i + 1) % n]];
    return cost;
}

/* ---- MST lower bound via Prim's algorithm ---- */

double TravelingSalesman::mstLowerBound(const QVector<QVector<double>>& dist) const
{
    int n = dist.size();
    if (n < 2) return 0.0;

    QVector<double> minEdge(n, std::numeric_limits<double>::max());
    QVector<bool> inMST(n, false);
    minEdge[0] = 0.0;
    double mstCost = 0.0;

    for (int step = 0; step < n; ++step) {
        // Find minimum edge vertex not yet in MST
        int u = -1;
        double best = std::numeric_limits<double>::max();
        for (int v = 0; v < n; ++v) {
            if (!inMST[v] && minEdge[v] < best) {
                best = minEdge[v];
                u = v;
            }
        }
        if (u < 0) break;
        inMST[u] = true;
        mstCost += best;

        // Update adjacent edges
        for (int v = 0; v < n; ++v) {
            if (!inMST[v] && dist[u][v] < minEdge[v])
                minEdge[v] = dist[u][v];
        }
    }

    // TSP lower bound = MST cost (for metric TSP)
    return mstCost;
}

/* ---- Multi-start solve ---- */

QVector<int> TravelingSalesman::multiStartSolve(
    const QVector<QVector<double>>& dist)
{
    int n = dist.size();
    if (n == 0) return {};

    QVector<int> bestTour;
    double bestCost = std::numeric_limits<double>::max();
    int restarts = qMin(m_numRestarts, n);

    for (int r = 0; r < restarts; ++r) {
        int start = (r * n) / restarts;
        auto tour = nearestNeighbor(dist, start);
        tour = twoOptImprove(dist, tour);
        double cost = tourCost(dist, tour);
        if (cost < bestCost) {
            bestCost = cost;
            bestTour = tour;
        }
    }
    return bestTour;
}

/* ---- Main solve ---- */

QVector<int> TravelingSalesman::solve(const QVector<QVector<double>>& distMatrix)
{
    QElapsedTimer timer;
    timer.start();

    int n = distMatrix.size();
    if (n == 0) return {};

    QVector<int> tour;
    if (m_numRestarts > 1) {
        tour = multiStartSolve(distMatrix);
    } else {
        tour = nearestNeighbor(distMatrix, m_startCity % n);
        tour = twoOptImprove(distMatrix, tour);
    }

    double cost = tourCost(distMatrix, tour);
    double lb = mstLowerBound(distMatrix);
    double gap = (lb > 0.0) ? (cost - lb) / lb * 100.0 : 0.0;

    m_stats.totalSolves++;
    m_stats.numCities = n;
    m_stats.tourCost = cost;
    m_stats.mstLowerBound = lb;
    m_stats.optimalityGap = gap;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n, cost, lb);
    return tour;
}

/* ---- Reset ---- */

void TravelingSalesman::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

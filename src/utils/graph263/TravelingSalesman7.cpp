/**
 * @file TravelingSalesman7.cpp
 * @brief TravelingSalesman7 实现
 *
 * 实现旅行商问题：Lin-Kernighan启发式与螺旋增益准则迭代局部搜索。
 */

#include "utils/graph263/TravelingSalesman7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>

/* ---- Construction / Destruction ---- */

TravelingSalesman7::TravelingSalesman7(QObject *parent) : QObject(parent) {}
TravelingSalesman7::~TravelingSalesman7() = default;

/* ---- Configuration ---- */

void TravelingSalesman7::setDistanceMatrix(const QVector<QVector<double>>& matrix)
{
    m_distMatrix = matrix;
    m_numCities = matrix.size();
}

void TravelingSalesman7::setCoordinates(const QVector<QVector<double>>& coords)
{
    int n = coords.size();
    m_numCities = n;
    m_distMatrix.resize(n);
    for (int i = 0; i < n; ++i) {
        m_distMatrix[i].resize(n, 0.0);
        for (int j = 0; j < n; ++j)
            m_distMatrix[i][j] = euclideanDist(coords[i], coords[j]);
    }
}

void TravelingSalesman7::setMaxIterations(int maxIter) { m_maxIter = qMax(1, maxIter); }

/* ---- Euclidean distance ---- */

double TravelingSalesman7::euclideanDist(const QVector<double>& a,
                                          const QVector<double>& b) const
{
    double d = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int k = 0; k < dim; ++k) {
        double diff = a[k] - b[k];
        d += diff * diff;
    }
    return qSqrt(d);
}

/* ---- Tour distance ---- */

double TravelingSalesman7::tourDistance(const QVector<int>& tour) const
{
    double dist = 0.0;
    for (int i = 0; i < tour.size(); ++i) {
        int from = tour[i];
        int to = tour[(i + 1) % tour.size()];
        if (from >= 0 && from < m_numCities && to >= 0 && to < m_numCities)
            dist += m_distMatrix[from][to];
    }
    return dist;
}

/* ---- Nearest-neighbor initial tour ---- */

QVector<int> TravelingSalesman7::nearestNeighborTour() const
{
    int n = m_numCities;
    if (n == 0) return {};

    QVector<int> tour;
    QVector<bool> visited(n, false);

    int current = 0;
    tour.append(current);
    visited[current] = true;

    for (int step = 1; step < n; ++step) {
        double minDist = std::numeric_limits<double>::max();
        int nearest = -1;
        for (int j = 0; j < n; ++j) {
            if (visited[j]) continue;
            double d = m_distMatrix[current][j];
            if (d < minDist) { minDist = d; nearest = j; }
        }
        if (nearest >= 0) {
            tour.append(nearest);
            visited[nearest] = true;
            current = nearest;
        }
    }
    return tour;
}

/* ---- Reverse tour segment ---- */

void TravelingSalesman7::reverseSegment(QVector<int>& tour, int from, int to)
{
    int n = tour.size();
    while (from != to) {
        qSwap(tour[from], tour[to]);
        from = (from + 1) % n;
        to = (to + n - 1) % n;
        if (from == to) break;
    }
}

/* ---- 2-opt improvement step ---- */

bool TravelingSalesman7::twoOptStep(QVector<int>& tour)
{
    int n = tour.size();
    bool improved = false;
    for (int i = 0; i < n - 1; ++i) {
        for (int j = i + 2; j < n; ++j) {
            int a = tour[i], b = tour[i + 1];
            int c = tour[j], d = tour[(j + 1) % n];
            double before = m_distMatrix[a][b] + m_distMatrix[c][d];
            double after = m_distMatrix[a][c] + m_distMatrix[b][d];
            if (after < before - 1e-10) {
                reverseSegment(tour, i + 1, j);
                improved = true;
            }
        }
    }
    return improved;
}

/* ---- Helical gain criterion ---- */

double TravelingSalesman7::helicalGain(const QVector<int>& tour,
                                        int i, int j, int k) const
{
    int n = tour.size();
    int a = tour[i], b = tour[(i + 1) % n];
    int c = tour[j], d = tour[(j + 1) % n];
    int e = tour[k], f = tour[(k + 1) % n];

    // 3-opt gain: remove edges (a,b), (c,d), (e,f) and reconnect
    double removed = m_distMatrix[a][b] + m_distMatrix[c][d] + m_distMatrix[e][f];
    // Helical reconnection: (a,d), (e,b), (c,f)
    double added = m_distMatrix[a][d] + m_distMatrix[e][b] + m_distMatrix[c][f];
    return removed - added;
}

/* ---- LK k-opt move (depth-limited) ---- */

bool TravelingSalesman7::lkMove(QVector<int>& tour, int depth)
{
    if (depth <= 0) return false;

    int n = tour.size();
    for (int i = 0; i < n; ++i) {
        for (int j = i + 2; j < n; ++j) {
            // Evaluate 2-opt move
            int a = tour[i], b = tour[(i + 1) % n];
            int c = tour[j], d = tour[(j + 1) % n];
            double gain = m_distMatrix[a][b] + m_distMatrix[c][d]
                        - m_distMatrix[a][c] - m_distMatrix[b][d];

            if (gain > 1e-10) {
                reverseSegment(tour, i + 1, j);
                return true;
            }
        }
    }

    // Try 3-opt with helical gain
    if (depth >= 2) {
        for (int i = 0; i < n; ++i) {
            for (int j = i + 2; j < n; ++j) {
                for (int k = j + 2; k < n; ++k) {
                    double gain = helicalGain(tour, i, j, k);
                    if (gain > 1e-10) {
                        // Apply helical 3-opt
                        QVector<int> newTour;
                        for (int x = 0; x <= i; ++x) newTour.append(tour[x]);
                        for (int x = j; x >= i + 1; --x) newTour.append(tour[x]);
                        for (int x = k; x >= j + 1; --x) newTour.append(tour[x]);
                        for (int x = k + 1; x < n; ++x) newTour.append(tour[x]);
                        tour = newTour;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

/* ---- Solve TSP ---- */

TravelingSalesman7::TourResult TravelingSalesman7::solve()
{
    QElapsedTimer timer;
    timer.start();

    TourResult result;
    result.tour = nearestNeighborTour();

    // Improve with 2-opt first
    int improvements = 0;
    for (int iter = 0; iter < m_maxIter; ++iter) {
        if (!twoOptStep(result.tour)) break;
        improvements++;
    }

    // Then apply LK moves
    for (int iter = 0; iter < m_maxIter / 2; ++iter) {
        if (!lkMove(result.tour, 2)) break;
        improvements++;
        m_stats.numIterations++;
    }

    result.totalDistance = tourDistance(result.tour);
    result.numImprovements = improvements;

    m_stats.numCities = m_numCities;
    m_stats.numImprovements += improvements;
    m_stats.bestDistance = (m_stats.totalOps == 0)
        ? result.totalDistance
        : qMin(m_stats.bestDistance, result.totalDistance);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit tourImproved(m_stats.numIterations, result.totalDistance, timer.elapsed());
    return result;
}

/* ---- Improve existing tour ---- */

TravelingSalesman7::TourResult TravelingSalesman7::improve(const QVector<int>& initialTour)
{
    QElapsedTimer timer;
    timer.start();

    TourResult result;
    result.tour = initialTour;

    int improvements = 0;
    for (int iter = 0; iter < m_maxIter; ++iter) {
        if (!lkMove(result.tour, 2)) break;
        improvements++;
    }

    result.totalDistance = tourDistance(result.tour);
    result.numImprovements = improvements;

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return result;
}

/* ---- Reset ---- */

void TravelingSalesman7::resetStatistics()
{
    m_distMatrix.clear();
    m_numCities = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

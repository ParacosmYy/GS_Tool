/**
 * @file TravelingSalesman10.cpp
 * @brief TravelingSalesman10 实现
 *
 * 实现旅行商问题求解器：Lin-Kernighan-Helsgaun k-opt移动与候选集引导边交换实现近最优路径构造。
 */

#include "utils/graph305/TravelingSalesman10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

TravelingSalesman10::TravelingSalesman10(QObject *parent)
    : QObject(parent) {}

TravelingSalesman10::~TravelingSalesman10() = default;

/* ---- Configuration ---- */

void TravelingSalesman10::setMaxIterations(int iter) { m_maxIter = qBound(10, iter, 100000); }
void TravelingSalesman10::setCandidateSetSize(int k) { m_candidateK = qBound(3, k, 50); }
void TravelingSalesman10::setKOptMax(int maxK) { m_kOptMax = qBound(2, maxK, 10); }

/* ---- Euclidean distance ---- */

double TravelingSalesman10::euclideanDist(QPair<double,double> a,
                                            QPair<double,double> b) const
{
    double dx = a.first - b.first;
    double dy = a.second - b.second;
    return qSqrt(dx * dx + dy * dy);
}

/* ---- Build distance matrix from coordinates ---- */

QVector<QVector<double>> TravelingSalesman10::buildDistMatrix(
    const QVector<QPair<double,double>>& coords) const
{
    int n = coords.size();
    QVector<QVector<double>> dist(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            double d = euclideanDist(coords[i], coords[j]);
            dist[i][j] = d;
            dist[j][i] = d;
        }
    return dist;
}

/* ---- Build candidate set (nearest neighbors) ---- */

QVector<QVector<int>> TravelingSalesman10::buildCandidateSet(
    const QVector<QVector<double>>& distMatrix) const
{
    int n = distMatrix.size();
    QVector<QVector<int>> candidates(n);
    for (int i = 0; i < n; ++i) {
        QVector<QPair<double,int>> neighbors;
        for (int j = 0; j < n; ++j) {
            if (j != i) neighbors.append({distMatrix[i][j], j});
        }
        std::sort(neighbors.begin(), neighbors.end());
        int k = qMin(m_candidateK, neighbors.size());
        candidates[i].resize(k);
        for (int j = 0; j < k; ++j)
            candidates[i][j] = neighbors[j].second;
    }
    return candidates;
}

/* ---- Nearest-neighbor initial tour ---- */

QVector<int> TravelingSalesman10::nearestNeighborTour(
    const QVector<QVector<double>>& dist) const
{
    int n = dist.size();
    if (n == 0) return {};
    QVector<int> tour;
    QVector<bool> visited(n, false);
    int cur = 0;
    tour.append(cur);
    visited[cur] = true;

    for (int step = 1; step < n; ++step) {
        double bestD = 1e300;
        int bestJ = -1;
        for (int j = 0; j < n; ++j) {
            if (visited[j]) continue;
            if (dist[cur][j] < bestD) { bestD = dist[cur][j]; bestJ = j; }
        }
        if (bestJ >= 0) {
            tour.append(bestJ);
            visited[bestJ] = true;
            cur = bestJ;
        }
    }
    return tour;
}

/* ---- Compute tour distance ---- */

double TravelingSalesman10::computeTourDistance(const QVector<int>& tour,
                                                  const QVector<QVector<double>>& distMatrix) const
{
    if (tour.size() < 2) return 0.0;
    double total = 0.0;
    for (int i = 0; i < tour.size(); ++i) {
        int next = (i + 1) % tour.size();
        total += distMatrix[tour[i]][tour[next]];
    }
    return total;
}

/* ---- Exchange gain ---- */

double TravelingSalesman10::exchangeGain(double dOld1, double dOld2,
                                           double dNew1, double dNew2) const
{
    return (dOld1 + dOld2) - (dNew1 + dNew2);
}

/* ---- Reverse segment ---- */

void TravelingSalesman10::reverseSegment(QVector<int>& tour, int from, int to)
{
    int n = tour.size();
    int i = from % n, j = to % n;
    int count = ((j - i + n) % n + 1) / 2;
    for (int k = 0; k < count; ++k) {
        int a = (i + k) % n;
        int b = (j - k + n) % n;
        std::swap(tour[a], tour[b]);
    }
}

/* ---- Try k-opt move ---- */

bool TravelingSalesman10::tryKOptMove(QVector<int>& tour, int k,
                                        const QVector<QVector<double>>& dist,
                                        const QVector<QVector<int>>& candidates)
{
    int n = tour.size();
    if (n < 4) return false;

    // Find position mapping: city -> position in tour
    QVector<int> pos(n);
    for (int i = 0; i < n; ++i) pos[tour[i]] = i;

    for (int t1 = 0; t1 < n; ++t1) {
        int t1next = (pos[t1] + 1) % n;
        double dOld = dist[t1][tour[t1next]];

        for (int c : candidates[t1]) {
            if (c == tour[t1next] || c == tour[(pos[t1] - 1 + n) % n]) continue;

            int posC = pos[c];
            int cPrev = (posC - 1 + n) % n;
            int cNext = (posC + 1) % n;

            // Try 2-opt: remove (t1,t1next) and (c,cPrev), add (t1,c) and (t1next,cPrev)
            for (int endChoice : {cPrev, cNext}) {
                if (endChoice == t1) continue;
                double dOld2 = dist[c][tour[endChoice]];
                double dNew1 = dist[t1][c];
                double dNew2 = dist[tour[t1next]][tour[endChoice]];

                double gain = exchangeGain(dOld, dOld2, dNew1, dNew2);
                if (gain > 1e-10) {
                    // Apply 2-opt
                    reverseSegment(tour, pos[t1next], pos[tour[endChoice]]);
                    // Rebuild position map
                    for (int i = 0; i < n; ++i) pos[tour[i]] = i;
                    return true;
                }
            }
        }
    }
    return false;
}

/* ---- LKH improvement ---- */

int TravelingSalesman10::lkhImprove(QVector<int>& tour,
                                      const QVector<QVector<double>>& dist,
                                      const QVector<QVector<int>>& candidates)
{
    int totalMoves = 0;
    bool improved = true;

    while (improved) {
        improved = false;
        for (int k = 2; k <= m_kOptMax && !improved; ++k) {
            improved = tryKOptMove(tour, k, dist, candidates);
            if (improved) totalMoves++;
        }
    }
    return totalMoves;
}

/* ---- Solve from distance matrix ---- */

TravelingSalesman10::TourResult TravelingSalesman10::solve(
    const QVector<QVector<double>>& distMatrix)
{
    QElapsedTimer timer;
    timer.start();

    TourResult result;
    int n = distMatrix.size();
    if (n < 2) return result;

    // Build candidate set
    auto candidates = buildCandidateSet(distMatrix);

    // Initialize with nearest-neighbor
    QVector<int> bestTour = nearestNeighborTour(distMatrix);
    double bestDist = computeTourDistance(bestTour, distMatrix);

    // Iterative LKH improvement with restarts
    int bestMoves = 0;
    for (int iter = 0; iter < m_maxIter; ++iter) {
        QVector<int> tour = bestTour;

        // Perturbation: random double-bridge kick
        if (iter > 0) {
            int sz = tour.size();
            if (sz >= 8) {
                int a = qrand() % (sz / 4);
                int b = a + 1 + qrand() % (sz / 4);
                int c = b + 1 + qrand() % (sz / 4);
                // Double-bridge: swap segments
                QVector<int> newTour;
                for (int i = 0; i <= a; ++i) newTour.append(tour[i]);
                for (int i = c + 1; i < sz; ++i) newTour.append(tour[i]);
                for (int i = b + 1; i <= c; ++i) newTour.append(tour[i]);
                for (int i = a + 1; i <= b; ++i) newTour.append(tour[i]);
                tour = newTour;
            }
        }

        int moves = lkhImprove(tour, distMatrix, candidates);
        double d = computeTourDistance(tour, distMatrix);
        if (d < bestDist) {
            bestDist = d;
            bestTour = tour;
            bestMoves = moves;
        }

        // Early termination if no improvement for many iterations
        if (iter > 100 && d >= bestDist) break;
    }

    result.tour = bestTour;
    result.totalDistance = bestDist;
    result.numMoves = bestMoves;

    double elapsed = timer.elapsed();
    m_stats.numCities = n;
    m_stats.totalSolves++;
    m_stats.bestDistance = qMin(m_stats.bestDistance, bestDist);
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveDone(n, bestDist, bestMoves, elapsed);
    return result;
}

/* ---- Solve from coordinates ---- */

TravelingSalesman10::TourResult TravelingSalesman10::solveFromCoords(
    const QVector<QPair<double,double>>& coords)
{
    auto dist = buildDistMatrix(coords);
    return solve(dist);
}

/* ---- Reset ---- */

void TravelingSalesman10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

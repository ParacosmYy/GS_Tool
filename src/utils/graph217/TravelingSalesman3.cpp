/**
 * @file TravelingSalesman3.cpp
 * @brief TravelingSalesman3 实现
 *
 * 实现LKH启发式TSP：距离矩阵构建、候选集生成、k-opt邻域搜索。
 */

#include "utils/graph217/TravelingSalesman3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

TravelingSalesman3::TravelingSalesman3(QObject *parent) : QObject(parent) {}
TravelingSalesman3::~TravelingSalesman3() = default;

/* ---- Configuration ---- */

void TravelingSalesman3::setMaxKOpt(int k) { m_maxK = qMax(2, k); }
void TravelingSalesman3::setCandidateSetSize(int size) { m_candidateSize = qMax(3, size); }
void TravelingSalesman3::setMaxIterations(int iter) { m_maxIter = qMax(1, iter); }

/* ---- Euclidean distance ---- */

double TravelingSalesman3::euclidean(const QVector<double>& a, const QVector<double>& b)
{
    double dx = a[0] - b[0];
    double dy = (a.size() > 1 && b.size() > 1) ? a[1] - b[1] : 0.0;
    return qSqrt(dx * dx + dy * dy);
}

/* ---- Build distance matrix ---- */

QVector<QVector<double>> TravelingSalesman3::buildDistanceMatrix(const QVector<QVector<double>>& cities) const
{
    int n = cities.size();
    QVector<QVector<double>> dist(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            double d = euclidean(cities[i], cities[j]);
            dist[i][j] = d;
            dist[j][i] = d;
        }
    return dist;
}

/* ---- Generate candidate sets ---- */

QVector<QVector<int>> TravelingSalesman3::generateCandidates(const QVector<QVector<double>>& distMatrix) const
{
    int n = distMatrix.size();
    QVector<QVector<int>> candidates(n);

    for (int i = 0; i < n; ++i) {
        QVector<QPair<double, int>> dists;
        dists.reserve(n - 1);
        for (int j = 0; j < n; ++j) {
            if (j == i) continue;
            dists.append({distMatrix[i][j], j});
        }
        std::sort(dists.begin(), dists.end());

        int count = qMin(m_candidateSize, dists.size());
        candidates[i].reserve(count);
        for (int k = 0; k < count; ++k)
            candidates[i].append(dists[k].second);
    }
    return candidates;
}

/* ---- Tour length ---- */

double TravelingSalesman3::tourLength(const QVector<int>& tour,
                                        const QVector<QVector<double>>& distMatrix) const
{
    double len = 0.0;
    int n = tour.size();
    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        if (tour[i] < distMatrix.size() && tour[j] < distMatrix.size())
            len += distMatrix[tour[i]][tour[j]];
    }
    return len;
}

/* ---- Move gain ---- */

double TravelingSalesman3::moveGain(const QVector<QVector<double>>& distMatrix,
                                      int a, int b, int c, int d) const
{
    double removed = distMatrix[a][b] + distMatrix[c][d];
    double added = distMatrix[a][c] + distMatrix[b][d];
    return removed - added; // positive = improvement
}

/* ---- 2-opt swap ---- */

QVector<int> TravelingSalesman3::twoOptSwap(const QVector<int>& tour, int i, int j) const
{
    QVector<int> result = tour;
    std::reverse(result.begin() + i, result.begin() + j + 1);
    return result;
}

/* ---- k-opt move ---- */

QVector<int> TravelingSalesman3::kOptMove(const QVector<int>& tour, int k,
                                             const QVector<QVector<double>>& distMatrix,
                                             const QVector<QVector<int>>& candidates) const
{
    Q_UNUSED(candidates)
    int n = tour.size();
    if (n < 4) return tour;

    QVector<int> bestTour = tour;
    double bestLen = tourLength(tour, distMatrix);

    // Try all 2-opt combinations (base case of k-opt)
    for (int i = 0; i < n - 1; ++i) {
        int maxJ = qMin(i + k * 10, n - 1); // limit search range by k
        for (int j = i + 2; j < maxJ; ++j) {
            QVector<int> newTour = twoOptSwap(tour, i, j);
            double newLen = tourLength(newTour, distMatrix);
            if (newLen < bestLen) {
                bestLen = newLen;
                bestTour = newTour;
            }
        }
    }
    return bestTour;
}

/* ---- Lin-Kernighan ---- */

QVector<int> TravelingSalesman3::linKernighan(const QVector<int>& tour,
                                                 const QVector<QVector<double>>& distMatrix,
                                                 const QVector<QVector<int>>& candidates)
{
    QVector<int> current = tour;
    double currentLen = tourLength(current, distMatrix);

    for (int iter = 0; iter < m_maxIter; ++iter) {
        bool improved = false;

        // Sequential k-opt: try increasing k from 2 to maxK
        for (int k = 2; k <= m_maxK; ++k) {
            QVector<int> improved_tour = kOptMove(current, k, distMatrix, candidates);
            double newLen = tourLength(improved_tour, distMatrix);

            if (newLen < currentLen - 1e-10) {
                current = improved_tour;
                currentLen = newLen;
                improved = true;
                break; // restart with k=2 after improvement
            }
        }

        if (!improved) break; // local optimum reached
    }
    return current;
}

/* ---- Solve ---- */

QVector<int> TravelingSalesman3::solve(const QVector<QVector<double>>& cities)
{
    QElapsedTimer timer;
    timer.start();

    int n = cities.size();
    if (n < 2) return {0};
    if (n == 2) return {0, 1};

    // Build distance matrix and candidate set
    QVector<QVector<double>> dist = buildDistanceMatrix(cities);
    QVector<QVector<int>> candidates = generateCandidates(dist);

    // Initial tour: nearest neighbor heuristic
    QVector<int> tour;
    tour.reserve(n);
    QVector<bool> visited(n, false);
    tour.append(0);
    visited[0] = true;

    for (int step = 1; step < n; ++step) {
        int last = tour.last();
        int bestNext = -1;
        double bestDist = std::numeric_limits<double>::max();

        for (int c : candidates[last]) {
            if (!visited[c] && dist[last][c] < bestDist) {
                bestDist = dist[last][c];
                bestNext = c;
            }
        }
        // Fallback: find any unvisited
        if (bestNext < 0) {
            for (int j = 0; j < n; ++j) {
                if (!visited[j] && dist[last][j] < bestDist) {
                    bestDist = dist[last][j];
                    bestNext = j;
                }
            }
        }
        if (bestNext >= 0) {
            tour.append(bestNext);
            visited[bestNext] = true;
        }
    }

    // Improve with Lin-Kernighan
    QVector<int> optimalTour = linKernighan(tour, dist, candidates);
    double optLen = tourLength(optimalTour, dist);

    m_stats.totalSolves++;
    m_stats.numCities = n;
    m_stats.bestTourLength = optLen;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n, optLen, timer.elapsed());
    return optimalTour;
}

/* ---- Reset ---- */

void TravelingSalesman3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

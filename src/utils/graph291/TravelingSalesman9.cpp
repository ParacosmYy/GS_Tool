/**
 * @file TravelingSalesman9.cpp
 * @brief TravelingSalesman9 实现
 *
 * 实现旅行商问题求解器：Lin-Kernighan-Helsgaun可变深度搜索与候选集的近最优路径改进。
 */

#include "utils/graph291/TravelingSalesman9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

TravelingSalesman9::TravelingSalesman9(QObject *parent)
    : QObject(parent) {}

TravelingSalesman9::~TravelingSalesman9() = default;

/* ---- Configuration ---- */

void TravelingSalesman9::setConfig(const Config& config) { m_config = config; }

/* ---- Euclidean distance ---- */

double TravelingSalesman9::eucDist(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/* ---- Set distance matrix ---- */

void TravelingSalesman9::setDistanceMatrix(const QVector<QVector<double>>& dist)
{
    m_dist = dist;
    m_n = dist.size();
    m_candidates = buildCandidateSet();
    m_stats.numCities = m_n;
}

/* ---- Set city coordinates ---- */

void TravelingSalesman9::setCities(const QVector<QVector<double>>& coords)
{
    m_n = coords.size();
    m_dist.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_dist[i].resize(m_n, 0.0);
        for (int j = 0; j < m_n; ++j) {
            m_dist[i][j] = (i != j) ? eucDist(coords[i], coords[j]) : 0.0;
        }
    }
    m_candidates = buildCandidateSet();
    m_stats.numCities = m_n;
}

/* ---- Build candidate set (nearest neighbors) ---- */

QVector<QVector<int>> TravelingSalesman9::buildCandidateSet() const
{
    QVector<QVector<int>> cands(m_n);
    for (int i = 0; i < m_n; ++i) {
        // Sort all other cities by distance to city i
        QVector<QPair<double, int>> sorted;
        for (int j = 0; j < m_n; ++j) {
            if (i != j) sorted.append({m_dist[i][j], j});
        }
        std::sort(sorted.begin(), sorted.end());
        int k = qMin(m_config.maxCandidates, sorted.size());
        cands[i].reserve(k);
        for (int c = 0; c < k; ++c)
            cands[i].append(sorted[c].second);
    }
    return cands;
}

/* ---- Compute tour length ---- */

double TravelingSalesman9::tourLength(const QVector<int>& tour) const
{
    if (tour.size() < 2) return 0.0;
    double len = 0.0;
    for (int i = 0; i < tour.size(); ++i) {
        int next = (i + 1) % tour.size();
        if (tour[i] < m_n && tour[next] < m_n)
            len += m_dist[tour[i]][tour[next]];
    }
    return len;
}

/* ---- Nearest-neighbor initial tour ---- */

QVector<int> TravelingSalesman9::nearestNeighborTour(int startCity) const
{
    QVector<int> tour;
    QVector<bool> visited(m_n, false);
    int cur = startCity;
    visited[cur] = true;
    tour.append(cur);

    for (int step = 1; step < m_n; ++step) {
        double minDist = 1e18;
        int best = -1;
        for (int j = 0; j < m_n; ++j) {
            if (!visited[j] && m_dist[cur][j] < minDist) {
                minDist = m_dist[cur][j];
                best = j;
            }
        }
        if (best < 0) break;
        visited[best] = true;
        tour.append(best);
        cur = best;
    }
    return tour;
}

/* ---- Evaluate k-opt move ---- */

double TravelingSalesman9::evalMove(const QVector<int>& tour,
                                      const QVector<QPair<int, int>>& breaks,
                                      const QVector<QPair<int, int>>& joins) const
{
    double delta = 0.0;
    // Remove broken edges
    for (const auto& b : breaks) {
        if (tour[b.first] < m_n && tour[b.second] < m_n)
            delta -= m_dist[tour[b.first]][tour[b.second]];
    }
    // Add new edges
    for (const auto& j : joins) {
        if (tour[j.first] < m_n && tour[j.second] < m_n)
            delta += m_dist[tour[j.first]][tour[j.second]];
    }
    return delta;
}

/* ---- Reconstruct tour after k-opt ---- */

QVector<int> TravelingSalesman9::reconstructTour(const QVector<int>& tour,
                                                    const QVector<QPair<int, int>>& joins) const
{
    // Build adjacency from joins
    int n = tour.size();
    QVector<int> next(n, -1);
    for (const auto& j : joins) {
        next[j.first] = j.second;
    }
    // Follow chain from 0
    QVector<int> result;
    result.reserve(n);
    int cur = 0;
    for (int i = 0; i < n && cur >= 0; ++i) {
        result.append(tour[cur]);
        cur = next[cur];
    }
    // Fill remaining if chain is incomplete
    if (result.size() < n) {
        for (int i = 0; i < n; ++i) {
            if (!result.contains(tour[i]))
                result.append(tour[i]);
        }
    }
    return result;
}

/* ---- LKH variable-depth improvement ---- */

QVector<int> TravelingSalesman9::lkhImprove(const QVector<int>& tour)
{
    QVector<int> bestTour = tour;
    double bestLen = tourLength(tour);
    int n = tour.size();
    bool improved = true;

    while (improved) {
        improved = false;
        for (int t1 = 0; t1 < n && !improved; ++t1) {
            int city1 = bestTour[t1];
            int t2idx = (t1 + 1) % n;
            int city2 = bestTour[t2idx];

            // Try removing edge (t1, t2) and reconnecting via candidates
            for (int cand : m_candidates[city1]) {
                if (cand == city2) continue;

                // Find cand position in tour
                int t3 = -1;
                for (int i = 0; i < n; ++i) {
                    if (bestTour[i] == cand) { t3 = i; break; }
                }
                if (t3 < 0) continue;

                int t4idx = (t3 + 1) % n;
                int city4 = bestTour[t4idx];

                // 2-opt: replace (t1,t2)+(t3,t4) with (t1,t3)+(t2,t4)
                double oldCost = m_dist[city1][city2] + m_dist[cand][city4];
                double newCost = m_dist[city1][cand] + m_dist[city2][city4];
                if (newCost < oldCost - m_config.precision) {
                    // Perform 2-opt: reverse segment [t2idx..t3]
                    QVector<int> newTour = bestTour;
                    int lo = t2idx, hi = t3;
                    if (lo > hi) hi += n;
                    while (lo < hi) {
                        int a = lo % n, b = hi % n;
                        std::swap(newTour[a], newTour[b]);
                        lo++; hi--;
                    }
                    double newLen = tourLength(newTour);
                    if (newLen < bestLen - m_config.precision) {
                        bestTour = newTour;
                        bestLen = newLen;
                        improved = true;
                        break;
                    }
                }

                // 3-opt: try deeper moves via variable depth
                if (!improved && m_candidates[cand].size() >= 2) {
                    for (int cand2 : m_candidates[cand]) {
                        if (cand2 == city1 || cand2 == city2 || cand2 == city4) continue;
                        int t5 = -1;
                        for (int i = 0; i < n; ++i) {
                            if (bestTour[i] == cand2) { t5 = i; break; }
                        }
                        if (t5 < 0) continue;
                        int t6idx = (t5 + 1) % n;
                        double old3 = m_dist[city1][city2] + m_dist[cand][city4]
                                     + m_dist[cand2][bestTour[t6idx]];
                        double new3 = m_dist[city1][cand2] + m_dist[city2][cand]
                                     + m_dist[city4][bestTour[t6idx]];
                        if (new3 < old3 - m_config.precision) {
                            QVector<int> newTour = bestTour;
                            // Reverse segment [t2idx..t5]
                            int lo2 = t2idx, hi2 = t5;
                            if (lo2 > hi2) hi2 += n;
                            while (lo2 < hi2) {
                                std::swap(newTour[lo2 % n], newTour[hi2 % n]);
                                lo2++; hi2--;
                            }
                            double nLen = tourLength(newTour);
                            if (nLen < bestLen - m_config.precision) {
                                bestTour = newTour;
                                bestLen = nLen;
                                improved = true;
                                break;
                            }
                        }
                    }
                    if (improved) break;
                }
            }
        }
    }
    return bestTour;
}

/* ---- 2-opt local search ---- */

QVector<int> TravelingSalesman9::twoOpt(const QVector<int>& tour)
{
    QVector<int> best = tour;
    double bestLen = tourLength(best);
    int n = best.size();
    bool improved = true;

    while (improved) {
        improved = false;
        for (int i = 0; i < n - 1 && !improved; ++i) {
            for (int j = i + 2; j < n && !improved; ++j) {
                int a = best[i], b = best[i + 1];
                int c = best[j], d = best[(j + 1) % n];
                double oldCost = m_dist[a][b] + m_dist[c][d];
                double newCost = m_dist[a][c] + m_dist[b][d];
                if (newCost < oldCost - m_config.precision) {
                    // Reverse segment [i+1..j]
                    int lo = i + 1, hi = j;
                    while (lo < hi) {
                        std::swap(best[lo], best[hi]);
                        lo++; hi--;
                    }
                    bestLen = tourLength(best);
                    improved = true;
                }
            }
        }
    }
    return best;
}

/* ---- Solve TSP ---- */

TravelingSalesman9::TSPResult TravelingSalesman9::solve()
{
    QElapsedTimer timer;
    timer.start();

    TSPResult result;
    if (m_n < 2) return result;

    double bestOverall = 1e18;
    QVector<int> bestTour;
    int totalImprov = 0;

    for (int trial = 0; trial < m_config.maxTrials; ++trial) {
        // Initial tour: nearest-neighbor from different start cities
        int start = trial % m_n;
        auto tour = nearestNeighborTour(start);

        // LKH improvement
        tour = lkhImprove(tour);

        // Optional 2-opt refinement
        if (m_config.use2Opt)
            tour = twoOpt(tour);

        double len = tourLength(tour);
        if (len < bestOverall) {
            bestOverall = len;
            bestTour = tour;
            totalImprov++;
            emit tourImproved(trial, len, timer.elapsed());
        }
    }

    result.tour = bestTour;
    result.tourLength = bestOverall;
    result.iterations = m_config.maxTrials;
    result.improvements = totalImprov;
    result.optimal = false; // LKH is heuristic
    result.processingTimeMs = timer.elapsed();

    m_stats.bestTourLength = qMin(m_stats.bestTourLength, bestOverall);
    m_stats.totalImprovements += totalImprov;
    m_stats.totalOps++;
    m_timeSum += result.processingTimeMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveComplete(m_n, bestOverall, totalImprov, result.processingTimeMs);

    return result;
}

/* ---- Reset ---- */

void TravelingSalesman9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
